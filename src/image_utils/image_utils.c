#include "image_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <png.h>

// Function to encode data to base64
char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length)
{
    static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static const char padding_char = '=';
    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = (char *)malloc(*output_length + 1);
    if (encoded_data == NULL)
        return NULL;

    for (size_t i = 0, j = 0; i < input_length;)
    {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (size_t i = 0; i < (input_length % 3 ? 3 - (input_length % 3) : 0); i++)
        encoded_data[*output_length - 1 - i] = padding_char;

    encoded_data[*output_length] = '\0';
    return encoded_data;
}

// Function to draw a blue circle on the image array
void draw_blue_circle(unsigned char ***image, unsigned width, unsigned height, unsigned center_x, unsigned center_y, unsigned radius)
{
    for (unsigned y = 0; y < height; y++)
    {
        for (unsigned x = 0; x < width; x++)
        {
            unsigned dx = x - center_x;
            unsigned dy = y - center_y;
            if (dx * dx + dy * dy < radius * radius)
            {
                image[x][y][0] = 0;   // Red
                image[x][y][1] = 0;   // Green
                image[x][y][2] = 255; // Blue
                image[x][y][3] = 255; // Alpha
            }
        }
    }
}

// Function to create a PNG image from an input array of floating point values
unsigned char *create_png_from_array(size_t *png_size, unsigned char ***image, unsigned width, unsigned height, int current_index)
{
    // Encode the image to PNG in memory using libpng
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        return NULL;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, NULL);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return NULL;
    }

    unsigned char *png_data = NULL;
    png_size_t png_data_size = 0;
    FILE *fp = open_memstream((char **)&png_data, &png_data_size);
    if (!fp)
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return NULL;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    // Write the image data
    for (unsigned y = 0; y < height; ++y)
    {
        png_bytep row = (png_bytep)malloc(4 * width * sizeof(png_byte));
        for (unsigned x = 0; x < width; ++x)
        {
            int circular_index = (current_index + x) % width;
            row[x * 4 + 0] = image[circular_index][y][0]; // Red
            row[x * 4 + 1] = image[circular_index][y][1]; // Green
            row[x * 4 + 2] = image[circular_index][y][2]; // Blue
            row[x * 4 + 3] = image[circular_index][y][3]; // Alpha
        }
        png_write_row(png_ptr, row);
        free(row);
    }

    png_write_end(png_ptr, NULL);
    fclose(fp);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    *png_size = png_data_size;
    return png_data;
}

// Function to create a 256x256 PNG image of a blue circle in memory
unsigned char *create_blue_circle_png(size_t *png_size, unsigned center_x, unsigned center_y, unsigned radius)
{
    unsigned width = 256, height = 256;

    // Dynamically allocate memory for the 3D array
    unsigned char ***image = malloc(width * sizeof(unsigned char **));
    for (unsigned i = 0; i < width; i++)
    {
        image[i] = malloc(height * sizeof(unsigned char *));
        for (unsigned j = 0; j < height; j++)
        {
            image[i][j] = malloc(4 * sizeof(unsigned char));
            memset(image[i][j], 255, 4 * sizeof(unsigned char)); // Initialize with white background
        }
    }

    // Draw a blue circle
    draw_blue_circle(image, width, height, center_x, center_y, radius);

    // Create PNG from the image array
    unsigned char *png_data = create_png_from_array(png_size, image, width, height, 0);

    // Free the allocated memory
    for (unsigned i = 0; i < width; i++)
    {
        for (unsigned j = 0; j < height; j++)
        {
            free(image[i][j]);
        }
        free(image[i]);
    }
    free(image);

    return png_data;
}