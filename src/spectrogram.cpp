#define MCAP_IMPLEMENTATION // Define this in exactly one .cpp file
#include <mcap/writer.hpp>
#include <mcap/reader.hpp>
#include <cargs.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <chrono>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "nst_main.h"
#include "nelder_mead/nelder_mead.h"

static struct cag_option options[] = {
    {.identifier = 'i',
     .access_letters = "i",
     .access_name = "infile",
     .value_name = "INPUT_FILE",
     .description = "Input file"},

    {.identifier = 'o',
     .access_letters = "o",
     .access_name = "outfile",
     .value_name = "OUTPUT_FILE",
     .description = "Output file"},

    {.identifier = 'p',
     .access_letters = "p",
     .access_name = "paramfile",
     .value_name = "PARAM_FILE",
     .description = "Parameter file"},

    {.identifier = 's',
     .access_letters = "s",
     .access_name = "start_time",
     .value_name = "START_TIME",
     .description = "Start time"},

    {.identifier = 'e',
     .access_letters = "e",
     .access_name = "end_time",
     .value_name = "END_TIME",
     .description = "End time"}};

std::function<void(int, point_t *, const void *)> global_cost_function;

fun_t cost_function = [](int n, point_t *point, const void *data)
{
    global_cost_function(n, point, data);
};

