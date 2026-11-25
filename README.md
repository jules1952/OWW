# Projekt p-median dla stacji rowerowych

Repozytorium przygotowuje porównanie trzech implementacji heurystyki Vertex Substitution dla ważonego problemu p-median: sekwencyjnej na CPU, wielowątkowej z OpenMP oraz hybrydowej CPU+GPU z CUDA. C++17 obsługuje obliczenia mierzone, a Python generowanie instancji, uruchamianie eksperymentów i analizę wyników.

Repozytorium zawiera generator danych, wspólny parser C++17 oraz dwa etapy wersji sekwencyjnej.

## Generowanie pierwszych danych

W terminalu otwartym w katalogu projektu uruchom:

```powershell
python data/gen.py --n 1000 --m 100 --p 5 10 20 --distribution uniform --seed 1 --out data/generated/uniform_demo
python data/gen.py --n 1000 --m 100 --p 5 10 20 --distribution clustered --seed 1 --out data/generated/clustered_demo
```

Każde uruchomienie zapisuje instancję, po 10 rozwiązań startowych dla każdego p oraz manifest parametrów i sum kontrolnych. Wybierz nowy katalog `--out`, jeśli wskazany już istnieje. Generator nie wymaga dodatkowych bibliotek. Szczegóły i format wejścia opisano w [instrukcji generatora](data/README.md).

## Budowanie i uruchamianie wersji sekwencyjnej

Uruchom polecenia z głównego katalogu projektu. W Windows użyj **x64 Native Tools Command Prompt for Visual Studio** z CMake i Ninja w `PATH`. Testy tworzą własne dane w katalogu kompilacji.

```text
cmake -S . -B build/ninja-seq -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/ninja-seq
ctest --test-dir build/ninja-seq --output-on-failure
build/ninja-seq/pm_s0.exe data/generated/uniform_demo/instance.txt data/generated/uniform_demo/start_p5_s1.txt results/raw/demo_s0.csv
build/ninja-seq/pm_s1.exe data/generated/uniform_demo/instance.txt data/generated/uniform_demo/start_p5_s1.txt results/raw/demo_s1.csv
```

Program przyjmuje instancję, rozwiązanie startowe i nazwę nowego pliku CSV. Obok CSV zapisuje końcowy zestaw stacji i metadane.

S0 przelicza pełny koszt każdej próbnej zamiany. S1 zapamiętuje dla każdego punktu dwie najmniejsze odległości do wybranych stacji. W każdej iteracji liczy odległości do dodawanego kandydata raz i wykorzystuje je we wszystkich próbach z tym kandydatem. Oba programy oceniają wszystkie zamiany, wybierają najlepszą i kończą, gdy poprawa nie przekracza ustalonego progu. [Dokumentacja S0 i S1](docs/iterations/S0-S1.md) zawiera historię zmian, profilowanie i instrukcję odtworzenia pomiarów.

Wspólny moduł `pmedian_common` udostępnia `read_instance` i `read_start` przez [input.hpp](src/common/input.hpp). Parser sprawdza układ wierszy i wartości, a przy błędzie zgłasza wyjątek z nazwą pliku i numerem wiersza, jeśli udało się rozpocząć odczyt.

## Zacznij tutaj

1. Przeczytaj [brief projektu](docs/project-brief.md) i [architekturę](docs/architecture.md).
2. Uzgodnij otwarte decyzje wymienione w briefie, zwłaszcza dostępność GPU i wymagania dotyczące klastra.
3. Implementuj kolejne wersje od referencyjnego S0. Nie porównuj backendów na różnych danych ani różnych rozwiązaniach startowych.
4. Dokumentuj każdą optymalizację według [szablonu iteracji](docs/iteration-template.md), a badania według [szablonu eksperymentu](docs/experiment-template.md).

## Układ projektu

```text
.
├── .agents/skills/       # workflow implementacji, dokumentacji i badań
├── .codex/agents/        # wyspecjalizowane role Codex
├── analysis/             # wykresy i analiza surowych CSV
├── bench/configs/        # konfiguracje eksperymentów
├── data/                 # generator i instancje; duże dane generowane lokalnie
├── docs/                 # brief, architektura, metodologia i notatki
├── profiles/             # raporty profilowania CPU i GPU
├── results/raw/          # surowe wyniki CSV
├── src/common/           # wspólny model danych i reguły algorytmu
├── src/seq/              # implementacja sekwencyjna
├── src/omp/              # implementacja OpenMP
├── src/cuda/             # implementacja CUDA sterowana przez CPU
└── tests/                # testy parsera i przyszłe kontrole zgodności
```

## Agenci i skills

Agenci projektowi znajdują się w `.codex/agents/`: `pmedian_core`, `pmedian_openmp`, `pmedian_cuda` i `pmedian_research`. Skills w `.agents/skills/` opisują powtarzalną pracę nad implementacją, notatkami z iteracji, analizą eksperymentów i sprawozdaniem końcowym. Można poprosić Codex o użycie agenta po nazwie lub wywołać skill przez `$pmedian-implementation`, `$pmedian-iteration-report`, `$pmedian-experiment-analysis` albo `$pmedian-final-report`.

## Środowisko obliczeniowe

Kod mierzony jest przeznaczony dla C++17, OpenMP i CUDA C++. Lokalnie projekt będzie rozwijany natywnie w Windows z MSVC i CUDA Toolkit; WSL2 pozostaje alternatywą. Jeśli GPU nie ma, materiały źródłowe wskazują Kaggle jako wariant zapasowy; porównanie CPU–GPU powinno wtedy odbywać się w całości na tej samej maszynie Kaggle. Nie zapisuj wniosków o wydajności przed uzyskaniem porównywalnych pomiarów.

## Dokumentacja projektu

- [Wymagania i decyzje otwarte](docs/project-brief.md)
- [Granice wariantów i kontrakt wejścia/wyjścia](docs/architecture.md)
- [Konfiguracja Windows, WSL2 i CUDA](docs/environment-windows.md)
- [Plan badań E1–E7](docs/experiments.md)
- [Szablon notatki iteracji](docs/iteration-template.md)
- [Szablon opisu eksperymentu](docs/experiment-template.md)
