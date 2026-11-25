# Brief projektu p-median

## Cel

Porównać wydajność i jakość trzech implementacji Vertex Substitution dla ważonego problemu p-median z zastosowaniem do rozmieszczenia stacji rowerowych. Porównywane są: sekwencyjny CPU, wielowątkowy CPU z OpenMP oraz hybrydowy CPU+GPU z CUDA. Wszystkie wersje powinny pracować na tych samych danych i stosować te same reguły algorytmu.

## Wymagania wynikające z materiałów źródłowych

- Zbiór punktów zapotrzebowania `D` ma współrzędne i wagę `w_i`; zbiór kandydatów `F` zawiera możliwe lokalizacje stacji.
- Dla ustalonego `1 <= p <= |F|` minimalizowany jest koszt `sum_i w_i * min_{j in S} d_ij`, gdzie `|S| = p`, a odległość jest euklidesowa.
- Metodą lokalnego przeszukiwania jest Vertex Substitution: w każdej iteracji oceniane są zamiany jednej wybranej lokalizacji na jedną niewybraną; wybierana jest najlepsza zamiana.
- Sekwencyjne S0 jest prostą referencją. Możliwe etapy optymalizacji obejmują S1 z odległością najbliższej/drugiej najbliższej stacji, S2 z układem danych i wektoryzacją, P1 z OpenMP oraz G1–G3 w CUDA. S3/G4 pozostają rozszerzeniami opcjonalnymi.
- Wybrana zamiana ma najmniejszą deltę, a remisy rozstrzyga para `(out, in)` w porządku leksykograficznym. Ruch wykonuje się, gdy `delta < -epsilon * cost`; plan źródłowy sugeruje `epsilon = 1e-9`.
- Sumy liczone są w `double`. Użycie `float` do odległości jest osobną decyzją, która wymaga pomiaru i kontroli zgodności.
- Wszystkie implementacje mają zwrócić taki sam zbiór stacji; koszt końcowy ma być zgodny z S0 z tolerancją względną `1e-6`.
- C++17 jest językiem kodu mierzonego. Python służy do generowania danych, orkiestracji eksperymentów i analizy.
- Wersja CUDA korzysta z CPU do sterowania iteracjami, a GPU do równoległej oceny zamian. W pomiarach GPU rozdzielane są przygotowanie, transfery H2D/D2H i wyszukiwanie.

## Dane i odtwarzalność

Generator ma tworzyć rozkład równomierny oraz skupiony, z wagami równymi lub zmiennymi. Instancje i rozwiązania startowe powinny być powtarzalne z seedów. Wszystkie warianty używają tych samych plików instancji i startów. Dużych wygenerowanych danych nie trzeba trzymać w repozytorium, jeśli są odtwarzalne z kodu i konfiguracji.

Proponowany tekstowy format wejściowy ułatwia inspekcję na początku projektu: nagłówek `n m`, następnie `n` wierszy `x y w` i `m` wierszy `x y`; osobny plik startowy zawiera `p` i indeksy kandydatów. Format binarny można wprowadzić po pomiarze, jeśli tekst stanie się kosztem.

## Kryteria porównania

- Zgodność: hash posortowanego końcowego zbioru stacji oraz koszt względem S0.
- Czas: przygotowanie, transfer H2D, wyszukiwanie, transfer D2H i całkowity czas end-to-end.
- Zasoby: szczytowy RAM, pamięć GPU i transferowane bajty, jeśli są dostępne.
- Każdy wiersz wyników musi pozwalać odtworzyć wariant, instancję, `n`, `m`, `p`, odległość, liczbę wątków i powtórzenie.

## Plan implementacji z materiałów

| Etap | Zakres | Rola w projekcie |
|---|---|---|
| S0 | Pełne przeliczenie kosztu dla każdej zamiany | Referencja poprawności |
| S1 | Przechowywanie najbliższej i drugiej najbliższej stacji | Ograniczenie ponownych obliczeń |
| S2 | SoA, wektoryzacja i badanie reprezentacji odległości | Optymalizacja sekwencyjna CPU |
| P1 | Równoległa ocena zamian przez OpenMP | Wariant wielordzeniowego CPU |
| G1 | Wątek CUDA ocenia zamianę | Pierwszy wariant hybrydowy |
| G2 | Wątki bloku dzielą punkty, redukcja GPU | Zmiana mapowania pracy |
| G3 | Dane i aktualizacje rezydują na GPU | Ograniczenie transferu w iteracji |

## Decyzje do potwierdzenia

1. Czy wymóg skalowania od jednego węzła do całego systemu oznacza klaster i MPI? Materiały nie definiują jeszcze wariantu wielowęzłowego.
2. Lokalnie wykryto RTX 3070 8 GB. Wybrano natywny Windows z MSVC i CUDA Toolkit; WSL2 jest alternatywą. Kompilacja i uruchomienie CUDA wymagają jeszcze potwierdzenia po instalacji narzędzi.
3. Jaka będzie ostateczna reguła numeryczna dla remisów prawie równych delt, szczególnie przy redukcji równoległej i mieszanej precyzji? Nie należy dodawać różnych progów w poszczególnych backendach.
4. Generator używa tekstowego formatu v1 z indeksami kandydatów od 0; szczegóły opisano w `data/README.md`. Format binarny pozostaje możliwą przyszłą optymalizacją.

## Zakres tego szkieletu

Dokumenty źródłowe zawierają także terminy zajęć i oddań. Repozytorium ma generator danych, parser C++17, warianty S0, S1 i P1 oraz polecenia budowania i uruchamiania w README. Wariant CUDA i pełna seria E1–E7 pozostają do wykonania.
