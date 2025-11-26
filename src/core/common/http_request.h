#pragma once
#include <string>
#include <map>
#include <boost/url.hpp>
#include "content_traits.h"
#include <boost/algorithm/string.hpp>

template<typename ContentTypeTag>
struct BasicHttpRequest {
    unsigned http_version = 11;
    std::string uri;
    std::string endpoint;
    std::map<boost::beast::http::field, std::string> headers;
    std::map<std::string, std::string> query_params;
    boost::beast::http::verb verb = boost::beast::http::verb::unknown;
    std::string content_type_header;
};

template<typename ContentTypeTag>
struct HttpRequest : public BasicHttpRequest<ContentTypeTag> {
    using BodyType = typename ContentTraits<ContentTypeTag>::BodyType;
    BodyType body;
};

template<typename ContentTypeTag>
inline std::shared_ptr<HttpRequest<ContentTypeTag>> from_request(const boost::beast::http::request<boost::beast::http::dynamic_body>& req)
{
    using Traits = ContentTraits<ContentTypeTag>;
    using BodyType = typename Traits::BodyType;

    auto httpRequest = std::make_shared<HttpRequest<ContentTypeTag>>();

    boost::urls::url_view url(req.target());

    httpRequest->uri = std::string(req.target());
    httpRequest->endpoint = std::string(url.path());
    httpRequest->http_version = req.version();
    httpRequest->verb = req.method();
    
    if (req.find(boost::beast::http::field::content_type) != req.end()) {
        httpRequest->content_type_header = req[boost::beast::http::field::content_type].data();
    }

    for (const auto& field : req) {
        httpRequest->headers[field.name()] = field.value().data();
    }

    for (auto const& p : url.params()) {
        httpRequest->query_params.emplace(std::string(p.key), std::string(p.value));
    }

    httpRequest->body = Traits::parse(req);

    return httpRequest;
}
