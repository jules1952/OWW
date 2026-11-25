# Cel projektu i model p-median

Dane wejściowe opisują punkty popytu i możliwe miejsca na stacje rowerowe. Każdy punkt popytu ma położenie i wagę odpowiadającą liczbie mieszkańców, których reprezentuje. Dla ustalonej liczby stacji `p` wybieramy spośród kandydatów taki zestaw, który minimalizuje sumę ważonych odległości do najbliższej stacji.

Dla zbioru wybranych stacji `S` koszt to `sum_i w_i * min_{j in S} d(i, j)`, gdzie `w_i` jest wagą punktu, a `d(i, j)` odległością euklidesową. Musi zachodzić `1 <= p <= m`, gdzie `m` oznacza liczbę kandydatów.

## Jak szukamy rozwiązania

Vertex Substitution zaczyna od gotowego zestawu `p` stacji. W każdej iteracji sprawdza wszystkie zamiany jednej wybranej stacji na jedną niewybraną. Wybiera ruch o najmniejszej różnicy kosztu `delta = nowy_koszt - bieżący_koszt`. Przy równych deltach decyduje najpierw indeks usuwanej stacji `out`, a potem indeks dodawanej `in`. [Teitz i Bart, Operations Research, 1968](https://pubsonline.informs.org/doi/10.1287/opre.16.5.955)

Ruch wykonujemy tylko przy `delta < -1e-9 * bieżący_koszt`. Jeśli żadna zamiana nie spełnia tego warunku, program kończy pracę. Wynik zależy od startu i opisuje rozwiązanie, którego heurystyka nie poprawiła już pojedynczą zamianą spełniającą próg.

## Co porównujemy

| Etap | Rola w projekcie |
|---|---|
| S0 | Prosta wersja sekwencyjna przeliczająca cały koszt po każdej próbnej zamianie. Służy do sprawdzania poprawności. |
| S1 | Sekwencyjna ocena zamian z użyciem zapamiętanych odległości. Punkt odniesienia dla wydajności. |
| P1 | Równoległa ocena zamian z OpenMP. |
| S2 | Opcjonalna dalsza optymalizacja CPU oparta na układzie danych i wektoryzacji. |
| G1 | CPU prowadzi iteracje, a każdy wątek GPU ocenia jedną zamianę. |
| G2 | Planowana ocena jednej zamiany przez blok GPU, z podziałem punktów między wątki. |
| G3 | Planowane ograniczenie transferów przez pozostawienie danych i aktualizacji stanu na GPU. |

Dla każdej porównywanej pary używamy tych samych współrzędnych, wag i stacji początkowych. Koszt i delty liczymy w `double`. Wymagamy tego samego końcowego zestawu stacji co w S0 oraz względnej różnicy kosztu do `1e-6`. Sumę kontrolną zestawu (`final_set_hash`) liczymy po posortowaniu indeksów.

G1 liczy odległości na CPU za pomocą tego samego `std::hypot` co S1 i przekazuje macierz odległości do GPU. Koszt pojedynczej zamiany sumuje jeden wątek, w kolejności punktów z pliku, z osobnym zaokrągleniem mnożenia i dodawania. Wybór ruchu i próg poprawy pozostają wspólne z wariantami CPU.

Pomiar obejmuje osobno przygotowanie danych, wyszukiwanie i pełne uruchomienie programu. Dla CUDA dochodzą czasy transferu w obu kierunkach. Zużycie pamięci i liczbę przesłanych bajtów raportujemy, gdy są dostępne. [Plan badań](experiments.md)

## Stan prac

Działają generator, wspólny parser oraz S0, S1, P1 i G1. Instancje mają format tekstowy v1 i są odtwarzalne z parametrów i ziaren losowania. G1 sprawdzono na RTX 3070. Ustalenie progu opłacalności GPU wymaga pomiarów dla różnych rozmiarów instancji. Zakres ewentualnej pracy wielowęzłowej, w tym użycie MPI, wymaga ustalenia. [Format danych](../data/README.md) · [Środowisko](environment-windows.md)
