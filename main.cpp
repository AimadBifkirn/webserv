#include "config/config.hpp"
#include <iostream>

Config globalConfig;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }
    try
    {
        globalConfig.parse(argv[1]);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    globalConfig.printConfig(); // For debugging
    return 0;
}
    