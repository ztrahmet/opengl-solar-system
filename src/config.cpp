/**
 * @file config.cpp
 * @brief Implements the configuration loading function using the inih library.
 */

#include "config.h" // Includes definition of Config struct
#include "ini.h"    // Includes the inih parser header
#include <iostream> // For error reporting (std::cerr)
#include <string>   // For std::string comparison
#include <cstring>  // For strcmp

/**
 * @brief Callback function used by the inih parser.
 * This function is called for each name=value pair found in the INI file.
 * It updates the Config struct passed via the 'user' pointer.
 *
 * @param user Pointer to the Config struct being populated.
 * @param section The current section name (e.g., "window").
 * @param name The setting name (e.g., "width").
 * @param value The setting value as a string (e.g., "1280").
 * @param lineno The line number in the INI file (unused here).
 * @return 1 on success (setting recognized), 0 if setting is unknown (but not an error).
 */
static int handler(void *user, const char *section, const char *name,
                   const char *value, int lineno)
{

    // Cast the generic user pointer back to our Config struct type
    Config *pconfig = (Config *)user;

// Use a macro for easier matching of section and name
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("window", "width"))
    {
        pconfig->width = std::stoi(value); // Convert string value to integer
    }
    else if (MATCH("window", "height"))
    {
        pconfig->height = std::stoi(value);
    }
    else if (MATCH("window", "fullscreen"))
    {
        // Interpret "true" (case-sensitive) as boolean true, otherwise false
        pconfig->startFullscreen = (strcmp(value, "true") == 0);
    }
    else
    {
        return 0; // Unknown section or name, but not necessarily an error, just ignore it
    }
    return 1; // Return 1 indicates success
}

/**
 * @brief Loads configuration from the specified INI file.
 * Sets default values first, then attempts to parse the file using inih,
 * which calls the 'handler' function to overwrite defaults with values from the file.
 */
Config loadConfig(const std::string &filename)
{
    Config config; // Uses default values defined in config.h initially

    // Attempt to parse the INI file
    if (ini_parse(filename.c_str(), handler, &config) < 0)
    {
        // ini_parse returns < 0 on file open error
        std::cerr << "Warning: Could not load config file '" << filename << "'. Using default settings." << std::endl;
        // No action needed, 'config' still holds the default values
    }
    // Note: ini_parse returns line number of error if parsing fails after opening,
    // but we currently ignore parsing errors and just use defaults for missing/bad values.

    return config; // Return the resulting config (either defaults or file values)
}
