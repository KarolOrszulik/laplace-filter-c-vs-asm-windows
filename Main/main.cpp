#include <iostream>
#include <windows.h>
#include <chrono>

#include "bmp.h"

// Define the function pointer type
typedef void (*my_function_t)(int, int, unsigned char*, unsigned char*, int, int);

static my_function_t laplace_c = nullptr;
static my_function_t laplace_asm = nullptr;

HMODULE c_handle = nullptr;
HMODULE asm_handle = nullptr;

void load_library(const wchar_t* path, HMODULE* handle, my_function_t* func) {
    *handle = LoadLibrary(path);
    if (!*handle) {
        std::cerr << "LoadLibrary failed: " << GetLastError() << std::endl;
        return;
    }

    *func = reinterpret_cast<my_function_t>(GetProcAddress(*handle, "laplace"));
    if (!*func) {
        std::cerr << "GetProcAddress failed: " << GetLastError() << std::endl;
        FreeLibrary(*handle);
        *handle = nullptr;
        return;
    }
}

void load_libraries() {
    load_library(L"laplace_c.dll", &c_handle, &laplace_c);
    load_library(L"laplace_asm.dll", &asm_handle, &laplace_asm);
}

void free_libraries() {
    if (c_handle)
        FreeLibrary(c_handle);
    if (asm_handle)
        FreeLibrary(asm_handle);
}


int main() {
    load_libraries();

    if (!laplace_c || !laplace_asm) {
        std::cerr << "Failed to load one or more libraries. Exiting." << std::endl;
        return 1;
    }

    bmp_t cat, cat_after;

    bmp_load(&cat, "../img/cat_big.bmp");
    bmp_empty_from(&cat_after, &cat);


    for (int numThreads = 1; numThreads <= 256; numThreads <<= 1) {
        const int SCALE = 1000000;

        // Measure time for the C function
        auto c_start = std::chrono::high_resolution_clock::now();
        laplace_c(cat.width, cat.height, cat.data, cat_after.data, numThreads, 16);
        auto c_end = std::chrono::high_resolution_clock::now();
        auto c_time = std::chrono::duration_cast<std::chrono::microseconds>(c_end - c_start).count();

        // Measure time for the ASM function
        auto asm_start = std::chrono::high_resolution_clock::now();
        laplace_asm(cat.width, cat.height, cat.data, cat_after.data, numThreads, 16);
        auto asm_end = std::chrono::high_resolution_clock::now();
        auto asm_time = std::chrono::duration_cast<std::chrono::microseconds>(asm_end - asm_start).count();

        std::cout << "Threads: " << numThreads
            << ", C: " << c_time
            << ", ASM: " << asm_time
            << ", Speedup: " << (float(c_time) / (asm_time)) << "x" << std::endl;
    }


    bmp_save(&cat_after, "../img/cat_big_laplace.bmp");

    bmp_free(&cat);
    bmp_free(&cat_after);

    free_libraries();

    return 0;
}