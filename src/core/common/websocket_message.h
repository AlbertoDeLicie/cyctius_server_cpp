#pragma once
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include "websocket_content_traits.h"

template<typename InputType>
inline std::shared_ptr<WebsocketRawMessage> to_message(const InputType& input) {
	using Traits = WebsocketContentTraits<InputType>;

	boost::beast::error_code code;
	auto raw_message = Traits::serialize(input, code);

	if (code.failed())
		return nullptr;

	return std::make_shared<WebsocketRawMessage>(std::move(raw_message));
}

template<typename InputType>
inline std::shared_ptr<InputType> from_message(const WebsocketRawMessage& raw_message) {
	using Traits = WebsocketContentTraits<InputType>;
	using MessageBody = typename Traits::MessageBodyType;

	boost::beast::error_code code;
	auto res = Traits::deserialize(raw_message, code);

	if (code.failed())
		return nullptr;

	return std::make_shared<InputType>(res);
}