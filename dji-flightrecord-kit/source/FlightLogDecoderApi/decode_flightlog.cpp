#include "helper.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include "DJIFRProtoParser.hpp"
#include <assert.h>
#include <thread>
#include <unistd.h>
#include <google/protobuf/util/json_util.h>

#define DEPARTMENT_SDK 4;
#define DEPARTMENT_APP 1;

using namespace IATM::helper;

namespace IATM
{
    namespace flightlog
    {
        void decode_flight_log(const std::string &file_path) {
            int departmentType = DEPARTMENT_SDK;
            auto parser = std::make_shared<DJIFRProto::Standard::Parser>();
            auto result = parser->load(file_path);
            if (result != DJIFRProto::Standard::Success) {
                printf("load file failed");
            }

            result = parser->startRequestParser(getenv("SDK_KEY"), departmentType, [&parser, &file_path](DJIFR::standardization::ServerError error_code, const std::string &error_description) {
                if (error_code == DJIFR::standardization::ServerError::Success) {
                    std::shared_ptr<DJIFRProto::Standard::SummaryInformation> info = nullptr;
                    parser->summaryInformation(&info);

                    std::string summary_proto_json_string = "";
                    std::string info_proto_json_string = "";

                    google::protobuf::util::JsonPrintOptions options;
                    options.add_whitespace = true;
                    options.always_print_primitive_fields = true;
                    options.preserve_proto_field_names = true;

                    if (!google::protobuf::util::MessageToJsonString(*info, &summary_proto_json_string, options).ok()) {
                        summary_proto_json_string = "{}";
                    }

                    std::shared_ptr<DJIFRProto::Standard::FrameTimeStates> frame_time_list;
                    parser->frame_time_states(&frame_time_list);
                    if (!google::protobuf::util::MessageToJsonString(*frame_time_list, &info_proto_json_string, options).ok()) {
                        info_proto_json_string = "{}";
                    }

                    auto json_filename = helper::build_json_filename(file_path);
                    printf("Exporting JSON file: %s\n", json_filename.c_str());

                    std::ofstream output_file(json_filename);
                    if (!output_file.is_open()) {
                        printf("Failed to open the file for conversion: %s\n", json_filename.c_str());
                    }

                    output_file << "{\"summary\": " << summary_proto_json_string << ", \"info\": " << info_proto_json_string << "}";

                    if (!output_file) {
                        printf("Error writing to the file: %s\n", json_filename.c_str());
                        output_file.close();
                    }
                    output_file.close();
                    printf("File saved successfully");

                } else {
                    printf("error code: %d decription: %s\n", (int)error_code, error_description.c_str());
                }
            });
        }
    }
}