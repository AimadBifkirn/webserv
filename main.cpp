#include "config/config.hpp"

config server;

int main()
{
    try
    {
       
    }
    catch(const std::exception& e)
    {
        // std::cerr << e.what() << '\n';
    }
    std::string rootDir = server.getRoot();

    // Application logic here

    return 0;
}
