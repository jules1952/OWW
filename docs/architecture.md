# Jak działa program

## Jedna iteracja

1. Wczytujemy instancję i rozwiązanie początkowe. Kolejność kandydatów w pliku nadaje im indeksy od zera.
2. Dla bieżącego zestawu rozpatrujemy `p * (m - p)` zamian. W pojedynczej próbie usuwamy stację `out` i wstawiamy kandydata `in`.
3. Porównujemy ruchy według `(delta, out, in)`. Wybieramy najmniejszą deltę, a przy remisie najmniejszą parę indeksów.
4. Zatwierdzamy jedną zamianę, jeśli `delta < -1e-9 * cost`. Każdą kolejną iterację zaczynamy od zaktualizowanego zestawu. Brak takiej zamiany kończy wyszukiwanie.

Dla `p = m` lista zamian jest pusta. Program zwraca wtedy zestaw początkowy i jego koszt.

## Gdzie są obliczenia

| Plik lub katalog | Zadanie |
|---|---|
| `src/common/input.cpp` | Odczyt i walidacja instancji oraz zestawu początkowego. |
| `src/common/solution.hpp` | Typ wyniku, porównanie ruchów i test progu poprawy. |
| `src/common/nearest_search.cpp` | Stan wyszukiwania i obliczenia wspólne dla S1, P1 oraz części CPU w G1. |
| `src/common/solver_cli.cpp` | Obsługa poleceń, pomiar i zapis wyników. |
| `src/seq/` | Wersje sekwencyjne S0 i S1. |
| `src/omp/p1.cpp` | Podział kandydatów między wątki i wybór spośród najlepszych ruchów wątków. |
| `src/cuda/g1.cu` | Przygotowanie danych i sterowanie na CPU, równoległa ocena kosztów zamian na GPU. |

Dla każdego punktu popytu S1 i P1 zapisują najbliższą wybraną stację i dwie najmniejsze odległości. Jeśli usuwamy zapamiętaną najbliższą stację, do porównania z odległością do `in` bierzemy drugą. W pozostałych przypadkach używamy pierwszej. Odległości do `in` liczymy raz dla danego kandydata w iteracji i wykorzystujemy dla wszystkich możliwych `out`. Po przyjęciu ruchu obliczamy dwie najbliższe stacje na nowo.

W P1 OpenMP statycznie dzieli kandydatów `in` między wątki. Każdy wątek ma własny bufor odległości i zapamiętuje najlepszą ocenioną przez siebie zamianę. Po zakończeniu pracy wątek sterujący wybiera wspólnego zwycięzcę za pomocą tego samego komparatora. Każdy koszt sumujemy w `double` w kolejności punktów z pliku, tak jak w S0.

W G1 CPU oblicza macierz odległości do wszystkich kandydatów i razem z wagami przesyła ją jednorazowo na GPU. W każdej iteracji przesyła bieżące indeksy stacji i kandydatów oraz dwie najbliższe odległości dla każdego punktu. Każdy wątek CUDA sumuje koszt jednej zamiany w kolejności punktów, z osobnym zaokrągleniem mnożenia i dodawania. GPU odsyła wszystkie koszty zamian, a CPU wybiera ruch wspólnym komparatorem, sprawdza próg poprawy i odświeża najbliższe odległości. Przy `p = m` wystarcza obliczenie początkowego kosztu na CPU.

## Koszt obliczeń

Przy `n` punktach popytu ocena wszystkich zamian w S0 kosztuje `O(n * p² * (m - p))` na iterację. W S1 i P1 koszt oceny wynosi `O(n * p * (m - p))`, a odświeżenie odległości po zamianie kosztuje `O(n * p)`. P1 dzieli tę samą pracę między wątki i potrzebuje dodatkowo bufora `n` odległości na każdy przydzielony wątek. Liczba iteracji zależy od danych i startu.

G1 wykonuje `O(n * m)` obliczeń w przygotowaniu. W każdej iteracji ocenia `O(n * p * (m - p))` operacji na GPU, a na CPU przegląda `p * (m - p)` kosztów i po zatwierdzeniu ruchu odświeża najbliższe odległości. Główne bufory GPU to macierz `n * m` i tablica `p * (m - p)` kosztów, każda po 8 bajtów na element.

## Wynik i czasy

Polecenia `pm_s0`, `pm_s1`, `pm_p1` i `pm_g1` zapisują po jednym wierszu CSV. W P1 pole `threads` oznacza największą faktyczną liczbę wątków CPU w trakcie wyszukiwania. Jeśli `p = m`, P1 zapisuje `threads=1`, ponieważ nie tworzy zespołu. W G1 `threads=1` opisuje wątek sterujący na CPU. Pole `final_set_hash` jest sumą kontrolną posortowanych indeksów stacji. Lista indeksów trafia też do osobnego pliku.

```text
variant,instance,dist,n,m,p,threads,rep,t_prep,t_h2d,t_search,t_d2h,t_total,iterations,cost,peak_ram_mb,gpu_mem_mb,final_set_hash
```

Czasy podajemy w sekundach. `t_prep` obejmuje odczyt danych, a w G1 także przygotowanie solvera, w tym obliczenie macierzy i przydział buforów. `t_h2d` i `t_d2h` obejmują transfery CUDA. `t_search` obejmuje pozostały czas solvera, w tym uruchomienia i synchronizację kerneli oraz wybór ruchów. `t_total` mierzy cały proces wraz z odczytem i zapisem. To ostatnie pole uzupełnia skrypt benchmarkowy. `gpu_mem_mb` w G1 oznacza rozmiar buforów solvera w MiB. Metadane zawierają też `h2d_bytes` i `d2h_bytes`. [Metoda pomiaru i tabele](experiments.md)
