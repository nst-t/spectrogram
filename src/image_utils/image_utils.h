#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <stddef.h>

// Function to encode data to base64
char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);

// Function to draw a blue circle on the image array
void draw_blue_circle(unsigned char ***image, unsigned width, unsigned height, unsigned center_x, unsigned center_y, unsigned radius);

// Function to create a PNG image from an input array of floating point values
unsigned char *create_png_from_array(size_t *png_size, unsigned char ***image, unsigned width, unsigned height, int current_index);

// Function to create a 256x256 PNG image of a blue circle in memory
unsigned char *create_blue_circle_png(size_t *png_size, unsigned center_x, unsigned center_y, unsigned radius);

#endif // IMAGE_UTILS_H