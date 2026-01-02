#pragma once

#include <string>

class config
{
    private:
        int port;
        std::string root;
    public:
        config() : port(3000), root("undefined") {};

        int getPort () {return this->port;};
        std::string getRoot () { return this->root;};
};

extern config server;