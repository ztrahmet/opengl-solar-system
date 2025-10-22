#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// Configuration structure
struct Config {
    int width;
    int height;
    bool startFullscreen;
};

// Function to load configuration from a file
Config loadConfig(const std::string& filename);

#endif // CONFIG_H
