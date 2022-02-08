#include "VarsCollection.h"

#include "Logging.h"

#include <fstream>
#include <iostream>
#include <string.h>

//static
std::optional<VarsCollection> VarsCollection::TryLoadVarsCollection(std::filesystem::path const& path) {
    if(!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
        return {};
    }

    std::ifstream stream(path.c_str());

    if(stream.bad()) {
        Logging::LogError("Couldn't open %s for reading.", path.c_str());
        return {};
    }

    VarsCollection collection;
    std::string line;

    bool insideContinuation = false;
    std::string continuationKey;
    std::string continuationValue;

    auto const LineIsComment = [](std::string_view line) -> bool {
        return line.size() >= 1 && line[0] == '#';
    };

    auto const LineHasContinuation = [](std::string_view line) -> bool {
        return 
        // ends with backslash
        line.size() >= 1 && line[line.size()-1] == '\\' 
        // backslash is not escaped
        && (line.size() == 1 || line[line.size()-2] == '\\');
    };

    while(std::getline(stream, line)) {
        if(insideContinuation) {
            if(LineHasContinuation(line)) {
                // continuation keeps going, don't include the last char.
                continuationValue += "\n" + line.substr(0, line.size()-1);
            }
            else {
                // continuation ends here, we can accept this entire line.
                continuationValue += "\n" + line;
                collection.SetVariable(std::move(continuationKey), std::move(continuationValue));
                insideContinuation = false;
            }
            continue;
        }

        if(line.size() == 0) {
            continue;
        }
        if(LineIsComment(line)) {
            continue;
        }
        
        // Check if this is an assignment operation
        size_t const assignmentIndex = line.find_first_of('=');
        if(assignmentIndex != std::string::npos) {
            std::string const variableName = line.substr(0, assignmentIndex);

            // Check if this assignment has a continuation
            if(LineHasContinuation(line)) {
                continuationKey = variableName;
                // just like usual we pick up the value after the assignment op
                // except we want to stop one short from the end so we don't pick up the backslash
                continuationValue = line.substr(assignmentIndex+1, line.size()-assignmentIndex-2);
                // This bool helps us pick up all following continuations
                insideContinuation = true;
            } else {
                if(line[line.size()-1] == '\\') {
                    // just like usual we pick up the value after the assignment op
                    // except we want to stop one short from the end so we don't pick up the escaped backslash
                    std::string const variableValue = line.substr(assignmentIndex+1, line.size()-assignmentIndex-2);
                    collection.SetVariable(variableName, variableValue);
                } else {
                    // pick up the value after the assignment op
                    std::string const variableValue = line.substr(assignmentIndex+1);
                    collection.SetVariable(variableName, variableValue);
                }
            }
        }
    }

    return {collection};
}


std::optional<std::string_view> VarsCollection::TryGetVariable(std::string_view key) const {
    // todo: Once C++20 support improves it should be possible remove this temporary string with a little work.
    // see: https://stackoverflow.com/q/34596768
    auto found = m_VarMap.find(std::string(key));
    if(found == m_VarMap.end()) {
        return {};
    }
    return { found->second };
}

void VarsCollection::SetVariable(std::string_view key, std::string_view value) {
    // todo: Once C++20 support improves it should be possible remove this temporary string with a little work.
    // see: https://stackoverflow.com/q/34596768
    std::string keyString(key);
    auto found = m_VarMap.find(keyString);
    if(found != m_VarMap.end() && Logging::g_Verbose) {
        Logging::LogWarning("Overwriting variable \"%s\" from \"%s\" to \"%s\".", keyString.c_str(), found->first.c_str(), std::string(value).c_str());
    }
    m_VarMap[keyString] = value;
}

void VarsCollection::ForeachKey(std::function<void(std::string_view)> const& func) const {
    for(auto const& kvp : m_VarMap) {
        func(kvp.first);
    }
}

size_t VarsCollection::size() const {
    return m_VarMap.size();
}