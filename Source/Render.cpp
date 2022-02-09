#include "Render.h"

#include <array>
#include <istream>
#include <functional>
#include <fstream>
#include <set>
#include <vector>

#include "VarsCollection.h"
#include "Paths.h"
#include "Logging.h"

namespace {
    using namespace std::string_view_literals;

    constexpr std::string_view k_IncludeIndicator = "{include:"sv;
    constexpr std::string_view k_VarDeclarationIndicator = "{variable:"sv;
    constexpr std::string_view k_VarSubstitutionIndicator = "{$"sv;

    constexpr std::string_view k_CapChar = "}"sv;

    struct CappedSearchResult {
        // Where does the indicator begin (example: first open curly brace character)
        std::streamsize ResultStart = static_cast<std::streamsize>(-1);
        // The length of the statement
        size_t ResultSize = std::string::npos;
        // The string between the search indicator and it's end.
        std::string ResultCenter;
    };

    // Searches a stream for an indication (ie: "{include:"sv) and the assumed-to-be-present cap (ie: "}"sv).
    // It's presumed that the entire statement will exist and no caps will be stranded.
    std::vector<CappedSearchResult> FindIndicatorsWithCaps(std::istream& stream, std::string_view indicator, std::string_view cap) {

        if(stream.fail())
        {
            Logging::LogError("FindIndicatorsWithCaps failed to open input stream. This is an unexpected internal error.");
            return {};
        }

        if(stream.tellg() != 0)
        {
            Logging::LogError("FindIndicatorsWithCaps is operating with a used stream. This is an unexpected internal error.");
            return {};
        }

        // Once C++ 20 support improves comparisons between string view and string will improve.
        std::string const indicatorString(indicator);
        std::string const capString(cap);

        std::vector<CappedSearchResult> results;

        enum class ParseState
        {
            SeekingIndicator,
            SeekingCap
        };

        // How the current parsing should be interpreted.
        ParseState state = ParseState::SeekingIndicator;

        // Current char being read
        char ch = '\0';

        // How many sequential characters match the pattern we're looking for (indicator/cap)
        size_t match = 0;
        // Current index in the file we're evaluating
        std::streamsize i = 0;
        // Once we've found an indicator or a cap we track that position. This will help with replacing the entire found statement later
        std::streamsize indicatorStart = 0;
        std::streamsize capStart = 0;
        // The value between the start indicator and the cap. Ie: "{example}" would store example if the indicator was '{' and the cap was '}'
        std::string midValue;
        size_t charsRead = 0;
        while(stream.get(ch)) {
            ++charsRead;
            switch(state) {
                case ParseState::SeekingIndicator:
                    if(ch == indicatorString[match]) {
                        if(match == 0) {
                            indicatorStart = i;
                        }
                        match++;
                    } else {
                        match = 0;
                    }
                    if(indicatorString.size() == match) {
                        state = ParseState::SeekingCap;
                        match = 0;
                    }
                break;
                case ParseState::SeekingCap:
                    midValue += ch;
                    if(ch == capString[match]) {
                        if(match == 0) {
                            capStart = i;
                        }
                        match++;
                    } else {
                        match = 0;
                    }

                    if(capString.size() == match) {
                        midValue.resize(midValue.size() - cap.size());

                        results.push_back({
                            indicatorStart,
                            static_cast<size_t>(capStart - indicatorStart)+1,
                            midValue
                        });

                        // reset everything
                        state = ParseState::SeekingIndicator;
                        match = 0;
                        indicatorStart = 0;
                        capStart = 0;

                        midValue.clear();
                    }
                break;
            }
            i++;
        }

        if(charsRead == 0) {
            Logging::LogWarning("File appears empty.");
        }

        return results;
    }

