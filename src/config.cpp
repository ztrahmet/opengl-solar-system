#include "config.h"
#include "ini.h"
#include <iostream>
#include <string>
#include <cstring> // For strcmp

// This is the callback function that inih will call for each line in the .ini file
static int handler(void* user, const char* section, const char* name,
                   const char* value, int lineno) {
    
    // Suppress "unused parameter" warning
    (void)lineno;

    Config* pconfig = (Config*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("window", "width")) {
        pconfig->width = std::stoi(value);
    } else if (MATCH("window", "height")) {
        pconfig->height = std::stoi(value);
    } else if (MATCH("window", "fullscreen")) {
        // Correctly interpret "true" as a boolean true, and anything else as false.
        pconfig->startFullscreen = (std::string(value) == "true");
    } else {
        return 0;  // unknown section/name, but not an error
    }
    return 1; // return 1 on success
}

// Loads configuration from a .ini file
Config loadConfig(const std::string& filename) {
    Config config;
    
    // --- Step 1: Set hardcoded default values ---
    // These values will be used if the config file is not found,
    // or if a specific setting is missing from the file.
    config.width = 800;
    config.height = 600;
    config.startFullscreen = false; // Default to windowed mode

    // --- Step 2: Attempt to parse the .ini file ---
    // The 'handler' function will be called for each setting in the file,
    // overwriting the default values that were set above.
    if (ini_parse(filename.c_str(), handler, &config) < 0) {
        // This block only runs if the file 'filename' cannot be opened or read.
        std::cerr << "Error: Could not load '" << filename << "'." << std::endl;
        std::cerr << "Using default settings." << std::endl;
    } else {
        // This block runs if the file was found and parsed successfully.
        std::cout << "Successfully loaded configuration from '" << filename << "'" << std::endl;
    }

    // --- Step 3: Return the final configuration ---
    // If the file was loaded, 'config' contains the values from the file.
    // If loading failed, 'config' still contains the default values from Step 1.
    return config;
}
