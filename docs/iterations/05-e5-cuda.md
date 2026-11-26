# E5. Kiedy CUDA skraca całe uruchomienie

## Metoda

Porównaliśmy S1, P1 z 4, 8 i 16 wątkami oraz G1 z 128 wątkami na blok. Główna seria zmienia liczbę punktów popytu `n` przy `m = 180` kandydatach i `p = 15` stacjach. Dodatkowa instancja przy `n = 10 000` zwiększa `m` do 720 i sprawdza wpływ większej liczby możliwych zamian. Dla wszystkich instancji generator używa położeń równomiernych, wag lognormalnych i ziarna 3. Każdy przypadek ma jeden start z ziarnem 1. Liczba zaakceptowanych zamian może się zmieniać między instancjami.

Każdy wariant dostał tę samą instancję i start. Wykonaliśmy jedną rozgrzewkę i pięć pomiarów, każdy w nowym procesie. Warianty zmieniały kolejność między powtórzeniami. `t_total` obejmuje pełny proces C++ wraz z odczytem, przygotowaniem, transferami, wyszukiwaniem i zapisem. `t_search` dla S1 i P1 obejmuje liczenie odległości, które G1 wykonuje wcześniej w `t_prep`. Dla każdej instancji uruchomiliśmy raz S0 i sprawdziliśmy koszt, hash, listę stacji oraz liczbę zamian dla rozgrzewek i pomiarów. [Konfiguracja](../../bench/configs/g1-e5.json) · [Manifest](../../results/raw/g1-e5/manifest.json) · [Walidacja](../../results/raw/g1-e5/validation.csv)

Serię wykonaliśmy na Intel Core i5-12600KF z 16 wątkami logicznymi i 32 GiB RAM oraz GeForce RTX 3070 z 8 GiB VRAM pod Windows 10 (19045). Użyliśmy binariów z poprzedniej serii G1, w Release x64 z MSVC 19.52.37029.0 (`/O2 /Ob2 /DNDEBUG`), OpenMP 2.0 (`OMP_DYNAMIC=FALSE`), CUDA 13.5.37 dla `sm_86` i sterownika 621.01. Manifest zawiera sumy kontrolne binariów i danych.

## Pełny czas uruchomienia

Czasy to mediana w ms, z rozstępem międzykwartylowym (IQR) w nawiasie. Przyspieszenie G1 liczymy jako stosunek median `t_total` w S1 i G1. [Surowe pomiary](../../results/raw/g1-e5/scaling.csv) · [Rozgrzewki](../../results/raw/g1-e5/warmup.csv)

| `n` | `m` | S1 | P1 · 4 | P1 · 8 | P1 · 16 | G1 · 128 | S1/G1 |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 000 | 180 | 48,7 (2,3) | 24,9 (7,6) | 24,0 (2,7) | 20,8 (15,5) | 140,4 (4,6) | 0,35× |
| 3 000 | 180 | 125,9 (2,1) | 48,9 (8,9) | 49,7 (8,3) | 44,1 (1,5) | 150,2 (7,0) | 0,84× |
| 10 000 | 180 | 462,1 (11,4) | 161,0 (3,2) | 147,2 (5,6) | 137,2 (9,9) | 230,5 (10,0) | 2,00× |
| 30 000 | 180 | 1 470,3 (16,2) | 457,8 (5,8) | 393,6 (8,5) | 388,6 (18,6) | 424,6 (13,7) | 3,46× |
| 60 000 | 180 | 3 169,5 (86,2) | 983,3 (53,7) | 835,6 (72,5) | 860,1 (116,9) | 687,3 (37,2) | 4,61× |
| 10 000 | 720 | 3 719,0 (33,4) | 1 037,7 (25,0) | 964,1 (30,2) | 936,5 (3,5) | 357,6 (13,6) | 10,40× |

Przy `m = 180` przewaga GPU nad S1 pojawia się między pomiarami dla `n = 3 000` i `n = 10 000`, a nad najszybszym P1 między `n = 30 000` i `n = 60 000`. Dla `n = 10 000, m = 720` G1 skrócił pełne uruchomienie względem najlepszego P1 2,62 raza. W tym przypadku oceniamy 10 575 zamian na iterację zamiast 2 475 przy `m = 180`, a liczba przyjętych zamian wzrosła z 18 do 41. Przedziały dotyczą zbadanych instancji, jednego startu na instancję i jednej maszyny.

Wszystkie 150 pomiarów i 30 rozgrzewek dało dokładnie te same koszty, listy stacji, wartości `final_set_hash` i liczby zaakceptowanych zamian co S0. [Wyniki referencyjne S0](../../results/raw/g1-e5/validation.csv)