    bool ReplaceIncludesWhileCopying(std::istream& inputStream, std::ostream& outputStream, std::vector<CappedSearchResult> const& includes, int depth) {
        if(includes.size() == 0) {
            return true;
        }
        if(inputStream.fail()) {
            Logging::LogError("ReplaceIncludesWhileCopying failed to open input stream. This is an unexpected internal error.");
            return false;
        }
        if(inputStream.tellg() != 0) {
            Logging::LogError("ReplaceIncludesWhileCopying operating with a used input stream. This is an unexpected internal error.");
            return false;
        }
        if(outputStream.fail()) {
            Logging::LogError("ReplaceIncludesWhileCopying failed to open output stream. This is an unexpected internal error.");
            return false;
        }
        if(outputStream.tellp() != 0) {
            Logging::LogError("ReplaceIncludesWhileCopying operating with a used output stream. This is an unexpected internal error.");
            return false;
        }

        if(includes.size() != 0) {
            // This position indicates where we are in the source file. We use this and the
            // CappedSearchResult indicies to advance through the source file and print to the output file.
            std::streamsize position = 0;
            for(CappedSearchResult const& include : includes) {

                // Collect all (non-include) file content from the source file.
                for(std::streamsize i = position; i < include.ResultStart; ++i, ++position) {
                    char ch = '\0';
                    inputStream.get(ch);
                    outputStream << ch;
                }

                // print the entire included file to the output file.
                auto const includePath = GetComponentPath() / include.ResultCenter;
                
                Logging::LogWorkVerbose("Including file: %s", includePath.string().c_str());

                std::ifstream includedFile(includePath.c_str());
                if(includedFile.is_open()) {
                    char ch = '\0';
                    while(includedFile.get(ch)) {
                        outputStream << ch;
                    }
                } else {
                    Logging::LogError("Include file not found: %s", includePath.string().c_str());
                }

                // skip over the include statement itself so we don't print it in subsequent iterations.
                // Advance position so our relative source file position is correct
                position += include.ResultSize;
                // Then advance the input file itself so we skil the include statement.
                inputStream.seekg(inputStream.tellg() + static_cast<std::streampos>(include.ResultSize));
            }

            // Copy the remainder of the file.
            char ch = '\0';
            while(inputStream.get(ch)) {
                outputStream << ch;
            }
        }
        else
        {
            Logging::LogWarning("ReplaceIncludesWhileCopying being called with no includes. This is easily handled but is an unexpected internal issue.");

            // Copy with the streams rather than filesystem::copy so we don't have to close the open streams.
            char ch = '\0';
            while(inputStream.get(ch)) {
                outputStream << ch;
            }
        }
        return true;
    }

