#pragma once
#include "../common/response_traits.h"
#include "../common/http_response.h"

template<typename ExceptionType>
struct ExceptionHandlerTraits {
	using BodyType = std::string;

	std::shared_ptr<RawResponse> handle_exception(const ExceptionType& ex) {
		return HttpResponse<BodyType>::internal_server_error("500 Internal Server Error");
	}
};

template<>
struct ExceptionHandlerTraits<std::bad_alloc> {
	using BodyType = std::string;

	std::shared_ptr<RawResponse> handle_exception(const std::bad_alloc& ex) {
		return HttpResponse<BodyType>::internal_server_error("500 Internal Server Error / Bad alloc");
	}
};