## Fazy i pamięć G1

Fazy w ms podano jako medianę (IQR). Pamięć GPU obejmuje bufory solvera. Szczytowe użycie RAM procesu i całkowite zużycie VRAM z kontekstem CUDA są niezmierzone. [Czasy i bajty transferów](../../results/raw/g1-e5/scaling.csv)

| `n` | `m` | Przygotowanie | H2D | Wyszukiwanie | D2H | Bufory GPU [MiB] |
|---:|---:|---:|---:|---:|---:|---:|
| 1 000 | 180 | 105,3 (1,6) | 2,0 (0,7) | 6,1 (0,5) | 0,5 (0,0) | 1,42 |
| 3 000 | 180 | 103,5 (6,2) | 2,4 (0,7) | 17,6 (0,6) | 0,7 (0,1) | 4,23 |
| 10 000 | 180 | 128,0 (3,8) | 4,6 (0,8) | 68,9 (4,0) | 0,8 (0,0) | 14,06 |
| 30 000 | 180 | 177,9 (29,5) | 10,3 (1,5) | 193,2 (15,0) | 1,0 (0,2) | 42,13 |
| 60 000 | 180 | 276,5 (35,3) | 15,1 (2,0) | 364,2 (34,4) | 1,0 (0,0) | 84,25 |
| 10 000 | 720 | 145,9 (10,9) | 11,0 (0,3) | 171,9 (6,5) | 2,0 (0,1) | 55,32 |

Osobna kompilacja diagnostyczna zmierzyła fragmenty przygotowania i wyszukiwania. Tabela podaje medianę (IQR) w ms dla pięciu przebiegów na instancję. Wszystkie przebiegi z profilem, w tym rozgrzewki, przeszły kontrolę zgodności z S0. [Surowy profil](../../results/raw/g1-e5-profile/phases.csv) · [Manifest](../../results/raw/g1-e5-profile/manifest.json)

| `n` | `m` | Zapytania CUDA | Alokacje GPU | Macierz na CPU | Kernel i synchronizacja |
|---:|---:|---:|---:|---:|---:|
| 1 000 | 180 | 17,4 (0,8) | 79,8 (4,0) | 1,0 (0,1) | 3,8 (0,3) |
| 3 000 | 180 | 17,1 (0,5) | 78,0 (9,2) | 3,0 (0,2) | 10,4 (0,3) |
| 10 000 | 180 | 17,1 (0,3) | 75,1 (6,4) | 9,5 (0,0) | 42,1 (0,6) |
| 30 000 | 180 | 18,8 (0,9) | 75,0 (2,1) | 29,0 (0,9) | 105,5 (4,0) |
| 60 000 | 180 | 19,0 (0,8) | 78,2 (1,5) | 56,8 (0,9) | 212,0 (0,7) |
| 10 000 | 720 | 16,8 (0,6) | 72,7 (0,7) | 37,4 (0,0) | 103,9 (0,5) |

Na małych instancjach koszt przygotowania skupia się w pierwszych wywołaniach CUDA. Alokacje trwały dłużej od przygotowania macierzy na każdej z sześciu instancji. Na większych instancjach kernel i synchronizacja stanowiły największą z faz mierzonych szczegółowo. Dlatego sprawdziliśmy zmianę konfiguracji uruchomienia kernela.

## Próba z 64 wątkami na blok

Na badanej RTX 3070 sterownik zgłosił 46 multiprocesorów strumieniowych. Przy `m = 180` i `p = 15` wykonujemy 2 475 równoległych ocen na iterację. Blok o 128 wątkach daje 20 bloków, a blok o 64 wątkach daje 39. Hipoteza zakłada, że więcej bloków pozwoli wykorzystać więcej multiprocesorów przy tej samej liczbie ocen. Jeden wątek nadal oblicza koszt jednej zamiany i sumuje punkty w kolejności wejścia.

Porównaliśmy obie kompilacje G1 na sześciu instancjach z E5. Na każdą kompilację przypada jedna rozgrzewka i sześć pomiarów bez profilowania. Kolejność uruchamiania kompilacji odwracaliśmy co powtórzenie. [Pomiary przed i po](../../results/raw/g1-e5-block-size/measurements.csv) · [Manifest](../../results/raw/g1-e5-block-size/manifest.json)

