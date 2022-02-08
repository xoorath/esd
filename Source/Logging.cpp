#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#endif

#include "Logging.h"

#include <iostream>
#include <filesystem>
#include <stdarg.h>
#include <stdio.h>


// Header for all logging and assertions

namespace Logging {
    bool g_Verbose = true;

    namespace {
        size_t s_Indentation = 0;
        constexpr size_t k_MaxIndentation = 6;
        std::string GetIndentation() {
            return std::string(std::min<size_t>(s_Indentation, k_MaxIndentation)*2, ' ');
        }


        enum class ConsoleColor
        {
            Normal, Red, Yellow, Cyan
        };
#if defined(_MSC_VER)
        void SetConsoleColor(ConsoleColor color) {
            switch (color) {
            case ConsoleColor::Normal:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            case ConsoleColor::Red:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
                break;
            case ConsoleColor::Yellow:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                break;
            case ConsoleColor::Cyan:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                break;
            default:
                break;
            }
        }
#else
        void SetConsoleColor(ConsoleColor color) {
            switch (color) {
            case ConsoleColor::Normal:
                std::cout << "\x1B[0m";
                break;
            case ConsoleColor::Red:
                std::cout << "\x1B[31m";
                break;
            case ConsoleColor::Yellow:
                std::cout << "\x1B[33m";
                break;
            case ConsoleColor::Cyan:
                std::cout << "\x1B[36m";
                break;
            default:
                break;
            }
    }
#endif
    }

    void AppendFileDetails(std::ostream& os, std::filesystem::path const& path)
    {
        os << "\tCWD: " << std::filesystem::current_path() << "\n"
        << "\tPath: " << path << "\n"
        << "\tAbsolute Path: " << std::filesystem::absolute(path) << "\n";
    }

    void LogWork(char const* format, ...) {
        std::cout << GetIndentation();
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        std::cout << std::endl;
    }

    void LogWarning(char const* format, ...) {
        SetConsoleColor(ConsoleColor::Yellow);
        std::cout << GetIndentation() << "Warning: ";
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        std::cout << std::endl << "\x1B[0m";
        SetConsoleColor(ConsoleColor::Normal);
    }

    void LogError(char const* format, ...) {
        SetConsoleColor(ConsoleColor::Red);
        std::cout << GetIndentation() << "Error: ";
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        std::cout << std::endl;
        SetConsoleColor(ConsoleColor::Normal);
    }

    void LogWorkVerbose(char const* format, ...) {
        if(g_Verbose) {
            SetConsoleColor(ConsoleColor::Cyan);
            std::cout << GetIndentation();
            va_list args;
            va_start(args, format);
            vprintf(format, args);
            va_end(args);
            std::cout << std::endl;
            SetConsoleColor(ConsoleColor::Normal);
        }
    }

    JobScope::JobScope(char const* jobName) {
        if(s_Indentation == 0) {
            std::cout << "============================== " << jobName << std::endl;
        }
        ++s_Indentation;
    }

    JobScope::~JobScope() {
        --s_Indentation;
    
    }
}
