cmake_minimum_required(VERSION 3.4.1)

set(current_project_name "FlightLogDecoderApi")
project("${current_project_name}")

set(all_project_root_path "${CMAKE_CURRENT_SOURCE_DIR}/../../../")

add_executable(${current_project_name} main.cc server.cpp decode_flightlog.cpp helper.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -O3 -fPIC -DBOOST_LOG_DYN_LINK")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --std=c++11 -O0")

find_package(Boost REQUIRED COMPONENTS system thread log program_options chrono)
find_package(Threads REQUIRED)
find_package(OpenSSL)
find_package(ZLIB)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -O3 -fPIC -DBOOST_LOG_DYN_LINK")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --std=c++11 -O0")

#set(CPPFLAGS -stdlib=libc++ LDFLAGS -lcpprest -lssl -lcrypto -lboost_system -lboost_thread-mt -lboost_chrono-mt -lpthread -pthread -lz)
set(CMAKE_CXX_STANDARD 11)

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)

# Pour in the File Filter traversal tool.
include(${all_project_root_path}/third-party/cmake/common/file/FileFilter.cmake)

file(GLOB_RECURSE StandardizationCppHeader ${all_project_root_path}/dji-flightrecord-kit/source/FlightRecordStandardizationCpp/*.hpp ${all_project_root_path}/dji-flightrecord-kit/source/FlightRecordStandardizationCpp/*.h ${all_project_root_path}/dji-flightrecord-kit/source/FlightRecordStandardization/*.h)
include_file_directory(StandardizationCppHeader)

add_subdirectory(${all_project_root_path}/dji-flightrecord-kit/source/FlightRecordStandardizationCpp StandardizationCppBuild)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED)

# Import third party - libcpprest
add_library(libcpprest STATIC IMPORTED)
set_target_properties( libcpprest PROPERTIES
    IMPORTED_LOCATION ${all_project_root_path}/third-party/build/libcpprest/lib/ubuntu/libcpprest.a
    INTERFACE_INCLUDE_DIRECTORIES "${all_project_root_path}/third-party/build/libcpprest/include"
    INTERFACE_LINK_LIBRARIES "Boost::system;Boost::thread;Boost::log;Boost::program_options;Boost::chrono;OpenSSL::SSL;OpenSSL::Crypto;ZLIB::ZLIB;Threads::Threads"
)

include_directories(include)
target_link_libraries(${current_project_name} libcpprest FlightRecordStandardizationCpp ${CURL_LIBRARIES})
