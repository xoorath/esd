#pragma once

#include <filesystem>

std::filesystem::path const& GetPublicPath();
std::filesystem::path const& GetPrivatePath();
std::filesystem::path const& GetSitePath();
std::filesystem::path const& GetComponentPath();
std::filesystem::path const& GetVarsPath();