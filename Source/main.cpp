
#include "Logging.h"
#include "Paths.h"
#include "Render.h"
#include "VarsCollection.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>

int main()
{
    try 
    {
        // Ensure all required paths exist before begining.
        for(const auto& path : {k_PublicPath, k_PrivatePath, k_SitePath, k_ComponentPath}) {
            if(!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
                std::stringstream errorText;
                errorText << "Error: Path does not exist or is not a directory.\n";
                Logging::AppendFileDetails(errorText, path);
                throw std::runtime_error(errorText.str());
            }
        }

        std::optional<VarsCollection> vars;

        {
            auto loadingVarsJob = Logging::JobScope("Loading Vars.txt");
            if(std::filesystem::exists(k_VarsPath) && std::filesystem::is_regular_file(k_VarsPath))
            {
                vars = VarsCollection::TryLoadVarsCollection(k_VarsPath);

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
                Logging::AppendFileDetails(std::cout, k_VarsPath);

                Logging::LogWarning("Vars.txt not found, no variables loaded.");
                // A safe warning to ignore if you know what you're doing and don't need vars.txt
                Logging::LogWorkVerbose("Warning: This is unexpected but can be ignored.");
                
            }
        }

        {
            auto renderJob = Logging::JobScope("Rendering Site");
            for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(k_SitePath)) {
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
    return 0;
}