| `n` | `m` | Cały proces · 128 | Cały proces · 64 | Wyszukiwanie · 128 | Wyszukiwanie · 64 |
|---:|---:|---:|---:|---:|---:|
| 1 000 | 180 | 138,8 (4,4) | 138,1 (2,9) | 6,2 (0,2) | 5,7 (0,3) |
| 3 000 | 180 | 155,9 (16,5) | 157,1 (15,1) | 16,6 (0,7) | 15,7 (0,6) |
| 10 000 | 180 | 221,9 (4,3) | 224,8 (7,8) | 67,4 (2,4) | 67,3 (0,7) |
| 30 000 | 180 | 365,3 (8,2) | 374,9 (7,0) | 167,8 (2,9) | 171,9 (2,9) |
| 60 000 | 180 | 612,9 (2,4) | 613,1 (6,6) | 339,5 (2,0) | 337,2 (8,1) |
| 10 000 | 720 | 350,7 (3,4) | 355,5 (3,8) | 160,8 (1,6) | 163,0 (2,5) |

Czasy w ms to mediana (IQR). Wszystkie 72 pomiary i 12 rozgrzewek przeszły kontrolę zgodności względem referencji S0. Obie kompilacje przeszły CTest z CUDA i OpenMP.

Odrzuciliśmy zmianę na 64 wątki jako domyślną. Największy spadek mediany całego czasu wyniósł 0,5%, a przy `n = 30 000` mediana wzrosła o 2,6%. Domyślnie budujemy bloki 128-wątkowe. Opcja `PMEDIAN_CUDA_BLOCK_SIZE` pozwala odtworzyć próbę. Porównanie z OpenMP opieramy na pomiarach E5 wykonanych w jednej serii. Przy `n = 30 000` mediana G1 z blokiem 128-wątkowym wyniosła 424,6 ms w E5 i 365,3 ms w osobnej próbie. Ta różnica pokazuje, że przedział przewagi CUDA warto sprawdzić na kolejnych startach i w kolejnej wspólnej serii.

## Odtworzenie

Z katalogu głównego wygeneruj brakujące instancje, zbuduj kompilację z 128 wątkami na blok i uruchom serię. Katalog wyników musi być nowy. Instancja `p1_e3` jest taka sama jak w badaniu OpenMP. [Polecenia kompilacji](../../README.md#cuda)

```powershell
foreach ($n in 1000, 10000, 30000, 60000) {
    python data/gen.py --n $n --m 180 --p 15 --seed 3 --start-seeds 1 --out "data/generated/e5_n${n}_m180"
}
python data/gen.py --n 3000 --m 180 --p 15 --seed 3 --out data/generated/p1_e3
python data/gen.py --n 10000 --m 720 --p 15 --seed 3 --start-seeds 1 --out data/generated/e5_n10000_m720
cmake -S . -B build/g1-e5-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DPMEDIAN_ENABLE_OPENMP=ON -DPMEDIAN_ENABLE_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=86 -DPMEDIAN_CUDA_BLOCK_SIZE=128
cmake --build build/g1-e5-release
python bench/run_omp_scaling.py build/g1-e5-release bench/configs/g1-e5.json results/raw/g1-e5-repeat --include-cuda
```

Profil szczegółowy zapisujemy z osobnej kompilacji Release. Czas `kernel_and_sync` mierzy od strony CPU uruchomienie kernela i oczekiwanie na jego zakończenie. `host_distance_matrix` obejmuje utworzenie i wypełnienie macierzy odległości. `cuda_device_query` obejmuje pierwsze zapytania o urządzenie, a `cuda_device_alloc` wszystkie sześć alokacji buforów. Uruchomienia z profilem mają osobne wyniki.

```powershell
cmake -S . -B build/g1-profile -G Ninja -DCMAKE_BUILD_TYPE=Release -DPMEDIAN_ENABLE_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=86 -DPMEDIAN_CUDA_BLOCK_SIZE=128 -DPMEDIAN_PROFILE_CUDA=ON
cmake --build build/g1-profile --target pm_g1
python bench/run_cuda_profile.py build/g1-profile results/raw/g1-e5-repeat results/raw/g1-e5-profile-repeat
```

Próbę 64 i 128 wątków na blok można powtórzyć z tymi samymi referencjami S0.

```powershell
cmake -S . -B build/g1-e5-64 -G Ninja -DCMAKE_BUILD_TYPE=Release -DPMEDIAN_ENABLE_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=86 -DPMEDIAN_CUDA_BLOCK_SIZE=64
cmake --build build/g1-e5-64 --target pm_g1
python bench/compare_cuda_builds.py build/g1-e5-release build/g1-e5-64 results/raw/g1-e5-repeat results/raw/g1-e5-block-size-repeat
```
