#define MCAP_IMPLEMENTATION // Define this in exactly one .cpp file
#include <mcap/writer.hpp>
#include <cargs.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "simple_fusion/simple_fusion.h"
}

static struct cag_option options[] = {
    {
        .identifier = 'p',
        .access_letters = "p",
        .access_name = "plan",
        .value_name = "PLAN_FILE",
        .description = "plan file",
    },
    {.identifier = 'o',
     .access_letters = "o",
     .access_name = "outfile",
     .value_name = "OUTPUT_FILE",
     .description = "Output file"},
};

int main(int argc, char *argv[])
{
    const char *plan_file = NULL;
    const char *outfile = NULL;
    bool write_output = false;

    cag_option_context context;
    cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
    while (cag_option_fetch(&context))
    {
        switch (cag_option_get_identifier(&context))
        {
        case 'p':
            plan_file = cag_option_get_value(&context);
            break;
        case 'o':
            outfile = cag_option_get_value(&context);
            break;
        case '?':
            cag_option_print_error(&context, stdout);
            return EXIT_FAILURE;
        }
    }

    if (!plan_file || !outfile)
    {
        fprintf(stderr, "Usage: %s --plan <plan_file> --outfile <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // set up output
    mcap::Channel outputChannel;
    mcap::McapWriter writer;

    // setup outfile for write
    mcap::McapWriterOptions mcapWriterOptions = mcap::McapWriterOptions("");
    mcapWriterOptions.compression = mcap::Compression::None;
    auto status = writer.open(outfile, mcapWriterOptions);
    if (!status.ok())
    {
        std::cerr << "Failed to open MCAP file for writing: " << status.message << "\n";
        return EXIT_FAILURE;
    }

    // Register a Schema
    json schemaData = json::parse(R"(
{
    "title": "sensor_event",
    "type": "object",
    "properties": {
        "values": {
            "type": "array",
            "items": {
                "type": "number"
            }
        }
    }
}
  )");
    mcap::Schema sensorEventSchema("sensor_event", "jsonschema", schemaData.dump());
    std::cout << schemaData.dump() << '\n';
    writer.addSchema(sensorEventSchema);

    // Register channels
    auto magChannel = mcap::Channel("mag", "json", sensorEventSchema.id);
    writer.addChannel(magChannel);
    auto accChannel = mcap::Channel("acc", "json", sensorEventSchema.id);
    writer.addChannel(accChannel);
    auto gyroChannel = mcap::Channel("gyro", "json", sensorEventSchema.id);
    writer.addChannel(gyroChannel);

    printf("Plan file: %s\n", plan_file);

    // Read the JSON file
    std::ifstream file(plan_file);
    if (!file.is_open())
    {
        std::cerr << "Could not open the plan_file" << std::endl;
        return 1;
    }

    json plan;
    file >> plan;

    // Set struct members from JSON
    if (plan.contains("segments"))
    {
        auto initialization = plan["initialization"];
        Quaternion current_q = {
            initialization["q"]["w"].get<float>(),
            initialization["q"]["x"].get<float>(),
            initialization["q"]["y"].get<float>(),
            initialization["q"]["z"].get<float>()};
        ;
        auto current_time = mcap::Timestamp(initialization["start_time_ns"].get<uint64_t>());
        auto sample_rate = initialization["sample_rate"].get<double>();
        auto sample_interval = mcap::Timestamp(1e9 / sample_rate);

        for (const auto &segment : plan["segments"])
        {
            std::string name = segment["name"].get<std::string>();
            double duration_s = segment["duration_s"].get<double>();
            mcap::Timestamp duration = mcap::Timestamp(duration_s * 1e9); // Convert to nanoseconds

            int samples_in_segment = duration_s * sample_rate;

            // Initialize Quaternion
            auto rotation_rpy_degrees = segment["rotation_rpy_degrees"];
            Quaternion rotation = eulerAnglesToQuaternion({deg2rad(rotation_rpy_degrees["roll"].get<float>() / samples_in_segment),
                                                           deg2rad(rotation_rpy_degrees["pitch"].get<float>() / samples_in_segment),
                                                           deg2rad(rotation_rpy_degrees["yaw"].get<float>() / samples_in_segment)});

            // Print the values to verify
            std::cout << "Segment name: " << name << std::endl;
            std::cout << "Duration: " << duration_s << std::endl;
            std::cout << "Rotation: w=" << rotation.w << ", x=" << rotation.x
                      << ", y=" << rotation.y << ", z=" << rotation.z << std::endl;

            // Define reference vectors (assuming they are [0, 0, 1] and [1, 0, 0] in world frame)
            Vector3 accel_ref = {0.0f, 0.0f, 1.0f};
            Vector3 mag_ref = {1.0f, 1.0f, 0.0f};

            auto segment_end_time = current_time + duration;
            // iterate over the duration and add events
            while (current_time < segment_end_time)
            {
                // rotate
                current_q = multiplyQuaternions(current_q, rotation);

                // mag
                Vector3 mag = rotateVector(mag_ref, current_q);
                nlohmann::json mag_data;
                mag_data["id"] = "mag";
                mag_data["timestamp"] = current_time;
                nlohmann::json mag_values = nlohmann::json::array();
                mag_values.push_back(mag.x);
                mag_values.push_back(mag.y);
                mag_values.push_back(mag.z);
                mag_data["values"] = mag_values;
                std::string mag_serialized = mag_data.dump();
                mcap::Message mag_msg;
                mag_msg.channelId = magChannel.id;
                mag_msg.logTime = current_time;     // Required nanosecond timestamp
                mag_msg.publishTime = current_time; // Set to logTime if not available
                mag_msg.data = reinterpret_cast<const std::byte *>(mag_serialized.data());
                mag_msg.dataSize = mag_serialized.size();
                writer.write(mag_msg);

                // acc
                Vector3 accel = rotateVector(accel_ref, current_q);
                nlohmann::json accel_data;
                accel_data["id"] = "acc";
                accel_data["timestamp"] = current_time;
                nlohmann::json accel_values = nlohmann::json::array();
                accel_values.push_back(accel.x);
                accel_values.push_back(accel.y);
                accel_values.push_back(accel.z);
                accel_data["values"] = accel_values;
                std::string accel_serialized = accel_data.dump();
                mcap::Message accel_msg;
                accel_msg.channelId = accChannel.id;
                accel_msg.logTime = current_time;     // Required nanosecond timestamp
                accel_msg.publishTime = current_time; // Set to logTime if not available
                accel_msg.data = reinterpret_cast<const std::byte *>(accel_serialized.data());
                accel_msg.dataSize = accel_serialized.size();
                writer.write(accel_msg);

                // gyro
                Vector3 gyro = {rotation.x, rotation.y, rotation.z};    
                nlohmann::json gyro_data;
                gyro_data["id"] = "gyro";
                gyro_data["timestamp"] = current_time;
                nlohmann::json gyro_values = nlohmann::json::array();
                gyro_values.push_back(gyro.x);
                gyro_values.push_back(gyro.y);
                gyro_values.push_back(gyro.z);
                gyro_data["values"] = gyro_values;
                std::string gyro_serialized = gyro_data.dump();
                mcap::Message gyro_msg;
                gyro_msg.channelId = gyroChannel.id;
                gyro_msg.logTime = current_time;     // Required nanosecond timestamp
                gyro_msg.publishTime = current_time; // Set to logTime if not available
                gyro_msg.data = reinterpret_cast<const std::byte *>(gyro_serialized.data());
                gyro_msg.dataSize = gyro_serialized.size();
                writer.write(gyro_msg);

                current_time += sample_interval;
            }
        }
    }

    writer.close();

    return EXIT_SUCCESS;
}