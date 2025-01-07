#include "laplace.h"
#include <windows.h>
#include <stdlib.h>

#define NUM_CHANNELS 3
#define LAPLACE_MAX_THREADS MAXIMUM_WAIT_OBJECTS

struct thread_args_s {
    int w;
    int h;
    unsigned char* from;
    unsigned char* to;
    int amplification;
    int start;
    int step;
};

DWORD WINAPI thread_func(LPVOID args);

void laplace(int w, int h, unsigned char* from, unsigned char* to, int numThreads, int amplification) {
    HANDLE threads[LAPLACE_MAX_THREADS];
    struct thread_args_s args[LAPLACE_MAX_THREADS];

    if (numThreads > LAPLACE_MAX_THREADS)
        numThreads = LAPLACE_MAX_THREADS;

    for (int i = 0; i < numThreads; i++) {
        args[i].w = w;
        args[i].h = h;
        args[i].from = from;
        args[i].to = to;
        args[i].amplification = amplification;
        args[i].start = i + 1; // omit 0-th row
        args[i].step = numThreads;

        threads[i] = CreateThread(NULL, 0, thread_func, &args[i], 0, NULL);

        if (!threads[i]) {            
			WaitForMultipleObjects(i, threads, TRUE, INFINITE);

			for (int j = 0; j < i; j++) {
				CloseHandle(threads[j]);
			}
			return;
        }
    }

    WaitForMultipleObjects(numThreads, threads, TRUE, INFINITE);

    for (int i = 0; i < numThreads; i++) {
        CloseHandle(threads[i]);
    }
}

DWORD WINAPI thread_func(LPVOID args) {
    struct thread_args_s* targs = (struct thread_args_s*)args;

    int w = targs->w;
    int h = targs->h;
    unsigned char* from = targs->from;
    unsigned char* to = targs->to;
    int amplification = targs->amplification;
    int start = targs->start;
    int step = targs->step;

    for (int y = start; y < h - 1; y += step) {
        for (int x = 1; x < w - 1; x++) {
            for (int c = 0; c < NUM_CHANNELS; c++) {
                const int idx = (y * w + x) * NUM_CHANNELS + c;

                short sum = 0;
                sum += -4 * from[idx];
                sum += from[idx - NUM_CHANNELS];
                sum += from[idx + NUM_CHANNELS];
                sum += from[idx - w * NUM_CHANNELS];
                sum += from[idx + w * NUM_CHANNELS];

                sum *= amplification;

                if (sum < 0)
                    sum = 0;
                else if (sum > 255)
                    sum = 255;

                to[idx] = (unsigned char)sum;
            }
        }
    }

    return 0;
}
