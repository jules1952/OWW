---
name: pmedian-experiment-analysis
description: Używaj przy planowaniu, uruchamianiu lub analizowaniu badań E1–E7 dla p-median, zwłaszcza tabel, wykresów, speedup, skalowania CPU i progu opłacalności GPU.
---

Przeczytaj `docs/experiments.md` oraz `docs/experiment-template.md`. Wejściem powinny być konfiguracja badania i surowe CSV; wyjściem jest odtwarzalny opis metody, podsumowanie wyników i analiza ograniczeń.

1. Sprawdź, czy porównywane wiersze dotyczą tych samych instancji, startów, parametrów, maszyny i zakresu czasu.
2. Zachowaj rozgrzewkę oddzielnie. Podsumuj powtórzenia medianą i IQR albo odchyleniem standardowym; podaj liczbę powtórzeń.
3. Speedup licz względem sekwencyjnego C++ na tej samej maszynie. Dla GPU przedstaw zarówno czas end-to-end z transferami, jak i rozkład składowych.
4. Opisz osie, jednostki, konfigurację i źródłowy plik CSV dla każdego wykresu.
5. Omawiaj zgodność wyniku i kosztu razem z czasem. Nie utożsamiaj lokalnego optimum heurystyki z optimum globalnym.
6. Nie uzupełniaj brakujących obserwacji fikcyjnymi wartościami. Oddziel wnioski od ograniczeń danych.

Jeśli konfiguracja lub dane nie wystarczą do uczciwego porównania, opisz, czego brakuje, i przygotuj poprawiony plan zamiast wyciągać wniosek o wydajności.
