#include <boost/asio.hpp>

inline std::pair<std::string, int> get_ip_port(const boost::asio::ip::tcp::socket& socket) {
	boost::asio::ip::tcp::endpoint remote_endpoint = socket.remote_endpoint();
	std::string ip = remote_endpoint.address().to_string();
	unsigned short port = remote_endpoint.port();

	return { ip, port };
}