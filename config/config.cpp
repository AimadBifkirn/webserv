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
static bool isDigits(const std::string& s)
{
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (!std::isdigit(s[i])) return false;
    }
    return true;
}

// helper function to parse client_max_body_size value
static size_t getClientMaxBodySize(const std::string &token)
{
    size_t size = 0;
    if (!token.empty() && token[token.size() - 1] == ';' && (token[token.size() - 2] == 'M' || token[token.size() - 2] == 'm'))
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

//helper function bach njib IP mn listen directive
static std::string getIP(const std::string &token)
{
    std::string ip = "";
    size_t i = 0;

    if (token.find(':') != std::string::npos) // ila 3tani ip + port
    {
        if (token[i] == ':')
            throw std::runtime_error("Error: missing IP address before ':'");
        std::stringstream ss;
        int count = 0;
        int dot_count = 0;
        while (i < token.size() && token[i] != ':')
        {
            if ((i == 0 && token[i] == '.') || (i > 0 && token[i] == '.' && token[i - 1] == '.')) // check double dots and dot at start
                throw std::runtime_error("Error: invalid IP address format");

            if (std::isdigit(token[i]))
            {
                ss << token[i];
                count++;
                if (count > 3)
                    throw std::runtime_error("Error: invalid IP address format");
                if (i + 1 < token.size() && (token[i + 1] == '.' || token[i + 1] == ':'))
                {
                    if (token[i + 1] == '.')
                        dot_count++;
                    if (dot_count > 3)
                        throw std::runtime_error("Error: invalid IP address format  here");
                    int octet;
                    if (!(ss >> octet) || (octet < 0 || octet > 255))
                        throw std::runtime_error("Error: invalid IP address octet");
                    ip += ss.str();
                    ip += '.';
                    ss.str(""); // .str("") katbdel contenu dyal ss b string khawi
                    ss.clear(); // katheyd error flags bhal eof ola fail, bach imkenlik tsta3mlha mra khra
                    count = 0;
                    if (token[i + 1] != ':')
                        i++;
                }
            }
            else
                throw std::runtime_error("Error: invalid character in Host IP address");
            ++i;
        }
        if (dot_count != 3 || token[i - 1] == '.')
            throw std::runtime_error("Error: invalid IP address format");
        return ip.substr(0, ip.size() - 1); // katheyd akher '.' li tzadt
    }
// ila 3tani gha port
    return "0.0.0.0"; // default ip
}

//helper function bach njib port mn listen directive
static int getPort(const std::string &token)
{
    int port = 0;
    size_t pos;
    if ((pos = token.find(':')) != std::string::npos)
        pos += 1; // bach ifot ':'
    else
        pos = 0;
    std::istringstream ss(token.substr(pos)); // ila madrnach argument tani by default kay3tik tal lkher d string.
    if (!(ss >> port) || port < 1 || port > 65535)
        throw std::runtime_error("Error: invalid port number");
    return port;
}

// helper function: kanparsi biha l block dyak location
static void parseLocationBlock(const std::vector<std::string>& tokens, size_t& i, LocationConfig& location)
{
    location.setPath(tokens[i + 1]);
    i += 3; // bach index ifot "location <path> {"
    while (i < tokens.size() && tokens[i] != "}")
    {
        if (tokens[i] == "allowed_methods")
        {
            if (i + 1 >= tokens.size())
                throw std::runtime_error("Error: expected allowed_methods values");
            std::vector<std::string> methods;
            size_t j = i + 1;
            int breaker = 0;
            while (j < tokens.size() && !breaker)
            {
                std::string method = tokens[j];
                if (tokens[j][tokens[j].size() - 1] == ';')
                {
                    method = method.substr(0, tokens[j].size() - 1);
                    breaker = 1;
                }
                if (method == "GET" || method == "POST" || method == "DELETE")
                    methods.push_back(method);
                else
                    throw std::runtime_error("Error: invalid HTTP method in allowed_methods");
                ++j;
            }
            if (!breaker)
                throw std::runtime_error("Error: expected ';' at the end of allowed_methods directive");
            location.setAllowedMethods(methods);
            i = j;
        }
        else if (tokens[i] == "root")
        {
            if (i + 1 >= tokens.size())
                throw std::runtime_error("Error: expected root string in location");
            location.setRoot(tokens[i + 1].substr(0, tokens[i + 1].size() - 1)); // heyd ';'
            i += 2;
        }
        else if (tokens[i] == "index")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1][tokens[i + 1].size() - 1] != ';')
                throw std::runtime_error("Error: expected index string in location + ';'");
            location.setIndex(tokens[i + 1].substr(0, tokens[i + 1].size() - 1)); // heyd ';'
            i += 2;
        }
        else if (tokens[i] == "autoindex")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1][tokens[i + 1].size() - 1] != ';')
                throw std::runtime_error("Error: expected autoindex value in location + ';'");
            if (tokens[i + 1] == "on;")
                location.setAutoindex(true);
            else if (tokens[i + 1] == "off;")
                location.setAutoindex(false);
            else
                throw std::runtime_error("Error: invalid autoindex value, expected 'on' or 'off'");
            i += 2;
        }
        else if (tokens[i] == "cgi_path")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1][tokens[i + 1].size() - 1] != ';')
                throw std::runtime_error("Error: expected cgi_path string in location + ';'");
            location.setCgiPath(tokens[i + 1].substr(0, tokens[i + 1].size() - 1)); // heyd ';'
            i += 2;
        }
        else if (tokens[i] == "return")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1][tokens[i + 1].size() - 1] != ';')
                throw std::runtime_error("Error: expected redirect string in location + ';'");
            location.setRedirect(tokens[i + 1].substr(0, tokens[i + 1].size() - 1)); // heyd ';'
            i += 2;
        }
        else
            throw std::runtime_error("Error: unknown directive in location block: " + tokens[i]);
    }
}

