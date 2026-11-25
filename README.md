# Rozmieszczenie stacji rowerowych — problem p-median

Celem projektu jest wybór `p` lokalizacji stacji rowerowych tak, by łączna ważona odległość mieszkańców do najbliższej stacji była jak najmniejsza. Do szukania rozwiązania używam heurystyki Vertex Substitution (Teitza–Barta). Porównuję czas działania tej samej metody w kolejnych implementacjach.

| Wariant | Jak ocenia zamiany |
|---|---|
| S0 | Przelicza koszt całego zestawu dla każdej próbnej zamiany. To wariant referencyjny. |
| S1 | Wykorzystuje zapamiętane odległości do dwóch najbliższych stacji i ponownie używa odległości do dodawanego kandydata. |
| P1 | Wykonuje te same obliczenia co S1, rozdzielając kandydatów między wątki OpenMP. |

Solvery są napisane w C++17. Skrypty w Pythonie generują dane i uruchamiają benchmarki. W projekcie przewidziany jest też wariant CUDA.

## Jak uruchomić projekt

Na Windows polecenia uruchom z katalogu repozytorium w terminalu deweloperskim Visual Studio dla x64. Potrzebne są MSVC z OpenMP, CMake 3.20 lub nowszy, Ninja i Python 3. Generator korzysta z biblioteki standardowej.

```powershell
python data/gen.py --n 1000 --m 100 --p 5 --distribution uniform --seed 1 --out data/generated/uniform_demo
cmake -S . -B build/p1-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DPMEDIAN_ENABLE_OPENMP=ON
cmake --build build/p1-release
ctest --test-dir build/p1-release --output-on-failure
```

Każdy program przyjmuje ścieżkę do instancji, pliku z rozwiązaniem początkowym i nowego pliku CSV. W przykładzie wszystkie wersje dostają ten sam start.

```powershell
.\build\p1-release\pm_s0.exe data/generated/uniform_demo/instance.txt data/generated/uniform_demo/start_p5_s1.txt results/raw/demo_s0.csv
.\build\p1-release\pm_s1.exe data/generated/uniform_demo/instance.txt data/generated/uniform_demo/start_p5_s1.txt results/raw/demo_s1.csv
.\build\p1-release\pm_p1.exe data/generated/uniform_demo/instance.txt data/generated/uniform_demo/start_p5_s1.txt results/raw/demo_p1.csv --threads 8
```

Wybierz nową ścieżkę, jeśli katalog danych lub plik wynikowy już istnieje. Obok CSV każdy solver zapisuje indeksy końcowych stacji i metadane przebiegu. Opcja `--threads N` ustawia górną granicę liczby wątków P1. Warianty sekwencyjne można zbudować samodzielnie przez ustawienie `PMEDIAN_ENABLE_OPENMP=OFF`.

## Co udało się sprawdzić

Na 20 startach S1 i P1 zwróciły te same zestawy stacji i koszty co S0. Na instancji z 3000 punktami popytu, 180 kandydatami i 15 stacjami P1 z 16 wątkami skrócił medianę wyszukiwania z 109,0 ms do 30,9 ms. W czasie całego uruchomienia przyspieszenie wyniosło 2,78×. To wynik jednej serii na jednej maszynie. [Warunki pomiaru i pełna tabela](docs/iterations/03-p1-openmp.md)

## Dokumentacja

- [Model i cel projektu](docs/project-brief.md)
- [Przebieg algorytmu i format wyników](docs/architecture.md)
- [Generator i format danych](data/README.md)
- [Środowisko kompilacji](docs/environment-windows.md)
- [Metoda pomiaru i plan badań](docs/experiments.md)
- [Wyniki poszczególnych etapów](docs/iterations/README.md)
