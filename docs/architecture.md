# Architektura i kontrakt wariantów

## Przepływ obliczeń

1. Generator Python tworzy instancję i zapisuje ziarno oraz parametry.
2. Program C++ wczytuje instancję i wspólne rozwiązanie startowe.
3. Backend ocenia każdą parę zamiany `(out, in)` względem bieżącego zbioru stacji.
4. Wspólna reguła redukcji wybiera najlepszą zamianę deterministycznie.
5. Backend zaakceptuje ruch tylko przy `delta < -epsilon * cost`, aktualizuje rozwiązanie i przechodzi do następnej iteracji.
6. Program emituje jeden wiersz CSV w ustalonym schemacie.

## Granice modułów

| Katalog | Odpowiedzialność |
|---|---|
| `src/common/` | Typy instancji i rozwiązania, odczyt danych, funkcja celu, kryterium stopu, reguła remisów, hash zbioru, wynik i pomiar czasu |
| `src/seq/` | Referencyjne S0 oraz kolejne optymalizacje sekwencyjne S1/S2 |
| `src/omp/` | Wielowątkowa ocena zamian przez OpenMP, lokalne wyniki wątków i deterministyczne scalenie |
| `src/cuda/` (planowany) | CPU koordynuje algorytm, GPU oblicza i redukuje delty zamian |
| `data/` | Generator, konfiguracje i format instancji/startów |
| `bench/` | Siatka parametrów, uruchomienia, powtórzenia i zbieranie CSV |
| `analysis/` (planowany) | Tabele i wykresy generowane z surowych wyników |
| `docs/` | Kontrakt algorytmu, metodologia badań i notatki optymalizacyjne |

## CLI i wynik

Planowany interfejs ma postać:

```text
pm --variant seq|omp|cuda --instance data/X.txt --start data/S.txt --threads 8 --reps 10 --out results/raw/E3.csv
```

Sekwencyjny i CUDA wariant mogą ignorować `--threads`; pole pozostaje wspólne dla porównywalnego interfejsu. Nazwa backendu nie zmienia semantyki algorytmu.

Aktualne programy to `pm_s0`, `pm_s1` i `pm_p1`. P1 przyjmuje dodatkowo `--threads N` po ścieżce CSV. Dla P1 pole `threads` w CSV oznacza największy faktyczny rozmiar zespołu w trakcie wyszukiwania. Jeśli wszystkie stacje są wybrane, P1 kończy pracę bez tworzenia zespołu i zapisuje `threads=1`.

Wspólny CSV powinien zawierać kolumny:

```text
variant,instance,dist,n,m,p,threads,rep,t_prep,t_h2d,t_search,t_d2h,t_total,iterations,cost,peak_ram_mb,gpu_mem_mb,final_set_hash
```

Jednostką czasu jest sekunda, pamięci MB, a `dist` identyfikuje metrykę. Hash licz z kanonicznie posortowanych indeksów wybranych stacji, aby niezależnie od kolejności przechowywania zestawu porównywać ten sam wynik.

## Semantyka numeryczna

- Referencją jest S0, które dla każdej możliwej zamiany ponownie liczy koszt kompletnego rozwiązania.
- `--variant seq` wskazuje aktualnie wybrany sekwencyjny wariant do końcowych porównań. Na początku jest to S0; po potwierdzeniu zgodności może nim zostać S1 lub S2. S0 pozostaje dostępne jako referencja poprawności. W notatce, konfiguracji lub manifeście CSV zapisuj etap i tag Git użytej implementacji.
- Wariant S1 może używać dwóch najbliższych wybranych stacji, by szybciej wyznaczać koszt po usunięciu stacji.
- Koszt i delta są typu `double`; precyzja odległości może być zmieniona tylko jawnie.
- Remisy rozwiązuj tym samym porządkiem `(out, in)` w S0, OpenMP i CUDA. Redukcja GPU musi zachować ten porządek.
- W każdej iteracji wszystkie zamiany oceniane są względem jednego, niezmienionego bieżącego rozwiązania. Najlepsza zamiana jest zatwierdzana dopiero po redukcji.
- Kryterium stopu: brak zamiany z `delta < -epsilon * cost`, domyślnie `epsilon = 1e-9`.

## Oczekiwana złożoność na iterację

Niech `n=|D|`, `m=|F|`, a `p` oznacza liczbę wybieranych lokalizacji. S0 ma koszt `O(p^2 (m-p) n)` przy pełnym przeliczaniu kosztu dla każdej zamiany. S1/S2 i równoległe mapowania tej samej pracy mają koszt `O(p (m-p) n)` plus aktualizację struktur rzędu `O(np)`. Przejście na OpenMP lub CUDA zmienia czas wykonania, nie całkowitą pracę algorytmu. Złożoność całkowita zależy od liczby iteracji, której nie należy przedstawiać jako dobrze ograniczonej wielomianowo.

P1 przydziela kandydatów `in` statycznie. Każdy wątek ma własny bufor `n` odległości i zapisuje swój najlepszy ruch. Po zakończeniu oceny wątek sterujący wybiera najlepszy z tych ruchów za pomocą wspólnego komparatora, zatwierdza go i odświeża dwie najbliższe stacje. Bufory są używane ponownie w kolejnych iteracjach.

## Pomiar GPU

`t_prep`, `t_h2d`, `t_search` i `t_d2h` raportuj oddzielnie. `t_total` musi obejmować te składowe oraz pozostałe narzuty potrzebne do wykonania badanego wariantu. Każde porównanie powinno podawać zarówno czas end-to-end, jak i rozbicie czasu GPU. G3 może utrzymywać dane rezydentnie na urządzeniu, ale nie może ukrywać kosztu początkowego przygotowania i transferu.
