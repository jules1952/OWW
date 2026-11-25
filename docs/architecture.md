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
| `src/common/nearest_search.cpp` | Obliczenia wspólne dla S1 i P1. |
| `src/common/solver_cli.cpp` | Obsługa poleceń, pomiar i zapis wyników. |
| `src/seq/` | Wersje sekwencyjne S0 i S1. |
| `src/omp/p1.cpp` | Podział kandydatów między wątki i wybór spośród najlepszych ruchów wątków. |

Dla każdego punktu popytu S1 i P1 zapisują najbliższą wybraną stację i dwie najmniejsze odległości. Jeśli usuwamy zapamiętaną najbliższą stację, do porównania z odległością do `in` bierzemy drugą. W pozostałych przypadkach używamy pierwszej. Odległości do `in` liczymy raz dla danego kandydata w iteracji i wykorzystujemy dla wszystkich możliwych `out`. Po przyjęciu ruchu obliczamy dwie najbliższe stacje na nowo.

W P1 OpenMP statycznie dzieli kandydatów `in` między wątki. Każdy wątek ma własny bufor odległości i zapamiętuje najlepszą ocenioną przez siebie zamianę. Po zakończeniu pracy wątek sterujący wybiera wspólnego zwycięzcę za pomocą tego samego komparatora. Każdy koszt sumujemy w `double` w kolejności punktów z pliku, tak jak w S0.

## Koszt obliczeń

Przy `n` punktach popytu ocena wszystkich zamian w S0 kosztuje `O(n * p² * (m - p))` na iterację. W S1 i P1 koszt oceny wynosi `O(n * p * (m - p))`, a odświeżenie odległości po zamianie kosztuje `O(n * p)`. P1 dzieli tę samą pracę między wątki i potrzebuje dodatkowo bufora `n` odległości na każdy przydzielony wątek. Liczba iteracji zależy od danych i startu.

## Wynik i czasy

Polecenia `pm_s0`, `pm_s1` i `pm_p1` zapisują po jednym wierszu CSV. W P1 pole `threads` oznacza największą faktyczną liczbę wątków w trakcie wyszukiwania. Jeśli `p = m`, P1 zapisuje `threads=1`, ponieważ nie tworzy zespołu. Pole `final_set_hash` jest sumą kontrolną posortowanych indeksów stacji. Lista indeksów trafia też do osobnego pliku.

```text
variant,instance,dist,n,m,p,threads,rep,t_prep,t_h2d,t_search,t_d2h,t_total,iterations,cost,peak_ram_mb,gpu_mem_mb,final_set_hash
```

Czasy podajemy w sekundach. `t_prep` obejmuje odczyt danych, `t_search` pracę solvera, a `t_total` cały proces wraz z odczytem i zapisem. To ostatnie pole uzupełnia skrypt benchmarkowy. W CUDA dojdą transfery `t_h2d` i `t_d2h`. [Metoda pomiaru i tabele](experiments.md)
