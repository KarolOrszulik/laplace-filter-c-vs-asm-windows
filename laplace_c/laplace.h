/**
* Filtr Laplace'a - implementacja wielow¹tkowa w C
*
* Filtr konwolucyjny s³u¿¹cy do wykrywania krawêdzi na obrazie.
*
* Autor: Karol Orszulik
* Politechnika Œl¹ska, wydzia³ AEI, kierunek Informatyka
* Rok akademicki 2024/2025, semestr 5.
*/

#ifndef LAPLACE_H
#define LAPLACE_H

#include <windows.h>

/**
* Deklaracja funkcji laplace z dyrektyw¹ eksportu z biblioteki DLL.
* Rozdziela pracê do wykonania i tworzy w¹tki.
* Nastêpnie czeka na ich zakoñczenie i koñczy pracê.
* 
* Parametry:
*   w - szerokoœæ obrazu
*   h - wysokoœæ obrazu
*   from - wskaŸnik na tablicê pikseli obrazu wejœciowego
*   to - wskaŸnik na tablicê pikseli obrazu wyjœciowego
*   numThreads - liczba w¹tków (1-64)
*   amplification - wspó³czynnik wzmocnienia (mno¿nik dla ka¿dego subpiksela)
* Zwraca: void
*/
void __declspec(dllexport) WINAPI laplace(int w, int h, unsigned char* from, unsigned char* to, int numThreads, int amplification);


/**
* Funkcja w¹tku.
* Wykonuje filtr Laplace'a na fragmencie obrazu, wierszami.
* Filtruje wiersz, nastêpnie przeskakuje o krok równy liczbie w¹tków.
*
* Parametry: args - wskaŸnik na strukturê z argumentami w¹tku
* Zwraca: 0 (nieu¿ywane, efektywnie void)
*/
DWORD WINAPI thread_func(LPVOID args);


/** 
* Struktura przekazywana jako argument dla w¹tków.
*/
struct thread_args_s;

#endif
