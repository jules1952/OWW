# Pomiar end-to-end i walidacja S0/S1

Serię wykonano 25 listopada 2026 r. na Windows 10 (19045), na procesorze Intel Core i5-12600KF z 16 wątkami logicznymi i 32 GiB RAM. Kompilator MSVC 19.52.37029.0, Release x64, C++17, flagi optymalizacji `/O2 /Ob2 /DNDEBUG`. Oba solvery pracowały sekwencyjnie na CPU. Manifest odnotowuje bazowy commit `bdf0bdf`, lokalne zmiany oraz sumy SHA-256 kodu, plików wykonywalnych i wejścia. [Manifest serii](../../results/raw/seq-s0-s1-end-to-end/manifest.json).

## Zakres pomiaru

W poprzedniej serii `t_total` powielał czas solvera. W tej serii `t_prep` mierzy odczyt instancji i startu, a `t_search` mierzy wywołanie solvera. Zewnętrzny zegar obejmuje uruchomienie procesu, odczyt danych, obliczenia, zapis plików i odbiór wyjścia na konsolę. Skrypt zapisuje ten czas w `t_total` po zakończeniu procesu. Przy uruchomieniu samego programu pole `t_total` jest puste.

Dla każdego solvera wykonano jedną rozgrzewkę z `rep=0`, pięć pomiarów z `rep=1–5` i osobny przebieg z profilowaniem oraz pustym `rep`. W pomiarach zmieniano kolejność S0/S1 między kolejnymi powtórzeniami. Skrypt porównuje koszt z tolerancją względną `1e-6`, zestaw stacji, hash i liczbę iteracji. Manifest odczytuje kompilator, konfigurację i ustawienie profilowania bezpośrednio z użytych programów. [Tabela pomiarów](../../results/raw/seq-s0-s1-end-to-end/measurements.csv) · [Rozgrzewki](../../results/raw/seq-s0-s1-end-to-end/warmup.csv) · [Przebiegi profilowane](../../results/raw/seq-s0-s1-end-to-end/profile-runs.csv).

## Wyniki

Użyto instancji z rozkładem równomiernym, wagami lognormalnymi i seedem 1 (`n=1000, m=100, p=10`). Instancja i start `start_p10_s1.txt` mają te same sumy kontrolne co w poprzedniej serii. Mediany i IQR poniżej pochodzą z pięciu pomiarów bez profilowania. [Konfiguracja instancji](../../bench/configs/seq-profile-instance.json) · [Surowe wyniki](../../results/raw/seq-s0-s1-end-to-end/).

| Wariant | Mediana `t_prep` | Mediana `t_search` (IQR) | Mediana `t_total` (IQR) |
|---|---:|---:|---:|
| S0 | 1,290 ms | 345,000 ms (7,082 ms) | 354,001 ms (6,813 ms) |
| S1 | 1,344 ms | 8,863 ms (0,608 ms) | 17,212 ms (0,821 ms) |

Koszt obu wariantów wyniósł `32541187.262297336`, hash `b6d7ddd3409dc132`, a liczba zaakceptowanych zamian 9. S1 był **38,93×** szybszy w obliczeniach i **20,57×** szybszy w pełnym uruchomieniu na tej instancji. Poprzedni czas end-to-end jest niezmierzony, a wcześniejsze czasy solvera pochodzą z innej maszyny. [Profil faz](../../results/raw/seq-s0-s1-end-to-end/phases.csv).

## Odtworzenie i walidacja

Z katalogu głównego w terminalu Visual Studio uruchom poniższe polecenia. Wybierz nowy katalog wyników, jeśli wskazany już istnieje.

```powershell
cmake -S . -B build/seq-end-to-end-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DPMEDIAN_PROFILE_SEQ=OFF
cmake --build build/seq-end-to-end-release
ctest --test-dir build/seq-end-to-end-release --output-on-failure
cmake -S . -B build/seq-end-to-end-profile -G Ninja -DCMAKE_BUILD_TYPE=Release -DPMEDIAN_PROFILE_SEQ=ON
cmake --build build/seq-end-to-end-profile
ctest --test-dir build/seq-end-to-end-profile --output-on-failure
python data/gen.py --n 1000 --m 100 --p 10 --distribution uniform --seed 1 --out data/generated/seq_end_to_end
python bench/run_seq_profile.py build/seq-end-to-end-release build/seq-end-to-end-profile data/generated/seq_end_to_end/instance.txt data/generated/seq_end_to_end/start_p10_s1.txt results/raw/seq-s0-s1-end-to-end-repeat
```

Manifest osobnej kompilacji można zapisać przez `python bench/configs/record_build.py --help`, które pokazuje argumenty dla czterech plików wykonywalnych i ścieżki wyjściowej.

W obu konfiguracjach przeszedł CTest, w tym osiem przypadków S0/S1 z oczekiwanym kosztem, zestawem stacji i liczbą iteracji. Sprawdzono również CSV, ustawienia profilowania w binariach oraz sumy wejścia i plików wykonywalnych. Nowy sposób pomiaru przyjęto na podstawie tych kontroli.
