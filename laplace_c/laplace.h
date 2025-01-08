/**
* Filtr Laplace'a - implementacja wielow�tkowa w C
*
* Filtr konwolucyjny s�u��cy do wykrywania kraw�dzi na obrazie.
*
* Autor: Karol Orszulik
* Politechnika �l�ska, wydzia� AEI, kierunek Informatyka
* Rok akademicki 2024/2025, semestr 5.
*/

#ifndef LAPLACE_H
#define LAPLACE_H

#include <windows.h>

/**
* Deklaracja funkcji laplace z dyrektyw� eksportu z biblioteki DLL.
* Rozdziela prac� do wykonania i tworzy w�tki.
* Nast�pnie czeka na ich zako�czenie i ko�czy prac�.
* 
* Parametry:
*   w - szeroko�� obrazu
*   h - wysoko�� obrazu
*   from - wska�nik na tablic� pikseli obrazu wej�ciowego
*   to - wska�nik na tablic� pikseli obrazu wyj�ciowego
*   numThreads - liczba w�tk�w (1-64)
*   amplification - wsp�czynnik wzmocnienia (mno�nik dla ka�dego subpiksela)
* Zwraca: void
*/
void __declspec(dllexport) WINAPI laplace(int w, int h, unsigned char* from, unsigned char* to, int numThreads, int amplification);


/**
* Funkcja w�tku.
* Wykonuje filtr Laplace'a na fragmencie obrazu, wierszami.
* Filtruje wiersz, nast�pnie przeskakuje o krok r�wny liczbie w�tk�w.
*
* Parametry: args - wska�nik na struktur� z argumentami w�tku
* Zwraca: 0 (nieu�ywane, efektywnie void)
*/
DWORD WINAPI thread_func(LPVOID args);


/** 
* Struktura przekazywana jako argument dla w�tk�w.
*/
struct thread_args_s;

#endif
