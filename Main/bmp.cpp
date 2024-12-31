#include "bmp.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

// Load a BMP file
void bmp_load(bmp_t* bmp, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << filename << std::endl;
        return;
    }

    unsigned char header[54];
    file.read(reinterpret_cast<char*>(header), sizeof(header));

    // Validate the BMP file
    if (header[0] != 'B' || header[1] != 'M') {
        std::cerr << "Error: invalid BMP file" << std::endl;
        return;
    }

    bmp->width = *reinterpret_cast<unsigned int*>(&header[18]);
    bmp->height = *reinterpret_cast<unsigned int*>(&header[22]);

    if (*reinterpret_cast<unsigned short*>(&header[28]) != 24) {
        std::cerr << "Error: unsupported color depth (only 24-bit supported)" << std::endl;
        return;
    }

    size_t data_size = bmp->width * bmp->height * 3;
    bmp->data = new unsigned char[data_size];

    file.read(reinterpret_cast<char*>(bmp->data), data_size);
    file.close();
}

// Save a BMP file
void bmp_save(const bmp_t* bmp, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << filename << std::endl;
        return;
    }

    unsigned char header[54] = { 0 };
    header[0] = 'B';
    header[1] = 'M';
    *reinterpret_cast<unsigned int*>(&header[2]) = 54 + bmp->width * bmp->height * 3;
    *reinterpret_cast<unsigned int*>(&header[10]) = 54;
    *reinterpret_cast<unsigned int*>(&header[14]) = 40;
    *reinterpret_cast<unsigned int*>(&header[18]) = bmp->width;
    *reinterpret_cast<unsigned int*>(&header[22]) = bmp->height;
    *reinterpret_cast<unsigned short*>(&header[26]) = 1;
    *reinterpret_cast<unsigned short*>(&header[28]) = 24;

    file.write(reinterpret_cast<char*>(header), sizeof(header));
    file.write(reinterpret_cast<const char*>(bmp->data), bmp->width * bmp->height * 3);
    file.close();
}

// Free BMP data
void bmp_free(bmp_t* bmp) {
    delete[] bmp->data;
    bmp->data = nullptr;
}

// Create an empty BMP from another BMP
void bmp_empty_from(bmp_t* bmp, const bmp_t* src) {
    bmp->width = src->width;
    bmp->height = src->height;
    bmp->data = new unsigned char[bmp->width * bmp->height * 3]();
}
