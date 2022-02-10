
#include "Logging.h"
#include "Paths.h"
#include "Render.h"
#include "VarsCollection.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>

int main(int argc, char const* argv[])
{
    auto startTime = std::chrono::steady_clock::now();
    try 
    {
        for (int i = 0; i < argc; ++i) {
            if (std::strcmp(argv[i], "-v") == 0) {
                Logging::g_Verbose = true;
            }
        }

        {
            // The site path is required, if we don't have it we probably didn't start the program correctly.
            const auto& sitePath = GetSitePath();
            if (!std::filesystem::exists(sitePath) || !std::filesystem::is_directory(sitePath)) {
                std::stringstream errorText;
                errorText << sitePath << " does not exist or is not a directory.\n";
                Logging::AppendFileDetails(errorText, sitePath);
                throw std::runtime_error(errorText.str());
            }

            if (std::filesystem::is_empty(sitePath)) {
                std::stringstream errorText;
                errorText << sitePath << " is empty, there's no work to do.\n";
                Logging::AppendFileDetails(errorText, sitePath);
                throw std::runtime_error(errorText.str());
            }

            const auto& publicPath = GetPublicPath();
            if (!std::filesystem::exists(publicPath)) {
                std::filesystem::create_directories(publicPath);
            }
        }

        std::optional<VarsCollection> vars;

        {
            auto loadingVarsJob = Logging::JobScope("Loading Vars.txt");
            if(std::filesystem::exists(GetVarsPath()) && std::filesystem::is_regular_file(GetVarsPath()))
            {
                vars = VarsCollection::TryLoadVarsCollection(GetVarsPath());

                if(vars.has_value()) {
                    Logging::LogWork("%d variables loaded.", static_cast<int>(vars.value().size()));

                    if(Logging::g_Verbose) {
                        std::stringstream ss;
                        vars.value().ForeachKey([&ss](std::string_view key) -> void {
                            ss << key << " ";
                        }); 
                        Logging::LogWorkVerbose("Variables: %s", ss.str().c_str());
                    }
                } else {
                    Logging::LogWarning("Vars.txt couldn't be loaded. No variables loaded.");
                }
            }
            else {
                Logging::AppendFileDetails(std::cout, GetVarsPath());

                Logging::LogWarning("Vars.txt not found, no variables loaded.");
                // A safe warning to ignore if you know what you're doing and don't need vars.txt
                Logging::LogWorkVerbose("Warning: This is unexpected but can be ignored.");
                
            }
        }

        {
            auto renderJob = Logging::JobScope("Rendering Site");
            for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(GetSitePath())) {
                if(entry.is_regular_file()) {
                    RenderPage(entry.path(), vars);
                }
            }
        }
    }
    catch(std::exception& exception)
    {
        Logging::LogError(exception.what());
        return -1;
    }
    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    auto ms = (elapsedSeconds * 1000.0).count();
    if (ms >= 1.0) {
        std::cout << "Took " << static_cast<int>(ms) << "ms" << std::endl;
    } else {
        // This is very optimistic for a file reading application... probably not needed.
        std::cout << "Took " << static_cast<int>(ms*1000.0) << "us" << std::endl;
    }
    return 0;
}