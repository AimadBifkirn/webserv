#include "config.hpp"

 bool Config::parse (const std::string& filepath)
{
    std::ifstream file(filepath.c_str());
    if (!file.is_open())
        return false;
    // This is a placeholder implementation
    return true;
}