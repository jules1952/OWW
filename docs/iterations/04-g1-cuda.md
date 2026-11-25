# Równoległa ocena zamian w CUDA G1

## Hipoteza i implementacja

W profilu S1 na instancji `n = 1000`, `m = 100`, `p = 10` ocena próbnych zamian i obliczenie odległości do kandydatów zajęły łącznie 8,24 ms, wobec 0,86 ms na odświeżanie najbliższych stacji. W G1 przenieśliśmy równoległą ocenę kosztów zamian na GPU. Każdy wątek liczy jeden koszt, a CPU wybiera ruch i aktualizuje stan. Macierz odległości jest liczona na CPU przez to samo `std::hypot` co w S1 i przesyłana jednorazowo razem z wagami. W pojedynczym koszcie kolejność punktów i osobne zaokrąglenia w `double` odpowiadają kompilacji referencji użytej w pomiarze. [Profil S1](../../results/raw/seq-s0-s1-end-to-end/phases.csv) · [Przebieg G1](../architecture.md)

## Walidacja E1

G1 zwrócił dokładnie ten sam koszt, zestaw stacji, hash i liczbę zamian co S0 dla wszystkich 20 startów. Użyto po dziesięć startów dla instancji skupionej z wagami lognormalnymi oraz równomiernej z wagami jednostkowymi (`n = 200`, `m = 30`, `p = 5`). CTest sprawdził również przypadki remisów, granicy `epsilon`, wszystkich stacji wybranych, wyniku z ostatniego niepełnego bloku CUDA oraz przepełnienia odległości i kosztu. Przeszły kompilacje i testy Release z CUDA i OpenMP oraz w konfiguracji sekwencyjnej. Compute Sanitizer uruchomiony w trybie `memcheck` na binarium testów solverów zgłosił zero błędów. [Surowa walidacja](../../results/raw/g1-initial/validation.csv)

## Pierwszy pomiar

Jedną serię wykonano na i5-12600KF i RTX 3070, pod Windows 10 (19045). Użyto konfiguracji Release x64, MSVC 19.52.37029.0, OpenMP 2.0 z `OMP_DYNAMIC=FALSE` oraz CUDA 13.5.37 dla `sm_86` ze sterownikiem 621.01. Dane są z tej samej instancji co w E3. To rozkład równomierny z wagami lognormalnymi i ziarnem 3 (`n = 3000`, `m = 180`, `p = 15`), ze startem `start_p15_s1.txt`. Wykonano jedną rozgrzewkę i pięć pomiarów na konfigurację, zmieniając kolejność wariantów między powtórzeniami. Tabela podaje medianę i IQR w milisekundach. Zestaw zawiera S1, P1 z najkrótszą medianą całego czasu w tej serii oraz G1. [Surowe pomiary](../../results/raw/g1-initial/scaling.csv) · [Rozgrzewka](../../results/raw/g1-initial/warmup.csv) · [Manifest](../../results/raw/g1-initial/manifest.json)

| Wariant | Przygotowanie | H2D | Wyszukiwanie | D2H | Cały proces |
|---|---:|---:|---:|---:|---:|
| S1 | 3,842 (2,392) | — | 105,600 (4,428) | — | 118,228 (4,826) |
| P1, 16 wątków | 3,777 (2,569) | — | 29,726 (3,113) | — | 41,389 (3,637) |
| G1 | 102,227 (12,063) | 2,036 (0,119) | 15,968 (0,337) | 0,530 (0,006) | 146,440 (9,405) |

Wszystkie przebiegi zgodziły się z S0. Koszt wyniósł `77219654.755592793`, hash `5d92de54509c088e`, a liczba przyjętych zamian 15. G1 przydzielił 4,23 MiB buforów na GPU i przesłał 5 519 040 bajtów do GPU oraz 316 800 bajtów do CPU. Szczytowe użycie RAM i pamięć zajęta przez środowisko CUDA są niezmierzone. [Surowe pomiary](../../results/raw/g1-initial/scaling.csv)

## Decyzja

G1 przyjęto jako punkt odniesienia dla kolejnych wersji CUDA. Na tej instancji całe uruchomienie G1 trwało 1,24× dłużej niż S1. Największą mierzoną fazą G1 było przygotowanie, które obejmuje obliczenie macierzy odległości, inicjalizację CUDA i przydział pamięci. Udziały tych operacji w przygotowaniu są niezmierzone. S1 oblicza odległości podczas wyszukiwania, więc przyspieszenie G1 względem S1 liczymy z czasu całego procesu. Próg opłacalności pozostaje do ustalenia w E5. [Metoda pomiaru](../experiments.md)

## Odtworzenie

Zbuduj Release zgodnie z README i przygotuj instancje E1 oraz E3 poleceniami z notatki P1. Do kolejnej serii użyj nowego katalogu wyników. Zapisana seria użyła PowerShell do uruchamiania tych samych binariów z konfiguracją poniżej, ponieważ lokalny launcher Pythona zwracał odmowę dostępu. [Kompilacja](../../README.md#cuda) · [Odtworzenie instancji](03-p1-openmp.md#odtworzenie)

```powershell
python bench/run_omp_scaling.py build/g1-release bench/configs/p1-e1-e3.json results/raw/g1-repeat --include-cuda
```