    void RenderIncludes(std::filesystem::path const& sourcePath, std::filesystem::path const& outputPath) {
        auto job = Logging::JobScope("Render Includes");
        std::filesystem::path const sitePathRelative = std::filesystem::relative(sourcePath, GetSitePath());

        if(std::filesystem::exists(outputPath)) {
            Logging::LogWorkVerbose("Deleting %s", outputPath.string().c_str());
            if(!std::filesystem::remove(outputPath)) {
                Logging::LogError("Could not delete %s. We will attempt to continue but rendering this page is unlikely to succeed.", outputPath.string().c_str());
            }
        }

        std::fstream sourceFile(sourcePath.c_str(), std::ios::in);

        if(sourceFile.fail()) {
            Logging::LogError("Could not open the source file for reading: %s", sourcePath.string().c_str());
            return;
        }


        std::vector<CappedSearchResult> results;

        results = FindIndicatorsWithCaps(sourceFile, k_IncludeIndicator, k_CapChar);
        sourceFile.clear();
        sourceFile.seekg(0);

        std::string buffer;
        int  depth = 0;

        // HACK: The way I've designed includes to work doesn't allow us to easily determine where a particular include
        // file came from (ie: what file caused this include declaration to exist). Because of that we are limiting 
        // the max include depth to something high that is unlikely to be hit under normal circumstances.
        // Ideally we should just do a pre-processing pass where we create an include tree to find circular dependencies first
        // or change the renderer design to more easily collect a stack of work it's doing.
        constexpr int k_maxIncludeDepth = 30;

        //continue to count the number of includes proccessed (to log later)
        int includesProcessed = static_cast<int>(results.size());

        // Only process includes if there are any, otherwise we can skip a temporary file and copy directly to output.
        if(results.size() > 0) {
            // we keep two temporary paths so we can ping-pong between temporary files as we process includes
            size_t tempFileOutIndex = 0;
            std::array<std::filesystem::path, 2> const tempPaths = {
                std::filesystem::temp_directory_path() / sitePathRelative.parent_path() / (std::filesystem::path("0_") += (sitePathRelative.filename())),
                std::filesystem::temp_directory_path() / sitePathRelative.parent_path() / (std::filesystem::path("1_") += (sitePathRelative.filename()))
            };

            for(auto const& p : tempPaths) {
                if(std::filesystem::exists(p)) {
                    Logging::LogWorkVerbose("Deleting %s", p.string().c_str());
                    if(!std::filesystem::remove(p)) {
                        Logging::LogError("Could not delete %s. We will attempt to continue but rendering this page is unlikely to succeed.", p.string().c_str());
                    }
                }
                std::filesystem::create_directories(p.parent_path());
            }

            std::array<std::fstream, 2> tempFiles = { std::fstream(tempPaths[0], std::ios::in | std::ios::out | std::ios::trunc), std::fstream(tempPaths[1], std::ios::in | std::ios::out | std::ios::trunc) };

            std::fstream* currentSourceFile = &sourceFile;
            std::fstream* currentOutputFile = &tempFiles[tempFileOutIndex];

            if(currentOutputFile->fail()) {
                Logging::LogError("Could not open temp file for writing: %s", tempPaths[tempFileOutIndex].string().c_str());
                
                throw std::runtime_error("Could not open temp file for writing.");
            }

            // repeatedly go over the file until no includes remain
            // (this is how we recursively collect includes)
            while(results.size() > 0) {
                if(!ReplaceIncludesWhileCopying(*currentSourceFile, *currentOutputFile, results, depth)) {
                    // returning false indicates an unexpected error, stop processing.
                    // That error should already be logged from ReplaceIncludesWhileCopying
                    results.clear();
                    break;
                }

                currentSourceFile->clear();
                currentSourceFile->seekg(0);
                currentOutputFile->clear();
                currentOutputFile->seekp(0);
                
                // Begin the ping-pong between two temporary files 
                // this also swaps between the initial source file and the first temporary file.
                size_t const nextTempFileOutIndex = ((tempFileOutIndex + 1) % tempFiles.size());            
                currentSourceFile = currentOutputFile;
                currentOutputFile = &tempFiles[nextTempFileOutIndex];

                // Look for more include processing to do in the new current source file 
                // (the file we just wrote to) This allows us to recursively process includes.
                results = FindIndicatorsWithCaps(*currentSourceFile, k_IncludeIndicator, k_CapChar);

                includesProcessed += static_cast<int>(results.size());
                
                currentSourceFile->clear();
                currentSourceFile->seekg(0);

                // If we continue processing more results: advance the tempFileOutIndex.
                // Otherwise we want to keep the index correct (current) for use outside this loop.
                tempFileOutIndex = results.size() ? nextTempFileOutIndex : tempFileOutIndex;

                // The include depth is to help us style/indicate include depth in the program output.
                // Using it for the k_maxIncludeDepth is a hack. See k_maxIncludeDepth declaration.
                if (++depth > k_maxIncludeDepth) {
                    Logging::LogError("Max include depth of %d hit. This normally means includes are circular.", k_maxIncludeDepth);
                    break;
                }
            }

            std::filesystem::create_directories(outputPath.parent_path());

            // intentionally not providing an error code param so we can handle copy errors with exceptions.
            std::filesystem::copy(tempPaths[tempFileOutIndex], outputPath, std::filesystem::copy_options::overwrite_existing);

        } else {
            std::filesystem::create_directories(outputPath.parent_path());
            // skip making a temporary file, just copy directly to output.
            std::filesystem::copy(sourcePath, outputPath, std::filesystem::copy_options::overwrite_existing);
            

        }

        Logging::LogWork("%d include%s processed", includesProcessed, includesProcessed==1?"":"s");
    }

