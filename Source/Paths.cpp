#include "Paths.h"


static std::filesystem::path s_PublicPath("./Public");
static std::filesystem::path s_PrivatePath("./Private");
static std::filesystem::path s_SitePath("./Private/Site");
static std::filesystem::path s_ComponentPath("./Private/Components");
static std::filesystem::path s_VarsPath("./Vars.txt");

std::filesystem::path const& GetPublicPath() {
    return s_PublicPath.make_preferred();
}

std::filesystem::path const& GetPrivatePath() {
    return s_PrivatePath.make_preferred();
}

std::filesystem::path const& GetSitePath() {
    return s_SitePath.make_preferred();
}

std::filesystem::path const& GetComponentPath() {
    return s_ComponentPath.make_preferred();
}

std::filesystem::path const& GetVarsPath() {
    return s_VarsPath.make_preferred();
}
