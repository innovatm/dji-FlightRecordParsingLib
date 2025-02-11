#ifndef HELPER_H
#define HELPER_H

#include <string>

namespace IATM {
    namespace helper {

        std::string formatted_date_time(const std::string& format);

        std::string build_json_filename(const std::string& org_filename);
    }
}

#endif