#pragma once

#include <filesystem>
#include <ostream>

// Header for all logging and assertions
namespace Logging {

    extern bool g_Verbose;

    // A logging helper to append details to a stringstream regarding a path. Example:
    // "\tCurrent working directory: /a/b/c\n"
    // "\tPath: ../d\n"
    // "\tAbsolute Path: /a/b/d\n"
    void AppendFileDetails(std::ostream& os, std::filesystem::path const& path);

    void LogWork(char const* format, ...);
    void LogWarning(char const* format, ...);
    void LogError(char const* format, ...);

    void LogWorkVerbose(char const* format, ...);

    // Writes a high visibility log regarding a job starting and stopping, controlled by the scope of the JobScope.
    // Will increase indentation for other logs.
    struct JobScope {
        JobScope(char const* jobName);
        ~JobScope();
    };
}