    // Removes all instances of variable declarations (like: "${variable:name=value}") in the file at mutablePagePath.
    // While doing so these variable declarations are parsed into the returned VarsCollection.
    // If any failure occurs that can not be recovered from {} will be returned instead.
    std::optional<VarsCollection> ParseInlineVariables(std::filesystem::path const& mutablePagePath) {
        auto job = Logging::JobScope("Variable Declaration");

        std::filesystem::path const publicPathRelative = std::filesystem::relative(mutablePagePath, GetPublicPath());

        if (!std::filesystem::exists(mutablePagePath) || !std::filesystem::is_regular_file(mutablePagePath)) {
            Logging::LogError("File not found: %s", mutablePagePath.string().c_str());
            return {};
        }

        std::fstream pageStream(mutablePagePath.c_str(), std::ios::in | std::ios::out);

        if (pageStream.fail()) {
            Logging::LogError("Could not open page file for reading: %s", mutablePagePath.string().c_str());
            return {};
        }

        std::vector<CappedSearchResult> results;
        VarsCollection collection;

        results = FindIndicatorsWithCaps(pageStream, k_VarDeclarationIndicator, k_CapChar);
        pageStream.clear();
        pageStream.seekg(0);

        int variablesDeclared = 0;
        
        if (results.size() > 0) {

            std::filesystem::path const tempPath = std::filesystem::temp_directory_path() / publicPathRelative.parent_path() / (std::filesystem::path("2_") += (publicPathRelative.filename()));
            if (std::filesystem::exists(tempPath)) {
                Logging::LogWorkVerbose("Deleting %s", tempPath.string().c_str());
                if (!std::filesystem::remove(tempPath)) {
                    Logging::LogError("Could not delete %s. We will attempt to continue but rendering this page is unlikely to succeed.", tempPath.string().c_str());
                }
            }

            std::filesystem::create_directories(tempPath.parent_path());

            std::fstream tempFileStream = std::fstream(tempPath, std::ios::out | std::ios::trunc);

            if (tempFileStream.fail()) {
                Logging::LogError("Could not open temp file for writing: %s.", tempPath.string().c_str());
                return {};
            }

            std::streamsize position = 0;
            for (CappedSearchResult const& variableDeclaration : results) {

                // Collect all (non-variable) file content from the source file.
                for (std::streamsize i = position; i < variableDeclaration.ResultStart; ++i, ++position) {
                    char ch = '\0';
                    pageStream.get(ch);
                    tempFileStream << ch;
                }

                size_t assignmentIndex = variableDeclaration.ResultCenter.find_first_of('=');

                if (assignmentIndex == std::string::npos) {
                    Logging::LogWarning("Inline variable declaration is invalid: \"%s\"", variableDeclaration.ResultCenter.c_str());
                }
                else {
                    std::string const key = variableDeclaration.ResultCenter.substr(0, assignmentIndex);
                    std::string const value = variableDeclaration.ResultCenter.substr(assignmentIndex+1);
                    collection.SetVariable(key, value);
                    ++variablesDeclared;
                }

                // Then advance the input file itself so we skip the variable declaration.
                position += variableDeclaration.ResultSize;
                pageStream.seekg(pageStream.tellg() + static_cast<std::streampos>(variableDeclaration.ResultSize));
            }

            // Copy the remainder of the file.
            char ch = '\0';
            while (pageStream.get(ch)) {
                tempFileStream << ch;
            }

            pageStream.close();
            tempFileStream.close();

            std::filesystem::copy(tempPath, mutablePagePath, std::filesystem::copy_options::overwrite_existing);
        }

        Logging::LogWork("%d inline variable%s declared", variablesDeclared, variablesDeclared == 1 ? "" : "s");
        return { collection };
    }

