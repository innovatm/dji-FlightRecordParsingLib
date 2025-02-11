#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>

using namespace std;

namespace IATM {
    namespace constants {
        constexpr size_t buffer_size  { 8192 };
        const char * temp_filename_prefix { "flightlog" };
    }
}
#endif