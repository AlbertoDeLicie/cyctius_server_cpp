#pragma once
#include <boost/beast.hpp>
#include <unordered_map>
#include "response_traits.h"

using RawResponse = boost::beast::http::response<boost::beast::http::dynamic_body>;

template<typename BodyType>
class HttpResponse {
public:
	HttpResponse(unsigned version, boost::beast::http::status status_code) :
		m_version(version),
		m_status_code(status_code)
	{
	}

	void set_body(BodyType body) {
		m_body = std::move(body);
	}

	void set_header(boost::beast::http::field key, const std::string& value) {
		m_headers[key] = value;
	}

	std::shared_ptr<RawResponse> build() const
	{
		using Serializer = HttpResponseTraits<BodyType>;

		auto response = std::make_shared<RawResponse>(m_status_code, m_version);

		response->set(boost::beast::http::field::server, "CyctiusBoostServer/1.0");

		auto t = std::time(nullptr);
		std::ostringstream oss;
		oss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT");
		response->set(boost::beast::http::field::date, oss.str());

		for (const auto& [key, value] : m_headers) {
			response->set(key, value);
		}
		
		boost::beast::error_code ec;
		boost::beast::ostream(response->body()) << Serializer::serialize(m_body, ec);

		if (ec) {
			return internal_server_error("500 Serialization failed", m_version);
		}

		response->prepare_payload();

		return response;
	}

	template<typename T>
    static std::shared_ptr<RawResponse> ok(T body, unsigned version = 11)
    {
        auto response = std::make_shared<HttpResponse<T>>(version, boost::beast::http::status::ok);
        response->set_body(body);
        return response->build();
    }

	template<typename T>
	static std::shared_ptr<RawResponse> not_found(T body, unsigned version = 11)
	{
		auto response = std::make_shared<HttpResponse<T>>(version, boost::beast::http::status::not_found);
		response->set_body(body);
		return response->build();
	}

	template<typename T>
	static std::shared_ptr<RawResponse> internal_server_error(T body, unsigned version = 11)
	{
		auto response = std::make_shared<HttpResponse<T>>(version, boost::beast::http::status::internal_server_error);
		response->set_body(body);
		return response->build();
	}

	template<typename T>
	static std::shared_ptr<RawResponse> bad_request(T body, unsigned version = 11)
	{
		auto response = std::make_shared<HttpResponse<T>>(version, boost::beast::http::status::bad_request);
		response->set_body(body);
		return response->build();
	}

private:
	unsigned m_version;
	std::unordered_map<boost::beast::http::field, std::string> m_headers;
	boost::beast::http::status m_status_code;
	BodyType m_body;
};