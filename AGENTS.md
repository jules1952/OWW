# Wskazówki dla agentów projektu

## Cel

Projekt bada wydajność ważonego problemu p-median rozwiązywanego heurystyką Vertex Substitution (Teitz–Bart). Powstają trzy porównywalne warianty w C++17: sekwencyjny CPU, wielowątkowy CPU z OpenMP oraz hybrydowy CPU+GPU z CUDA. CPU prowadzi iteracje, a GPU ocenia dużą liczbę możliwych zamian lokalizacji.

Przed pracą nad algorytmem przeczytaj `docs/project-brief.md` i `docs/architecture.md`. Przy pracy nad pomiarami lub dokumentacją użyj odpowiedniego workflow z `.agents/skills/`, w tym `pmedian-final-report` przy składaniu sprawozdania końcowego.

## Wspólne reguły

- Wszystkie mierzone wersje używają tego samego modelu, instancji, rozwiązania startowego i kryterium stopu.
- Wartość celu to suma `w_i * min_j d_ij`, gdzie `d_ij` jest odległością euklidesową.
- Wybieraj zamianę o najmniejszej delcie. Remisy rozstrzygaj leksykograficznie po `(out, in)`. Nie wprowadzaj osobnego, zależnego od backendu progu remisu.
- Wykonuj zamianę tylko, gdy `delta < -epsilon * cost`; przyjmij `epsilon = 1e-9`, chyba że uzasadniona decyzja projektowa zmieni tę wartość we wszystkich wersjach.
- Sumuj koszt i delty w `double`. Ewentualne użycie `float` dla odległości musi być jawnie opisane, zmierzone i sprawdzone pod kątem zgodności wyników.
- Warianty muszą zwracać ten sam zbiór stacji (`final_set_hash`) i koszt zgodny względnie do `1e-6` z referencją sekwencyjną.
- S0 pozostaje prostą referencją poprawności. Przyspieszenie raportuj między implementacjami C++ na tej samej maszynie i przy tym samym zakresie pomiaru.
- Python służy do generowania danych, uruchamiania eksperymentów i analizy. Nie używaj go do kodu, którego czas jest raportowany jako czas algorytmu.
- Nie dopisuj wartości profilowania ani wyników, których nie ma w surowych danych. Braki oznacz jako niezmierzone.
- Zachowuj surowe CSV, konfiguracje, opis sprzętu i notatki z odrzuconych optymalizacji. Duże wygenerowane instancje pozostają poza Gitem; zapisuj ich seedy i konfiguracje.

## Zasady zmian

- Kod ma być prosty do wyjaśnienia na zajęciach. Dodawaj komentarze po polsku przy istotnych krokach i opisuj cel funkcji, reguły losowania oraz przyczyny walidacji. W dokumentacji wyjaśniaj przebieg działania i zakres sprawdzeń; unikaj komentarzy powtarzających oczywistą składnię.
- Każdą optymalizację uzasadnij profilem, hipotezą i pomiarem przed oraz po zmianie.
- Oddziel zmianę algorytmiczną od zmiany równoległości, formatu danych i precyzji numerycznej.
- Nie zmieniaj reguł algorytmu tylko w jednym backendzie. Jeśli specyfikacja wymaga doprecyzowania, opisz decyzję w dokumentacji przed implementacją.
- Nie twierdź, że skalowanie klastrowe jest obsługiwane, dopóki nie zostanie ustalony i zaimplementowany model wielowęzłowy.
- Polecenia budowania i walidacji dodaj do README, kiedy będzie istniał działający kod i wybrany system budowania.