int main(int argc, char *argv[])
{
    const char *infile = NULL;
    const char *outfile = NULL;
    const char *param_file = NULL;
    bool write_output = false;

    cag_option_context context;
    cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
    while (cag_option_fetch(&context))
    {
        switch (cag_option_get_identifier(&context))
        {
        case 'i':
            infile = cag_option_get_value(&context);
            break;
        case 'o':
            outfile = cag_option_get_value(&context);
            break;
        case 'p':
            param_file = cag_option_get_value(&context);
            break;
        case '?':
            cag_option_print_error(&context, stdout);
            return EXIT_FAILURE;
        }
    }

    // Check if both infile and outfile are provided
    if (!infile || !outfile)
    {
        fprintf(stderr, "Usage: %s --infile <input_file> --outfile <output_file>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    // Your logic using infile, outfile
    printf("Input file: %s\n", infile);
    printf("Output file: %s\n", outfile);

    mcap::McapReader reader;
    {
        const auto res = reader.open(infile);
        if (!res.ok())
        {
            std::cerr << "Failed to open " << infile << " for reading: " << res.message
                      << std::endl;
            return 1;
        }
    }

    {
        algorithm_init();

        if (param_file)
        {
            // Read the JSON file
            std::ifstream file(param_file);
            if (!file.is_open())
            {
                std::cerr << "Could not open the file!" << std::endl;
                return 1;
            }

            json j;
            file >> j;

            // Set struct members from JSON
            if (j.contains("x_scale"))
            {
                nst_data.x_scale = j["x_scale"].get<double>();
            }

            // Print the value to verify
            std::cout << "x_scale: " << nst_data.x_scale << std::endl;
        }
    }
    const auto onProblem = [](const mcap::Status &status)
    {
        std::cerr << "Status " + std::to_string((int)status.code) + ": " + status.message;
    };

    mcap::ReadMessageOptions options;
    options.topicFilter = [](std::string_view topic)
    {
        return topic.find("lpom-1") != std::string::npos;
    };
    auto messageView = reader.readMessages(onProblem, options);

    // Define the inline lambda function
    global_cost_function = [&messageView, &reader, &outfile, &write_output](int n, point_t *point, const void *data)
    {
        nst_event_t input_event;
        nst_event_t output_events[4];
        int output_events_count = 0;
        nst_data_t *nst_data = (nst_data_t *)data;

        // set the x_scale value
        nst_data->x_scale = point->x[0];

        // initialize fx
        point->fx = 0;

        // Declare the writer to be used if write_output is true
        mcap::Channel outputChannel;
        mcap::McapWriter writer;
        if (write_output)
        {
            // setup outfile for write
            mcap::McapWriterOptions mcapWriterOptions = mcap::McapWriterOptions("");
            mcapWriterOptions.compression = mcap::Compression::None;
            auto status = writer.open(outfile, mcapWriterOptions);
            if (!status.ok())
            {
                std::cerr << "Failed to open MCAP file for writing: " << status.message << "\n";
                return;
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

            // Register a Channel
            outputChannel = mcap::Channel("a-fort", "json", sensorEventSchema.id);
            writer.addChannel(outputChannel);
        }

        for (auto it = messageView.begin(); it != messageView.end(); it++)
        {

            if (it->channel->messageEncoding != "json")
            {
                continue;
            }
            std::string_view asString(reinterpret_cast<const char *>(it->message.data),
                                      it->message.dataSize);

            auto parsed = nlohmann::json::parse(asString, nullptr, false);

            if (parsed.is_discarded())
            {
                std::cerr << "failed to parse JSON: " << asString << std::endl;
                reader.close();
                return;
            }
            if (!parsed.is_object())
            {
                std::cerr << "unexpected non-object message: " << asString << std::endl;
            }

            // 1) Shift in new raw data value
            size_t values_count = parsed["values"].size();
            int id = 0;
            std::string idStr = parsed["id"];

            if (idStr == "lpom-1")
            {
                // magnetometer
                id = 2;
            }
            else if (idStr == "lpom-15")
            {
                // accelerometer
                id = 1;
            }
            else if (idStr == "lpom-62")
            {
                // gyro
                id = 4;
            }
            else
            {
                std::cerr << "unexpected id " << idStr << std::endl;
            }
            for (int i = 0; i < values_count; i++)
            {
                input_event.id = id;
                input_event.values[i] = parsed["values"][i];
                input_event.values_count = values_count;
                input_event.timestamp = parsed["timestamp"];
            }

            output_events_count = 0;
            algorithm_update(input_event, output_events, &output_events_count);

            for (int output_index = 0; output_index < output_events_count; output_index++)
            {
                if (output_events[output_index].id == 1000)
                {
                    point->fx += abs(output_events[output_index].values[0]);
                }
                if (write_output)
                {
                    json payload;
                    payload["id"] = "mag_threshold";
                    payload["timestamp"] = parsed["timestamp"];
                    // Create a dynamic array of numbers and assign it to a field in the JSON object
                    nlohmann::json values = nlohmann::json::array();
                    for (int i = 0; i < output_events[output_index].values_count; ++i)
                    {
                        values.push_back(output_events[output_index].values[i]);
                    }
                    payload["values"] = values;
                    std::string serialized = payload.dump();

                    // Write our message
                    mcap::Message msg;
                    msg.channelId = outputChannel.id;
                    msg.sequence = it->message.sequence;       // Optional
                    msg.logTime = it->message.logTime;         // Required nanosecond timestamp
                    msg.publishTime = it->message.publishTime; // Set to logTime if not available
                    msg.data = reinterpret_cast<const std::byte *>(serialized.data());
                    msg.dataSize = serialized.size();

                    writer.write(msg);
                }
            }
        }
        if (write_output)
        {
            writer.close();
        }
    };

    optimset_t optimset;

    optimset.tolx = 0.001;   // tolerance on the simplex solutions coordinates
    optimset.tolf = 0.001;   // tolerance on the function value
    optimset.max_iter = 100; // maximum number of allowed iterations
    optimset.max_eval = 100; // maximum number of allowed function evaluations
    optimset.verbose = 1;    // toggle verbose output during minimization

    point_t start, solution;
    double start_x[1];
    double solution_x[1];
    start.x = start_x;
    start.x[0] = nst_data.x_scale;

    solution.x = solution_x;

    nelder_mead(1, &start, &solution, cost_function, &nst_data, &optimset);

    std::cout << "Solution: " << solution.x[0] << std::endl;

    nst_data.x_scale = solution.x[0];

    write_output = true;
    cost_function(1, &solution, &nst_data);

    reader.close();

    return EXIT_SUCCESS;
}