void ServerConfig::setDefaultRootAndIndexForLocations()
{
    for (size_t i = 0; i < locations.size(); ++i)
    {
        if (locations[i].getRoot().empty())
            locations[i].setRoot(this->root);
        if (locations[i].getIndex().empty())
            locations[i].setIndex(this->index);
    }
}

static void parseServerBlock(const std::vector<std::string>& tokens, size_t& i, ServerConfig& server)
{
    i += 2; // index ifot "server {"
    while (i < tokens.size() && tokens[i] != "}")
    {
        if (tokens[i] == "listen")
        {
            if (tokens[i + 1][tokens[i + 1].size() - 1] != ';')
                throw std::runtime_error("Error: expected ';' at the end of listen directive");
            server.setHost(getIP(tokens[i + 1].substr(0, tokens[i + 1].size() - 1))); // kantrimi tokens[i + 1] bach nhiyed ';'
            server.setPort(getPort(tokens[i + 1].substr(0, tokens[i + 1].size() - 1)));
            i += 2;
        }
        else if (tokens[i] == "server_name")
        {
            if (i + 1 >= tokens.size())
                throw std::runtime_error("Error: expected server_name string");
            server.setServerName(tokens[i + 1].substr(0, tokens[i + 1].size() - 1)); // heyd ';'
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
            std::string path = tokens[i + 2].substr(0, tokens[i + 2].size() - 1); // heyd ';'
            server.addErrorPage(code, path);
            i += 3;
        }
        else if (tokens[i] == "location")
        {
            if (i + 1 < tokens.size() && tokens[i + 2] == "{")
            {
                LocationConfig location;
                parseLocationBlock(tokens, i, location);
                server.addLocation(location);
                if (i < tokens.size() && tokens[i] == "}")
                    ++i; // nfot "}"
                else
                    throw std::runtime_error("Error: expected '}' at the end of location block");
            }
            else
                throw std::runtime_error("Error: expected '{' after location path");
        }
        else
            throw std::runtime_error("Error: unknown directive in server block: " + tokens[i]);
    }
    if (i < tokens.size() && tokens[i] != "}")
        throw std::runtime_error("Error: expected '}' at the end of server block");

    for (size_t j = 0; j < server.getLocations().size(); ++j) // hna kan3iyt l function likat3ti default root w index l locations ila mat3tawch f config file
    {
        server.setDefaultRootAndIndexForLocations();
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
                if (server.getPort() == -1 || server.getRoot().empty())
                    throw std::runtime_error("Error: missing required server directives (listen, root)");
                addServer(server);
            }
            else
            {
                throw std::runtime_error("Error: expected '{' after server");
            }
        }
        else
        {
            throw std::runtime_error("Error: unknown directive outside server block: " + tokens[i]);
        }
    }
    if (tokens.size() == 0)
        throw std::runtime_error("Error: empty config file");
    if (servers.size() == 0)
        throw std::runtime_error("Error: no server blocks found in config file");
    file.close();
}

void Config::printConfig() const
{
    std::cout << servers.size() << " server(s) configured.\n\n";
    for (size_t i = 0; i < servers.size(); i++)
    {
        const ServerConfig& server = servers[i];
        std::cout << "Server " << i + 1 << ":\n";
        std::cout << "  Host: " << server.getHost() << "\n";
        std::cout << "  Port: " << server.getPort() << "\n";
        std::cout << "  Server Name: " << server.getServerName() << "\n";
        std::cout << "  Root: " << server.getRoot() << "\n";
        std::cout << "  Index: " << server.getIndex() << "\n";
        std::cout << "  Client Max Body Size: " << server.getClientMaxBodySize() << "\n";
        std::cout << "  Error Pages:\n";
        const std::map<int, std::string>& error_pages = server.getErrorPages();
        for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it)
        {
            std::cout << "    " << it->first << " -> " << it->second << "\n";
        }
        const std::vector<LocationConfig>& locations = server.getLocations();
        for (size_t j = 0; j < locations.size(); j++)
        {
            const LocationConfig& location = locations[j];
            std::cout << "  Location " << j + 1 << ":\n";
            std::cout << "    Path: " << location.getPath() << "\n";
            std::cout << "    Allowed Methods: ";
            const std::vector<std::string>& methods = location.getAllowedMethods();
            for (size_t k = 0; k < methods.size(); k++)
            {
                std::cout << methods[k] << " ";
            }
            std::cout << "\n";
            std::cout << "    Root: " << location.getRoot() << "\n";
            std::cout << "    Index: " << location.getIndex() << "\n";
            std::cout << "    Autoindex: " << (location.isAutoindex() ? "on" : "off") << "\n";
            std::cout << "    CGI Path: " << location.getCgiPath() << "\n";
            std::cout << "    Redirect: " << location.getRedirect() << "\n";
        }
        std::cout << std::endl;
    }
}