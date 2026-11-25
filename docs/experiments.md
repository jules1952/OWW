# Plan badań wydajnościowych

Seria P1 obejmuje sprawdzenie poprawności na 20 startach i pomiar skalowania na jednej instancji. Tabela pokazuje pełny plan badań. Ustawienia zapisujemy w `bench/configs/`, a surowe wyniki w `results/raw/`. [Dotychczasowe wyniki E1 i E3](iterations/03-p1-openmp.md)

Wariant G1 zwalidowano na tych samych startach E1 i zmierzono na instancji z E3. Próba daje pierwszy punkt odniesienia dla CUDA. Wyznaczenie progu opłacalności w E5 wymaga serii na większej liczbie rozmiarów. [Pierwszy pomiar G1](iterations/04-g1-cuda.md)

| ID | Pytanie | Główne zmienne | Raport |
|---|---|---|---|
| E1 | Czy warianty zachowują poprawność i jaki jest wpływ startu? | instancja, wariant, co najmniej 10 startów | koszt końcowy, hash, iteracje i zgodność z S0 |
| E2 | Jak rozmiar wpływa na czas i pamięć? | `n`, `m`, `p`, typ instancji | wykres log-log, nachylenie i odniesienie do złożoności |
| E3 | Jak OpenMP skaluje się przy stałej instancji? | 1, 2, 4, … wątków | speedup, efektywność i metryka Karp–Flatta |
| E4 | Jak OpenMP skaluje się przy rosnącym problemie? | liczba wątków i proporcjonalne `n` | efektywność słabego skalowania |
| E5 | Od jakiego rozmiaru GPU się opłaca? | te same instancje dla seq, omp i cuda | punkt przecięcia oraz czasy prep/H2D/search/D2H |
| E6 | Jak rozkład danych wpływa na wynik i czas? | skupiony/równomierny, wagi równe/zmienne | iteracje, koszt i czas |
| E7 | Jak zmienia się zużycie pamięci i transfer? | rozmiar instancji i wariant | RAM, pamięć GPU i bajty transferu, jeśli dostępne |

## Protokół pomiaru

- Dla każdej konfiguracji wykonujemy rozgrzewkę oraz co najmniej pięć powtórzeń. Podajemy liczbę powtórzeń, medianę i rozstęp międzykwartylowy (IQR).
- Wszystkie warianty C++ uruchamiamy na tej samej maszynie i z tymi samymi instancjami oraz zestawami początkowymi. Zapisujemy sprzęt, system, kompilator, flagi i ustawienia OpenMP. Dla CUDA zapisujemy też wersję narzędzi.
- Przy porównaniu czasów dla GPU uwzględnimy pełne uruchomienie z transferami i osobno pokażemy `t_prep`, `t_h2d`, `t_search` oraz `t_d2h`.

## Zapis wyników

Każdy wiersz zbiorczego CSV odpowiada pojedynczemu uruchomieniu. Skrypt sekwencyjny zapisuje `measurements.csv`, `warmup.csv`, `profile-runs.csv`, `phases.csv` i `manifest.json`. Skrypt P1 zapisuje `scaling.csv`, `warmup.csv`, `validation.csv` i `manifest.json`. Opcja `--include-cuda` dołącza G1 i kolumny `h2d_bytes` oraz `d2h_bytes`. W nowych tabelach `station_indices` zawiera indeksy stacji rozdzielone spacjami.

W archiwalnych seriach sekwencyjnych `source_file` wskazuje pierwotny plik względem `results/raw/`, a `metadata_id` odsyła do treści jego metadanych w `archive.metadata` w manifeście. Pole `rep` zachowuje wartość z pierwotnego CSV. `run_number` zawiera numer odczytany z nazwy pliku, jeśli był w niej zapisany. [Pojedyncze wcześniejsze uruchomienia](../results/raw/seq-standalone/runs.csv) pozostają we wspólnej tabeli.

## Metryki

Dla `T` wątków i sekwencyjnego czasu `t_1`:

- speedup: `S(T) = t_1 / t_T`
- efektywność: `E(T) = S(T) / T`
- metryka Karp–Flatta: `e_KF = (1/S(T) - 1/T) / (1 - 1/T)`

Każda metryka musi używać tego samego zakresu mierzonej pracy w liczniku i mianowniku. Dla E5 domyślnym porównaniem end-to-end jest całkowity czas GPU wraz z transferami i przygotowaniem wymaganym w danym przebiegu.

## Wymagania do interpretacji

Raportuj jakość rozwiązania razem z wydajnością. Vertex Substitution daje lokalne optimum względem pojedynczych zamian i sam w sobie nie dowodzi osiągnięcia optimum globalnego. Nie porównuj bezpośrednio czasów z literatury, jeśli różnią się dane, sprzęt, implementacja lub metodologia.
