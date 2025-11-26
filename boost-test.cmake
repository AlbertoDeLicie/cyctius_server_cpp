if(WIN32)
    set(Boost_USE_STATIC_LIBS ON)
else()
    set(Boost_USE_STATIC_LIBS OFF)
    add_definitions(-DBOOST_ALL_DYN_LINK)
endif()

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(Boost REQUIRED COMPONENTS asio)
find_package(Boost REQUIRED COMPONENTS beast)
find_package(Boost REQUIRED COMPONENTS url)
find_package(spdlog CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

find_package(nlohmann_json CONFIG REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${nlohmann_json_INCLUDE_DIRS})
