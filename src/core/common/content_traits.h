#pragma once

#include <string_view>
#include <nlohmann/json.hpp>
#include <boost/beast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <vector>

template<typename ContentType>
struct ContentTraits {
    using BodyType = std::string; // safe default

    // non-template parse Ч всегда одна и та же сигнатура
    static BodyType parse(const boost::beast::http::request<boost::beast::http::dynamic_body>& req) {
        return boost::beast::buffers_to_string(req.body().data());
    }

    static constexpr const char* name = "unknown/unknown";
};

struct JsonContent { static constexpr const char* name = "application/json"; };
struct TextContent { static constexpr const char* name = "text/plain"; };
struct BinaryContent { static constexpr const char* name = "application/octet-stream"; };
struct IntContent { static constexpr const char* name = "text/int"; };
struct XWWWFormUrlencoded { static constexpr const char* name = "application/x-www-form-urlencoded"; };

template<>
struct ContentTraits<JsonContent> {
    using BodyType = nlohmann::json;

    static BodyType parse(const boost::beast::http::request<boost::beast::http::dynamic_body>& req) {
        if (req.body().size() == 0) return BodyType{};
        try {
            return nlohmann::json::parse(boost::beast::buffers_to_string(req.body().data()));
        }
        catch (nlohmann::json::exception&) {
            return BodyType{};
        }
    }

    static constexpr const char* name = JsonContent::name;
};

template<>
struct ContentTraits<TextContent> {
    using BodyType = std::string;

    static BodyType parse(const boost::beast::http::request<boost::beast::http::dynamic_body>& req) {
        return boost::beast::buffers_to_string(req.body().data());
    }

    static constexpr const char* name = TextContent::name;
};

template<>
struct ContentTraits<BinaryContent> {
    using BodyType = std::vector<uint8_t>;

    static BodyType parse(const boost::beast::http::request<boost::beast::http::dynamic_body>& req) {
        std::vector<uint8_t> out;

        const auto& buffers = req.body().data();

        std::size_t total_size = req.body().size();
        out.reserve(total_size);
        
        for (auto const& buffer : buffers) {
            const uint8_t* data = static_cast<const uint8_t*>(buffer.data());
            std::size_t size = buffer.size();
            out.insert(out.end(), data, data + size);
        }

        return out;
    }

    static constexpr const char* name = BinaryContent::name;
};

template<>
struct ContentTraits<XWWWFormUrlencoded> {
    using BodyType = std::unordered_multimap<std::string, std::string>;

    static BodyType parse(const boost::beast::http::request<boost::beast::http::dynamic_body>& req) {
        BodyType out;

        std::string body_string = boost::beast::buffers_to_string(req.body().data());

        std::vector<std::string> pairs;
        boost::algorithm::split(pairs, body_string, boost::algorithm::is_any_of("&"), boost::algorithm::token_compress_on);

        for (const std::string& pair : pairs) {
            std::vector<std::string> kv;
            boost::algorithm::split(kv, pair, boost::algorithm::is_any_of("="));

            if (kv.size() == 2) {
                out.emplace(kv[0], kv[1]);
            }
            else if (kv.size() == 1 && !kv[0].empty()) {
                out.emplace(kv[0], "");
            }
        }

        return out;
    }

    static constexpr const char* name = XWWWFormUrlencoded::name;
};


