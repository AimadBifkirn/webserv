#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class config
{
    private:
        int port;
        std::string root;
        std::vector<std::string> index;
    public:
        config(std::string path = "./config/default.config") : port(3000), root("./app/")
        {
            index.push_back("index.html");
        };

        void create (std::string path) 
        {
            std::fstream    file(path);
            if (!file.is_open())
                return;
            std::stringstream   ss;
            ss << file.rdbuf();
            ss.str();
        }

        int getPort () {return this->port;};
        std::string getRoot () { return this->root;};
};

extern config server;