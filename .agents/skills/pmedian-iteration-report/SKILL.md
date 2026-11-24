---
name: pmedian-iteration-report
description: Używaj do dokumentowania iteracji S0/S1/S2/P1/G1/G2/G3, wyników profilowania, optymalizacji zaakceptowanych lub odrzuconych oraz decyzji o zakończeniu optymalizacji.
---

Uzupełnij `docs/iteration-template.md` na podstawie kodu, commitów/tagów, profilerów i surowych pomiarów dostępnych w repozytorium.

- Opisz hipotezę o wąskim gardle i dowód, na którym ją oparto.
- Zapisz konkretną zmianę, wariant, kompilator, flagi, maszynę, instancję i rozwiązanie startowe.
- Porównuj pomiary przed i po przy identycznym zakresie pracy. Dla GPU rozdziel przygotowanie, H2D, kernel/search i D2H.
- Podaj koszt, zbiór stacji lub hash, liczbę iteracji i sposób sprawdzenia zgodności z S0.
- Rozróżnij wynik pomiaru od interpretacji. Dołącz ścieżkę do raportu profilu lub CSV.
- Zakończ decyzją: przyjęta, odrzucona albo optymalizacja zakończona, z uzasadnieniem liczbowym, jeśli dane istnieją.

Brakujących liczb nie estymuj. Wpisz „niezmierzone” i wskaż brakujący artefakt. Dokumentuj także nieudane i odrzucone podejścia.
