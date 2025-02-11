#include <server.h>

using namespace IATM;

int main() {
    /*
    * Start API server
    * ATTENTION !! filling local endpoint http://localhost:8080/api does not work at all.
    */
    ApiServer* server = new ApiServer("http://0.0.0.0:8080/api");
    server->start();

    return 0;
}