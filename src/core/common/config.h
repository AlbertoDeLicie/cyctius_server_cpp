#pragma once
#include <string>
#include <unordered_map>

struct ServerConfig {
	unsigned int port;
	std::string server_name;
	unsigned int idle_time;
	unsigned int max_requests_to_one_connect;
	unsigned int max_package_size;

	bool is_https;

	std::string tls_certs_path;

	std::string db_url;
	std::string db_password;
	std::string db_login;

	std::string cache_url;
	std::string cache_password;
	std::string cache_login;

	std::unordered_map<std::string, std::string> custom_parameters;
};