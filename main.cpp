#include "config/config.hpp"
#include <iostream>

Config globalConfig;

int main(int argc, char* argv[])
{
    try
    {
        // Parse config file
        std::string config_path = "config/default.conf";
        
        // Allow custom config file from command line
        if (argc > 1)
            config_path = argv[1];
        
        std::cout << "Loading config from: " << config_path << std::endl;
        
        if (!globalConfig.parse(config_path))
        {
            std::cerr << "Error: Failed to parse config file" << std::endl;
            return 1;
        }
        
        // Print parsed configuration for debugging
        globalConfig.printConfig();
        
        // Example: Access configuration
        const std::vector<ServerConfig>& servers = globalConfig.getServers();
        
        if (!servers.empty())
        {
            std::cout << "\nStarting servers..." << std::endl;
            for (size_t i = 0; i < servers.size(); ++i)
            {
                std::cout << "Server " << i + 1 << " listening on " 
                          << servers[i].host << ":" << servers[i].port << std::endl;
            }
        }
        
        // Your web server logic here
        // Example: Get server by port
        ServerConfig* server = globalConfig.getServerByPort(8080);
        if (server)
        {
            std::cout << "\nFound server on port 8080" << std::endl;
            std::cout << "Number of locations: " << server->locations.size() << std::endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
