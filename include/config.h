/**
 * @file config.h
 * @brief Defines the Config struct and the function to load configuration settings.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string> // For std::string

/**
 * @struct Config
 * @brief Holds application configuration settings loaded from config.ini.
 */
struct Config
{
    // Window settings
    int width = 800;              // Default window width
    int height = 600;             // Default window height
    bool startFullscreen = false; // Default to starting in windowed mode
};

/**
 * @brief Loads configuration settings from the specified INI file.
 * If the file cannot be read or a setting is missing, defaults are used.
 * @param filename Path to the configuration file (e.g., "config.ini").
 * @return A Config struct containing the loaded (or default) settings.
 */
Config loadConfig(const std::string &filename);

#endif // CONFIG_H
