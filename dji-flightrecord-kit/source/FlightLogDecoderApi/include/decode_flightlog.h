#ifndef DECODE_FLIGHTLOG_H
#define DECODE_FLIGHTLOG_H

#include <string>

namespace IATM {
    namespace flightlog {
        void decode_flight_log(const std::string &file_path);
    }
}

#endif