    // Replaces instances of variables (like: "{$var_name}") in a file at mutablePagePath with variables from variableCollections
    // If these variables do not exist the variable statement will be left in place to hopefully in many cases indicate clearly where a problem occured.
    // "mutablePagePath" is named as such because the file at this path will be changed.
    void SubstituteVariables(std::filesystem::path const& mutablePagePath, std::initializer_list<std::optional<VarsCollection>> variableCollections) {
        auto job = Logging::JobScope("Variable Substitution");
        std::filesystem::path const publicPathRelative = std::filesystem::relative(mutablePagePath, GetPublicPath());

       if(!std::filesystem::exists(mutablePagePath) || !std::filesystem::is_regular_file(mutablePagePath)) {
            Logging::LogError("File not found: %s", mutablePagePath.string().c_str());
            return;
        }

        std::fstream pageStream(mutablePagePath.c_str(), std::ios::in | std::ios::out);

        if(pageStream.fail()) {
            Logging::LogError("Could not open page file for reading: %s", mutablePagePath.string().c_str());
            return;
        }

        std::vector<CappedSearchResult> results;
        std::set<std::string> failedSubstitutionNames;

        results = FindIndicatorsWithCaps(pageStream, k_VarSubstitutionIndicator, k_CapChar);
        pageStream.clear();
        pageStream.seekg(0);

        int variablesSubstituted = 0;
        int failedSubstitutions = 0;

        // Only process includes if there are any, otherwise we can skip a temporary file and copy directly to output.
        if(results.size() > 0) {

            std::filesystem::path const tempPath = std::filesystem::temp_directory_path() / publicPathRelative.parent_path() / (std::filesystem::path("3_") += (publicPathRelative.filename()));
            if(std::filesystem::exists(tempPath)) {
                Logging::LogWorkVerbose("Deleting %s", tempPath.string().c_str());
                if(!std::filesystem::remove(tempPath)) {
                    Logging::LogError("Could not delete %s. We will attempt to continue but rendering this page is unlikely to succeed.", tempPath.string().c_str());
                }
            }

            std::filesystem::create_directories(tempPath.parent_path());

            std::fstream tempFileStream = std::fstream(tempPath, std::ios::out | std::ios::trunc);

            if(tempFileStream.fail()) {
                Logging::LogError("Could not open temp file for writing: %s.", tempPath.string().c_str());
                return;
            }

            std::streamsize position = 0;
            for(CappedSearchResult const& variableSubstitution : results) {

                // Collect all (non-variable) file content from the source file.
                for(std::streamsize i = position; i < variableSubstitution.ResultStart; ++i, ++position) {
                    char ch = '\0';
                    pageStream.get(ch);
                    tempFileStream << ch;
                }

                std::string substitution;
                bool valueSubstituted = false;
                for(auto varCollection : variableCollections) {
                    if(varCollection.has_value()) {
                        auto var = varCollection.value().TryGetVariable(variableSubstitution.ResultCenter);
                        if(var.has_value()) {
                            substitution = var.value();
                            valueSubstituted = true;
                            // Don't continue looking at other collections once we've found a suitable variable substitution
                            break;
                        }
                    }
                }

                if(valueSubstituted) {
                    tempFileStream << substitution;
                    variablesSubstituted++;
                } else {
                    tempFileStream << variableSubstitution.ResultCenter;
                    failedSubstitutions++;
                    failedSubstitutionNames.insert(variableSubstitution.ResultCenter);
                }

                // Then advance the input file itself so we skip to the end of the variable substitution
                position += variableSubstitution.ResultSize;
                pageStream.seekg(pageStream.tellg() + static_cast<std::streampos>(variableSubstitution.ResultSize));
            }

            // Copy the remainder of the file.
            char ch = '\0';
            while(pageStream.get(ch)) {
                tempFileStream << ch;
            }

            pageStream.close();
            tempFileStream.close();

            std::filesystem::copy(tempPath, mutablePagePath, std::filesystem::copy_options::overwrite_existing);
        }
        Logging::LogWork("%d variable%s substituted", variablesSubstituted, variablesSubstituted == 1 ? "" : "s");
        if (failedSubstitutions > 0) {
            std::stringstream ss;
            for (std::string const& name : failedSubstitutionNames) {
                ss << "'" << name << "' ";
            }
            Logging::LogWarning("Variable substitution failed %d times with these variables: %s", failedSubstitutions, ss.str().c_str());
        }
    }
}

void RenderPage(std::filesystem::path const& sourcePath, std::optional<VarsCollection> const& vars) {

    std::filesystem::path const sitePathRelative = std::filesystem::relative(sourcePath, GetSitePath());
    std::filesystem::path const outputPath = GetPublicPath() / sitePathRelative;

    Logging::LogWork("Source File: %s", sourcePath.string().c_str());
    Logging::LogWork("Output: %s", outputPath.string().c_str());
    
    if(!std::filesystem::exists(sourcePath) || !std::filesystem::is_regular_file(sourcePath)) {
        Logging::LogError("File not found: %s", sourcePath.string().c_str());
        return;
    }

    RenderIncludes(sourcePath, outputPath);
    std::optional<VarsCollection> inlineVariables = ParseInlineVariables(outputPath);

    // pass inlineVariables first so they are read before the variables from Vars.txt
    SubstituteVariables(outputPath, { inlineVariables, vars });

    Logging::LogWork("");
}