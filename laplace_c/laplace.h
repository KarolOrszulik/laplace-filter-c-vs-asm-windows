#ifndef LAPLACE_H
#define LAPLACE_H

#include <windows.h>

// Laplace function declaration
void __declspec(dllexport) WINAPI laplace(int w, int h, unsigned char* from, unsigned char* to, int numThreads, int amplification);

#endif // LAPLACE_H
