#ifndef SERVER_H
#define SERVER_H

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/filestream.h>
#include <string>

using namespace concurrency::streams;

namespace IATM {
    class ApiServer {
    private:
        std::string m_endpoint;

        void get_hello(web::http::http_request request);

        void post_decode(web::http::http_request request);

        pplx::task<void> copy_file_task(const concurrency::streams::istream &instream, const std::string &filename);

        pplx::task<void> write_stream_to_file_task(const concurrency::streams::istream &instream, const std::string &filename);

        pplx::task<void> do_while(std::function<pplx::task<bool>(void)> func);

        pplx::task<bool> _do_while_iteration(std::function<pplx::task<bool>(void)> func);

        pplx::task<bool> _do_while_impl(std::function<pplx::task<bool>(void)> func);

    public:
        ApiServer(std::string endpoint);

        std::string getEndpoint();

        void start();
    };
}
#endif