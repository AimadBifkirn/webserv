#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>


class LocationConfig {
	private:
		std::string					path;
		std::vector<std::string>	allowed_methods;
		std::string					root;
		std::string					index;
		bool						autoindex;
		std::string					cgi_path;
		std::string					redirect;
	public:

		LocationConfig();

		// setters
		void	setPath (const std::string& path) { this->path = path; };
		void	setAllowedMethods (const std::vector<std::string>& methods) { this->allowed_methods = methods; };
		void	setRoot (const std::string& root) { this->root = root; };
		void	setIndex (const std::string& index) { this->index = index; };
		void	setAutoindex (bool autoindex) { this->autoindex = autoindex; };
		void	setCgiPath (const std::string& cgi_path) { this->cgi_path = cgi_path; };
		void	setRedirect (const std::string& redirect) { this->redirect = redirect; };

		// getters
		const std::string&			getPath () const { return this->path; };
		const std::vector<std::string>&	getAllowedMethods () const { return this->allowed_methods; };
		const std::string&			getRoot () const { return this->root; };
		const std::string&			getIndex () const { return this->index; };
		bool						isAutoindex () const { return this->autoindex; };
		const std::string&			getCgiPath () const { return this->cgi_path; };
		const std::string&			getRedirect () const { return this->redirect; };
};


class ServerConfig {
	private:
		int							port;
		std::string					host;
		std::string					server_name;
		std::string					root;
		std::string					index;
		size_t						client_max_body_size;
		std::map<int, std::string>	error_pages;
		std::vector<LocationConfig>	locations;
	public:

		ServerConfig();

		// setters
		void	setPort (int port) { this->port = port; };
		void	setHost (const std::string& host) { this->host = host; };
		void	setServerName (const std::string& server_name) { this->server_name = server_name; };
		void	setRoot (const std::string& root) { this->root = root; };
		void	setIndex (const std::string& index) { this->index = index; };
		void	setClientMaxBodySize (size_t size) { this->client_max_body_size = size; };
		void	addErrorPage (int code, const std::string& path) { this->error_pages[code] = path; };
		void	addLocation (const LocationConfig& location) { this->locations.push_back(location);};

		// getters
		int					getPort () const { return this->port; };
		const std::string&	getHost () const { return this->host; };
		const std::string&	getServerName () const { return this->server_name; };
		const std::string&	getRoot () const { return this->root; };
		const std::string&	getIndex () const { return this->index; };
		size_t				getClientMaxBodySize () const { return this->client_max_body_size; };
		const std::map<int, std::string>&	getErrorPages () const { return this->error_pages; };
		const std::vector<LocationConfig>&	getLocations () const { return this->locations; };
};


class Config
{
    private:
        std::vector<ServerConfig> servers; // the servers
    public:
        void addServer(const ServerConfig& server) { servers.push_back(server); };
        const std::vector<ServerConfig>& getServers() const { return this->servers; };
        bool parse(const std::string& filepath);
        void printConfig() const; // For debugging
};

extern Config globalConfig;