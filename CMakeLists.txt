cmake_minimum_required (VERSION 3.6)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "/bigobj ${CMAKE_CXX_FLAGS}")

include(./boost-test.cmake)

add_executable(
	cyctius 
	#"src/test.cpp"
	"src/main_coro.cpp"

	"src/core/async/http_server.h" 
	"src/core/async/http_session.h" 
	"src/core/async/http_session.cpp" 
	"src/core/async/http_server.cpp"
	"src/core/async/router.h"
	"src/core/async/router.cpp" 

	"src/core/coro/http_server_coro.h" 
	"src/core/coro/http_server_coro.cpp" 
	"src/core/coro/http_session_coro.cpp"
	"src/core/coro/http_session_coro.h"
	"src/core/coro/router_coro.h" 
	"src/core/coro/router_coro.cpp"
	"src/core/coro/websocket_session.h" 
	"src/core/coro/websocket_session.cpp" 
	"src/core/coro/detect_session.h"
	"src/core/coro/detect_session.cpp"

	"src/core/db/pg_session.h" 
	"src/core/db/pg_mapper.h" 
	"src/core/db/pg_session.cpp"
	"src/core/db/pg_data_base.h"
	"src/core/db/pg_data_base.cpp"

	"src/core/common/http_request.h" 
	"src/core/common/http_response.h" 
	"src/core/common/content_traits.h" 
	"src/core/common/http_response.cpp"
	"src/core/common/route.h" 

	"src/core/common/logger.cpp"
	"src/core/common/logger.h"

	"src/core/common/config.h" 

	"src/core/common/socket_utils.h"
	"src/core/common/response_traits.h"

	"src/hello_route.h"
	"src/hello_route.cpp"
)

target_link_libraries(cyctius ${Boost_LIBRARIES})
target_link_libraries(cyctius nlohmann_json::nlohmann_json)
target_link_libraries(cyctius spdlog::spdlog)
target_link_libraries(cyctius libpqxx::pqxx)

