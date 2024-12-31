#pragma once

#include <string>

// Define the BMP structure
struct bmp_t {
    unsigned int width;
    unsigned int height;
    unsigned char* data;

    bmp_t() : width(0), height(0), data(nullptr) {}
};

// Function declarations
void bmp_load(bmp_t* bmp, const std::string& filename);
void bmp_save(const bmp_t* bmp, const std::string& filename);
void bmp_free(bmp_t* bmp);
void bmp_empty_from(bmp_t* bmp, const bmp_t* src);
