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

extern "C"
{
#include "nst_main.h"
#include "image_utils/image_utils.h"
}

// Define the array dimensions
const int ROWS = 32;
const int COLS = 32;
const int CHANNELS = 4;

void updateSlidingWindow(unsigned char ***image, unsigned char **newCol, int *currentIndex)
{
    printf("Updating sliding window current index: %d\n", *currentIndex);
    // Update each row in the circular buffer
    for (int i = 0; i < ROWS; ++i)
    {
        memcpy(image[*currentIndex][i], newCol[i], CHANNELS * sizeof(unsigned char));
    }

    // Move to the next index in the circular buffer
    *currentIndex = (*currentIndex + 1) % COLS;
}

void spectrogramToRGB(const double *spectrogram, unsigned char **newCol)
{
    for (int i = 0; i < ROWS; ++i)
    {
        // Normalize the spectrogram value
        double normalizedValue = spectrogram[i] / 0.01;

        // Convert the normalized value to RGB (simple grayscale for this example)
        unsigned char rgbValue = static_cast<unsigned char>((int)normalizedValue % 256);

        // Populate the newCol with the RGB values
        newCol[i][0] = rgbValue;      // Red
        newCol[i][1] = rgbValue; // Green
        newCol[i][2] = rgbValue; // Blue
        newCol[i][3] = 255;      // Alpha (fully opaque)
    }
}

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

int main(int argc, char *argv[])
{
    const char *infile = NULL;
    const char *outfile = NULL;
    const char *param_file = NULL;
    bool write_output = false;

    spectrogram_state_t state;
    unsigned char ***image = (unsigned char ***)malloc(COLS * sizeof(unsigned char **));
    for (int i = 0; i < COLS; ++i)
    {
        image[i] = (unsigned char **)malloc(ROWS * sizeof(unsigned char *));
        for (int j = 0; j < ROWS; ++j)
        {
            image[i][j] = (unsigned char *)malloc(CHANNELS * sizeof(unsigned char));
        }
    }

    int currentIndex = 0; // Circular buffer index
    unsigned char **newCol = (unsigned char **)malloc(ROWS * sizeof(char *));
    for (int i = 0; i < ROWS; ++i)
    {
        newCol[i] = (unsigned char *)malloc(CHANNELS * sizeof(char));
    }

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

        init_spectrogram_state(&state, COLS);

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
            // here is where we can pass in parameters to the algorithm
        }
    }
    const auto onProblem = [](const mcap::Status &status)
    {
        std::cerr << "Status " + std::to_string((int)status.code) + ": " + status.message;
    };

    mcap::ReadMessageOptions options;
    auto messageView = reader.readMessages(onProblem, options);

    nst_event_t input_event;
    nst_event_t output_events[4];
    int output_events_count = 0;

    // Declare the writer to be used if write_output is true
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
    json compressedImageSchemaJson = json::parse(R"(
{
  "title": "foxglove.CompressedImage",
  "description": "A compressed image",
  "$comment": "Generated by https://github.com/foxglove/schemas",
  "type": "object",
  "properties": {
    "timestamp": {
      "type": "object",
      "title": "time",
      "properties": {
        "sec": {
          "type": "integer",
          "minimum": 0
        },
        "nsec": {
          "type": "integer",
          "minimum": 0,
          "maximum": 999999999
        }
      },
      "description": "Timestamp of image"
    },
    "frame_id": {
      "type": "string",
      "description": "Frame of reference for the image. The origin of the frame is the optical center of the camera. +x points to the right in the image, +y points down, and +z points into the plane of the image."
    },
    "data": {
      "type": "string",
      "contentEncoding": "base64",
      "description": "Compressed image data"
    },
    "format": {
      "type": "string",
      "description": "Image format\n\nSupported values: image media types supported by Chrome, such as `webp`, `jpeg`, `png`"
    }
  }
}
  )");
    mcap::Schema compressedImageSchema("foxglove.CompressedImage", "jsonschema", compressedImageSchemaJson.dump());
    std::cout << "schema:" << compressedImageSchemaJson.dump() << '\n';
    writer.addSchema(compressedImageSchema);

    // Register a Channel
    outputChannel = mcap::Channel("spectrogram", "json", compressedImageSchema.id);
    writer.addChannel(outputChannel);

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
            return EXIT_FAILURE;
        }
        if (!parsed.is_object())
        {
            std::cerr << "unexpected non-object message: " << asString << std::endl;
        }

        // 1) Shift in new raw data value
        size_t values_count = parsed["values"].size();
        int id = 0;
        std::string idStr = parsed["id"];

        if (idStr == "lpom-1" || idStr == "mag")
        {
            // magnetometer
            id = 2;
        }
        else if (idStr == "lpom-15" || idStr == "acc")
        {
            // accelerometer
            id = 1;
            for (int i = 0; i < values_count; i++)
            {
                input_event.id = id;
                input_event.values[i] = parsed["values"][i];
                input_event.values_count = values_count;
                input_event.timestamp = parsed["timestamp"];
            }

            output_events_count = 0;
            algorithm_update(&state, &input_event);

            spectrogramToRGB(state.spectrogram, newCol);

            // prints out the full newCol as a 2D array
            for (int i = 0; i < ROWS; i++)
            {
                printf("%d %d %d %d\n", newCol[i][0], newCol[i][1], newCol[i][2], newCol[i][3]);
            }

            // prints currentIndex
            printf("currentIndex: %d\n", currentIndex);
            updateSlidingWindow(image, newCol, &currentIndex);

            // // prints out the full image as a 2D array
            // for (int i = 0; i < ROWS; i++)
            // {
            //     for (int j = 0; j < COLS; j++)
            //     {
            //         printf("%d %d ", image[j][i][0], image[j][i][1]);
            //     }
            //     printf("\n");
            // }

            json payload;
            payload["id"] = "spectrogram";
            // Create a timestamp object
            // Convert logTime to seconds and nanoseconds
            int64_t logTime = it->message.logTime;
            int64_t sec = logTime / 1000000000;  // Convert nanoseconds to seconds
            int32_t nsec = logTime % 1000000000; // Get the remaining nanoseconds

            // Create a timestamp object
            json timestamp;
            timestamp["sec"] = sec;
            timestamp["nsec"] = nsec;
            payload["timestamp"] = timestamp;

            // Create a dynamic array of numbers and assign it to a field in the JSON object
            nlohmann::json values = nlohmann::json::array();
            payload["format"] = "png";

            size_t png_size;
            unsigned char *png_data = create_png_from_array(&png_size, image, COLS, ROWS, currentIndex);

            // Convert to base64
            size_t output_length;
            char *base64_data = base64_encode(png_data, png_size, &output_length);
            if (!base64_data)
            {
                perror("Failed to encode base64");
            }

            printf("Base64 Encoded PNG:\n%s\n", base64_data);

            payload["data"] = base64_data;
            std::string serialized = payload.dump();

            // Write our message
            mcap::Message msg;
            msg.channelId = outputChannel.id;
            msg.logTime = it->message.logTime;         // Required nanosecond timestamp
            msg.publishTime = it->message.publishTime; // Set to logTime if not available
            msg.data = reinterpret_cast<const std::byte *>(serialized.data());
            msg.dataSize = serialized.size();

            writer.write(msg);
        }
        else if (idStr == "lpom-62" || idStr == "gyro")
        {
            // gyro
            id = 4;
        }
        else
        {
            std::cerr << "unexpected id " << idStr << std::endl;
        }
    }

    free(image);
    free(newCol);
    reader.close();
    writer.close();

    return EXIT_SUCCESS;
}