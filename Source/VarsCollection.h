#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string_view>
#include <unordered_map>

enum class VarNameValidity {
    Valid,
    FirstCharInvalid,
    Invalid
};

// Helper function to evaluate validity of variable name declaration or usage.
VarNameValidity CheckVariableNameValidity(std::string_view variableName);

/**************************************************************************************************
Vars Collection:
    Vars collection provides access to a table of named variables.

    See TryLoadVarsCollection for details on the Vars.txt file format.
**************************************************************************************************/
class VarsCollection
{
public:
    /***************************************************************************************************
        TryLoadVarsCollection will attempt to load a file (path) and parse it using the following 3
        features:

        1. Comments are lines starting with a # symbol.
        2. Variables are lines starting with any other text that contains one = character
        3. Variables can be multi-line if their line ends in a \ character. This supersedes the first 
        two features.
        3.1 This feature can be escaped with a double backslash as the last character.
    ***************************************************************************************************/
    static std::optional<VarsCollection> TryLoadVarsCollection(std::filesystem::path const& path);

    VarsCollection()                                 = default;
    ~VarsCollection()                                = default;
    VarsCollection(VarsCollection const&)            = default;
    VarsCollection& operator=(VarsCollection const&) = default;
    VarsCollection(VarsCollection &&)                = default;
    VarsCollection& operator=(VarsCollection &&)     = default;

    // Attempts to retrieve a string variable from the collection by key.
    // No logging is done if the variable is not found.
    std::optional<std::string_view> TryGetVariable(std::string_view key) const;

    // Assigns a variable with a key and a value. Logs a warning if the value is a duplicate.
    void SetVariable(std::string_view key, std::string_view value);

    void ForeachKey(std::function<void(std::string_view)> const& func) const;

    size_t size() const;

private:

    std::unordered_map<std::string, std::string> m_VarMap;
};