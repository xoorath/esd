#pragma once

#include <filesystem>
#include <optional>

class VarsCollection;

/**************************************************************************************************
    Include Files:
        
        example: {include:some/file.html}
    
        In the above example the entire statement will be replaced with a file in the
        Private/components directory. In this case the file Private/Components/some/file.html will
        replace the include statement.

        Include paths are recursive and importantly they occur before variables are processed.

    Variable Declaration:

        example: {variable:red=#ff0000}

        In the above example a variable can be declared in source. This variable declaration does
        not support multi-line variables like the vars file (VarsCollection.h).

        These variables have no scope and are globally accessable even before their declaration.

        There are no escape characters available, a close brace will always terminate the statement
        and can not be used as part of a variable name or value.

    Variable Substitution:

        example: {$site_title}

        In the above example the entire statement will be replaced by a variable named site_title.
        See VarsCollection.h for more detail on variable declaration.


    Limitations

        1) All source must avoid collisions with the following special strings:
            ["{$",  "{include:", "{variable:"]
            If your source file contains any of those strings they are assumed to be a part of 
            the esd process. For example, avoid javascript like {$(document).ready(()=>{});} which
            could be confused with a variable substitution due to the "{$" character.
        2) All statements are expected to be fully formed and are intolerant to errors or variations.


**************************************************************************************************/


void RenderPage(std::filesystem::path const& path, std::optional<VarsCollection> const& vars);