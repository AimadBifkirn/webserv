#include "config.hpp"

static std::vector<std::string> tokenize(const std::string &content)
{
    std::vector<std::string> tokens;
    std::istringstream stream(content);
    std::string token;
    while (stream >> token) // this skips whitspaces and stores the token, if stream = "hey there" -> token = "hey", then "there"
    {
        tokens.push_back(token);
    }
    return tokens;
}

// helper function to check if a string contains only digits
bool isDigits(const std::string& s)
{
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (!std::isdigit(s[i])) return false;
    }
    return true;
}

// helper function to parse client_max_body_size value
size_t getClientMaxBodySize(const std::string &token)
{
    size_t size = 0;
    if (token.back() == ';' && (token[token.size() - 2] == 'M' || token[token.size() - 2] == 'm'))
    {
        std::istringstream ss(token.substr(0, token.size() - 2));
        if (!(ss >> size) || size < 1 || size > 100)
            throw std::runtime_error("Error: invalid client_max_body_size value");
        size = size * 1024 * 1024; // charh: 1M = 1024 kb, 1kb = 1024 bytes
    }
    else 
        throw std::runtime_error("Error: client_max_body_size must end with 'M' or 'm' for megabytes + ';'");
    return size;
}

std::string getIPorPort(const std::string &token)
{
    for (size_t i = 0; i < token.size(); ++i)
    {
        if (token.find(':') != std::string::npos) // ila 3tani ip + port
        {
            std::stringstream ss;
            for (; token [i] && token[i] !=  '.'; ++i)
            {
                if (std::isdigit(token[i]))
                    ss << token[i];
                else
                    throw std::runtime_error("Error: invalid character in port number");
            }
            // if (token[i] == '.')
        }
        else // ila 3tani gha port
        {}
    }
}

static void parseServerBlock(const std::vector<std::string>& tokens, size_t& i, ServerConfig& server)
{
    i += 2; // Move past "server {"
    while (i < tokens.size() && tokens[i] != "}")
    {
        if (tokens[i] == "listen")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1].back() != ';')
                throw std::runtime_error("Error: expected port number + ';'");
            if (isDigits(tokens[i + 1].substr(0, tokens[i + 1].size() - 1)) == false)
                throw std::runtime_error("Error: port must be numeric");
            std::istringstream ss(tokens[i + 1]);
            int port;
            if (!ss >> port || port < 0 || port > 65535)
                throw std::runtime_error("Error: invalid port number");    
            server.setPort(port);
            i += 2;
        }
        else if (tokens[i] == "host")
        {
            if (i + 1 >= tokens.size())
                throw std::runtime_error("Error: expected host string");
            server.setHost(tokens[i + 1]);
            i += 2;
        }
        else if (tokens[i] == "server_name")
        {
            if (i + 1 >= tokens.size())
                throw std::runtime_error("Error: expected server_name string");
            server.setServerName(tokens[i + 1]);
            i += 2;
        }
        else if (tokens[i] == "root")
        {
            if (i + 1 >= tokens.size())
                throw std::runtime_error("Error: expected root string");
            server.setRoot(tokens[i + 1]);
            i += 2;
        }
        else if (tokens[i] == "index")
        {
            if (i + 1 >= tokens.size())
                throw std::runtime_error("Error: expected index string");
            server.setIndex(tokens[i + 1]);
            i += 2;
        }
        else if (tokens[i] == "client_max_body_size")
        {
            if (i + 1 >= tokens.size())
                throw std::runtime_error("Error: expected client_max_body_size value");
            size_t size = getClientMaxBodySize(tokens[i + 1]);
            server.setClientMaxBodySize(size);
            i += 2;
        }
        else if (tokens[i] == "error_page")
        {
            if (i + 2 >= tokens.size())
                throw std::runtime_error("Error: expected error_page code and path");   
            if (isDigits(tokens[i + 1]) == false)
                throw std::runtime_error("Error: error_page code must be numeric");
            std::istringstream ss(tokens[i + 1]);
            int code;
            if (!(ss >> code) || code < 400 || code > 599)
                throw std::runtime_error("Error: invalid error_page code");
            std::string path = tokens[i + 2];
            server.addErrorPage(code, path);
            i += 3;
        }
        else if (tokens[i] == "location")
        {
            if (i + 1 < tokens.size() && tokens[i + 2] == "{")
            {
                LocationConfig location;
                location.setPath(tokens[i + 1]);
                i += 3; // Move past "location <path> {"
                while (i < tokens.size() && tokens[i] != "}")
                {
                    if (tokens[i] == "allowed_methods")
                    {
                        if (i + 1 >= tokens.size())
                            throw std::runtime_error("Error: expected allowed_methods values");
                        std::vector<std::string> methods;
                        size_t j = i + 1;
                        while (j < tokens.size() && tokens[j] != "}" && tokens[j] != "location")
                        {
                            if (tokens[j] == "GET" || tokens[j] == "POST" || tokens[j] == "DELETE")
                                methods.push_back(tokens[j]);
                            else
                                throw std::runtime_error("Error: invalid HTTP method in allowed_methods");
                            ++j;
                        }
                        location.setAllowedMethods(methods);
                        i = j;
                    }
                    else if (tokens[i] == "root")
                    {
                        if (i + 1 >= tokens.size())
                            throw std::runtime_error("Error: expected root string in location");
                        location.setRoot(tokens[i + 1]);
                        i += 2;
                    }
                    else
                    {
                        ++i; // Skip unknown token in location
                    }
                }
                server.addLocation(location);
                if (i < tokens.size() && tokens[i] == "}")
                    ++i; // Move past "}"
            }
            else
            {
                throw std::runtime_error("Error: expected '{' after location");
            }
        }
        else
        {
            ++i; // Skip unknown token
        }
    }
}

void Config::parse (const std::string& filepath)
{
    std::ifstream file(filepath.c_str());
    if (!file.is_open())
        throw std::runtime_error("Error: could not open config file: " + filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    std::vector<std::string> tokens;
    tokens = tokenize(content);
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i] == "server")
        {
            if (i + 1 < tokens.size() && tokens[i + 1] == "{")
            {
                ServerConfig server;
                parseServerBlock(tokens, i, server);
                addServer(server);
            }
            else
            {
                throw std::runtime_error("Error: expected '{' after server");
            }
        }
    }
    file.close();
}