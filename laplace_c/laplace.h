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


/*
1.	Opis implementacji algorytmu
Program implementuje filtr Laplace’a. Jest to filtr konwolucyjny o masce: 
 
Rozdzielenie pracy między wątki w celu poprawienia wydajności odbywa się następująco: każdy wątek przetwarza obraz wierszami. Po zakończeniu przetwarzania wiersza, wątek przeskakuje liczbę wierszy równą liczbie przetwarzających wątków.
Implementacja biblioteki w języku C przetwarza każdy subpiksel osobno. Implementacja w asemblerze procesowa x86-64 dodatkowo wykorzystuje rozkazy wektorowe do dalszego przyspieszenia obliczeń. Ostatnie piksele obrazu są przetwarzane bez wykorzystania rozkazów wektorowych w celu uniknięcie błędów związanych z niepoprawnym przetwarzaniem wartości pikseli na krawędzi obrazu.

2. Opis parametrów wejściowych programu

Interfejs graficzny pozwala na otwarcie obrazu w formacie JPG, PNG oraz BMP.
Funkcje bibioteczne w C i ASM przyjmują następujące parametry:
- szerokość obrazu (w pikselach)
- wysokość obrazu (w pikselach)
- wskaźnik na tablicę pikseli obrazu wejściowego
- wskaźnik na tablicę pikseli obrazu wyjściowego (parametr wyjściowy)
- liczba wątków (1-64)
- współczynnik wzmocnienia (mnożnik dla każdego subpiksela), przydatny, ponieważ filtr Laplace'a często daje wynik o niskiej wartości, co może być niezauważalne na obrazie.

3. Opis wybranego fragmentu asemblerowego kodu źródłowego biblioteki DLL z komentarzami

*/