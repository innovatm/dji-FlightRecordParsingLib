#include "server.h"
#include "constants.h"
#include "helper.h"
#include "decode_flightlog.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cpprest/filestream.h>
#include <cpprest/rawptrstream.h>
#include <cpprest/producerconsumerstream.h>
#include <cpprest/astreambuf.h>

using namespace std;
using namespace utility;
using namespace web;
using namespace http;
using namespace web::http::experimental::listener;
using namespace concurrency::streams;

using namespace IATM;
using namespace IATM::constants;
using namespace IATM::helper;
using namespace IATM::flightlog;

ApiServer::ApiServer(std::string endpoint): m_endpoint(endpoint) {}

std::string ApiServer::getEndpoint() { return m_endpoint; }

void ApiServer::start() {
    http_listener listener(U(m_endpoint));

    listener.support(methods::GET, bind(&ApiServer::get_hello, this, placeholders::_1));
    listener.support(methods::POST, bind(&ApiServer::post_decode, this, placeholders::_1));
    
    try {
        listener.open().wait();
        std::cout << "Listening on " << m_endpoint << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::hours(24));
        }
        std::cout << "Listening stopped" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void ApiServer::get_hello(http_request message) {
    auto path = uri::split_path(uri::decode(message.relative_uri().path()));
    if (path[0] == "hello") {
        auto queryParameters = uri::split_query(message.request_uri().query());
        utility::string_t name = queryParameters[U("name")];

        json::value response;
        response["message"] = json::value::string(U("Hello, " + name + U("!")));

        message.reply(status_codes::OK, response);
    } else {
        message.reply(status_codes::NotFound);
    }
}

void ApiServer::post_decode(http_request request) {
    auto path = uri::split_path(uri::decode(request.relative_uri().path()));
    if (path[0] == "decode") {
        const std::string filename = path[1] + std::string(".txt");
        ApiServer::copy_file_task(request.body(), filename).then([=]() {
            IATM::flightlog::decode_flight_log(filename);
        }).then([=]() {
            auto json_filename = build_json_filename(filename);
            return concurrency::streams::file_stream<uint8_t>::open_istream(json_filename).then([=](concurrency::streams::istream stream) {
                http_response response(status_codes::OK);
                response.set_body(stream);
                auto length = stream.seek(0, std::ios_base::end);
                stream.seek(0);
                response.headers().set_content_type(U("text/plain; charset=utf-8"));
                response.headers().set_content_length((size_t)length);
                request.reply(response).get();
            });
        }).wait();

    } else {
        request.reply(status_codes::NotFound);
    }
}

pplx::task<void> ApiServer::copy_file_task(const concurrency::streams::istream &instream, const std::string &filename) {
    return concurrency::streams::fstream::open_ostream(filename, std::ios::out | std::ios::binary).then([=](concurrency::streams::ostream outstream) {
        for (;;) {
            uint8_t buffer[buffer_size];
            concurrency::streams::rawptr_buffer<uint8_t> buf1(buffer, buffer_size, std::ios::out | std::ios::binary);
            concurrency::streams::rawptr_buffer<uint8_t> buf2(buffer, buffer_size, std::ios::in | std::ios::binary);
            size_t bytes_read = instream.read(buf1, buffer_size).get();
            outstream.write(buf2, bytes_read).get();
            outstream.flush().get();

            if (bytes_read != buffer_size) break;
        }
        return outstream.close().then([=] {
            return instream.close();
        });
    });
}

pplx::task<void> ApiServer::write_stream_to_file_task(const concurrency::streams::istream &instream, const std::string &filename) {
    return concurrency::streams::fstream::open_ostream(filename, std::ios::out | std::ios::binary).then([=](concurrency::streams::ostream outstream) {
        return do_while([=]() {
            uint8_t buffer[buffer_size];
            concurrency::streams::rawptr_buffer<uint8_t> buf1(buffer, buffer_size, std::ios::out | std::ios::binary);
            concurrency::streams::rawptr_buffer<uint8_t> buf2(buffer, buffer_size, std::ios::in | std::ios::binary);
            return instream.read(buf1, buffer_size).then([=] (size_t read) {
                if (read > 0) {
                    return outstream.write(buf2, read).then([=](size_t) {
                        return pplx::task_from_result(true);
                    });
                } else {
                    return pplx::task_from_result(false);
                }
            });
        }).then([=]() {
            instream.close().then([=]() {
                outstream.close();
            });
        });
    });
}

pplx::task<void> ApiServer::do_while(std::function<pplx::task<bool>(void)> func) {
    return _do_while_impl(func).then([](bool) {});
}

pplx::task<bool> ApiServer::_do_while_impl(std::function<pplx::task<bool>(void)> func) {
    return _do_while_iteration(func).then([=](bool guard) -> pplx::task<bool> {
        if(guard) {
            return _do_while_impl(func);
        } else {
            return pplx::task_from_result(false);
        }
    });
}

pplx::task<bool> ApiServer::_do_while_iteration(std::function<pplx::task<bool>(void)> func) {
    pplx::task_completion_event<bool> ev;
    func().then([=](bool guard) {
        ev.set(guard);
    });
    return pplx::create_task(ev);
}
