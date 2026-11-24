---
name: pmedian-final-report
description: Używaj przy składaniu końcowego sprawozdania projektu p-median z briefu, literatury, kodu, notatek iteracji, konfiguracji, profilerów i wyników eksperymentów.
---

Przed pisaniem przeczytaj `docs/project-brief.md`, `docs/architecture.md`, `docs/experiments.md`, właściwe notatki iteracji i opisy eksperymentów. Traktuj surowe CSV, konfiguracje, profiler captures oraz źródła literaturowe jako dowody.

Ułóż sprawozdanie tak, aby dało się odtworzyć i ocenić pracę:

1. Cel i model ważonego p-median oraz ograniczenia danych syntetycznych.
2. Algorytm Vertex Substitution i kryteria poprawności.
3. Warianty S0/S1/S2, OpenMP i CUDA oraz uzasadnienie kolejnych optymalizacji.
4. Środowisko sprzętowe, kompilator, flagi, konfiguracje i metodologia pomiarów.
5. Wyniki poprawności i eksperymentów z tabelami lub wykresami wygenerowanymi z CSV.
6. Analiza przyspieszenia, efektywności, pamięci, transferów i ograniczeń.
7. Złożoność czasowa/pamięciowa, ograniczenia heurystyki oraz instrukcja odtworzenia.
8. Bibliografia spójna z cytowaniami w tekście.

Nie mieszaj wyników własnych z liczbami opublikowanymi w literaturze. Każdy wykres musi mieć źródłowe dane, opisane osie i jednostki. Rozdzielaj fakty, interpretacje i ograniczenia. Jeśli brakuje wyników, oznacz je jako niezmierzone; nie uzupełniaj ich przewidywaniami.

Nie twierdź, że lokalne optimum jest optimum globalnym. Nie porównuj speedup między różnymi maszynami ani różnymi zakresami pomiaru. Zachowaj znaczenie, warunki i ograniczenia źródeł podczas parafrazy.
