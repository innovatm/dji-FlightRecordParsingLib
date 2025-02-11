#include "helper.h"

#include <iostream>
#include <ctime>
#include <string>

namespace IATM {
    namespace helper {

        std::string formatted_date_time(const std::string &format)
        {
            std::time_t now = std::time(nullptr);
            std::tm now_tm = *std::localtime(&now);

            char buffer[15];
            std::strftime(buffer, sizeof(buffer), format.c_str(), &now_tm);
            return std::string(buffer);
        }

        std::string build_json_filename(const std::string& org_filename) {
            auto out_filename = org_filename;
            size_t pos = out_filename.rfind(".");
            if (pos != std::string::npos) {
                out_filename = out_filename.substr(0, pos + 1) + "json";
            } else {
                out_filename += ".json";
            }
            return out_filename;
        }
    }
}
