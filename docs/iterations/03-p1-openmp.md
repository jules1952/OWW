# Równoległa ocena zamian w P1

## Hipoteza i implementacja

W profilu S1 dla instancji `n = 1000`, `m = 100`, `p = 10` ocena zamian i liczenie odległości do kandydatów zajęły razem 8,24 ms. Odświeżenie najbliższych stacji zajęło 0,86 ms. Dlatego w P1 rozdzielamy między wątki ocenę kandydatów, a aktualizację zestawu wykonujemy po zakończeniu przeglądu. [Profil S1](../../results/raw/seq-s0-s1-end-to-end/phases.csv)

OpenMP przydziela kandydatów `in` statycznie. Wątek liczy odległości do każdego przydzielonego kandydata raz i używa ich dla wszystkich `out`. Punkty w pojedynczym koszcie sumujemy w `double` i w kolejności z pliku, tak samo jak w S0 i S1. Po zakończeniu oceny wątek sterujący porównuje najlepsze ruchy z każdego wątku według `(delta, out, in)`. Jeśli ruch obniża koszt o więcej niż ustalony próg, aktualizuje stacje i odświeża odległości. [Wspólne reguły](../architecture.md)

## Walidacja E1

Sprawdzono 20 startów, po 10 dla instancji skupionej z wagami lognormalnymi i równomiernej z wagami jednostkowymi (`n=200, m=30, p=5`). Dla każdego startu S1 oraz P1 z 1, 2, 4, 8 i 16 wątkami zwróciły te same stacje, hash, koszt i liczbę zamian co S0. W CTest wykonano również osiem małych przypadków z oczekiwanymi wynikami, w tym remisy i ścisłą granicę `epsilon`, oraz sprawdzono zgłoszenie przepełnienia odległości z wątku roboczego. [Wyniki walidacji](../../results/raw/p1-e1-e3/validation.csv).

## Skalowanie E3

Pomiar wykonano na Windows 10 (19045), Intel Core i5-12600KF z 16 wątkami logicznymi i 32 GiB RAM, w kompilacji Release x64 MSVC 19.52.37029.0 (`/O2 /Ob2 /DNDEBUG`), z OpenMP 2.0 i `OMP_DYNAMIC=FALSE`. Manifest odnotowuje bazowy commit `a3a50b2`, stan katalogu roboczego i sumy kontrolne.

S1 i P1 dostały tę samą instancję równomierną z wagami lognormalnymi i ziarnem 3 (`n = 3000`, `m = 180`, `p = 15`) oraz start `start_p15_s1.txt`. Po jednej rozgrzewce wykonano pięć pomiarów dla każdej konfiguracji. Tabela podaje mediany i rozstęp międzykwartylowy (IQR) w nawiasie. Przyspieszenie liczymy względem S1 dla tego samego zakresu czasu. [Surowe pomiary](../../results/raw/p1-e1-e3/scaling.csv) · [Manifest](../../results/raw/p1-e1-e3/manifest.json)

| Wariant | Wątki | Wyszukiwanie [ms] | Całe uruchomienie [ms] | Przyspieszenie wyszukiwania | Przyspieszenie całego procesu | Efektywność wyszukiwania | Karp–Flatt |
|---|---:|---:|---:|---:|---:|---:|---:|
| S1 | 1 | 108,952 (2,787) | 119,768 (2,267) | 1,00× | 1,00× | 100,0% | — |
| P1 | 1 | 118,366 (9,068) | 130,272 (6,981) | 0,92× | 0,92× | 92,0% | — |
| P1 | 2 | 62,112 (4,207) | 74,799 (5,029) | 1,75× | 1,60× | 87,7% | 0,140 |
| P1 | 4 | 39,362 (1,916) | 50,978 (4,710) | 2,77× | 2,35× | 69,2% | 0,148 |
| P1 | 8 | 33,998 (1,735) | 45,175 (1,637) | 3,20× | 2,65× | 40,1% | 0,214 |
| P1 | 16 | 30,862 (1,286) | 43,101 (2,377) | 3,53× | 2,78× | 22,1% | 0,235 |

Wyniki wszystkich przebiegów pokryły się z S0. Koszt wyniósł `77219654.755592793`, suma kontrolna zestawu `5d92de54509c088e`, a liczba zamian 15. P1 zostaje wariantem OpenMP. Dla tej instancji podwojenie liczby wątków z 8 do 16 skróciło medianę wyszukiwania z 34,0 do 30,9 ms. Zysk jest mniejszy niż przy przejściu z 2 do 4 wątków. Udział poszczególnych narzutów P1 i szczytowe użycie pamięci pozostają niezmierzone.

## Odtworzenie

Z katalogu głównego w terminalu Visual Studio wykonaj poniższe polecenia. Jeśli wskazany katalog danych już istnieje, można użyć danych z tym samym manifestem. Katalog wyników musi być nowy.

```powershell
cmake -S . -B build/p1-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DPMEDIAN_ENABLE_OPENMP=ON
cmake --build build/p1-release
ctest --test-dir build/p1-release --output-on-failure
python data/gen.py --n 3000 --m 180 --p 15 --distribution uniform --seed 3 --out data/generated/p1_e3
python data/gen.py --n 200 --m 30 --p 5 --distribution clustered --seed 2 --out data/generated/p1_e1_clustered
python data/gen.py --n 200 --m 30 --p 5 --distribution uniform --weights unit --seed 3 --out data/generated/p1_e1_uniform
python bench/run_omp_scaling.py build/p1-release bench/configs/p1-e1-e3.json results/raw/p1-e1-e3-repeat
```
