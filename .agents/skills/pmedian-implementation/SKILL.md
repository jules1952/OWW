---
name: pmedian-implementation
description: Używaj przy implementowaniu lub optymalizacji wspólnego algorytmu p-median, wariantu sekwencyjnego, OpenMP albo CUDA; prowadź zmiany od hipotezy i profilu do porównywalnej walidacji.
---

Przeczytaj `docs/project-brief.md` i `docs/architecture.md`. Przed kodowaniem wskaż, czy zmiana dotyczy semantyki, reprezentacji danych, równoległości, transferów czy pomiaru.

1. Zachowaj wspólny model ważonego p-median i ruch pojedynczej zamiany.
2. Dla optymalizacji wydajnościowej nazwij wąskie gardło i wskaż profil albo zmierz bazę.
3. Wprowadzaj jedną ukierunkowaną zmianę w iteracji. Nie łącz równolegle zmian algorytmicznych, precyzji i układu pamięci bez osobnych pomiarów.
4. Zachowaj wspólne dane, start, kryterium stopu i deterministyczny wybór `(out, in)`. Sumuj w `double`; wpływ `float` opisz osobno.
5. Oceniaj wynik względem S0: ten sam zbiór stacji i koszt w tolerancji względnej `1e-6`.
6. Raportuj zakres walidacji i rzeczywiście uzyskane pomiary. Nie oznaczaj pracy jako zweryfikowanej, jeśli jej nie sprawdzono.
7. Uzupełnij notatkę w `docs/iteration-template.md`, także gdy optymalizacja zostanie odrzucona.

Nie projektuj osobnej funkcji celu lub reguły stopu dla CPU i GPU. Jeśli wymagania są niejednoznaczne, zapisz konkretną decyzję do `docs/project-brief.md` przed rozchodzeniem się implementacji.
