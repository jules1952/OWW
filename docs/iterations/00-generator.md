# Przygotowanie generatora danych

Status: infrastruktura wejściowa, przed implementacją S0.

Dodano generator oparty na standardowej bibliotece Python, z rozkładami równomiernym i skupionym, wagami jednostkowymi lub lognormalnymi oraz zapisanymi zestawami początkowymi dla wielu p i seedów.

Przyjęto tekstowy format v1 oraz indeksowanie kandydatów od 0. Kandydaci, geometria i wagi używają niezależnych strumieni losowania. Starty zależą od m, p i własnego seeda. Pliki można podać bez zmian do seq, omp i cuda. Manifest zapisuje pochodzenie i sumy SHA-256.

Wagi lognormalne są niezależne od osiedli: jest to jawna decyzja umożliwiająca porównanie wag jednostkowych i zmiennych przy tej samej geometrii. Mocniejsze ważenie osiedli pozostaje możliwym osobnym scenariuszem.

To nie jest optymalizacja solvera: czasy profilowania, koszt końcowy i zgodność z S0 nie mają jeszcze zastosowania. S0 nie zostało zaimplementowane.

## Wykonana walidacja

W Python 3.12 dołączonym do środowiska Codex sprawdzono bajtową powtarzalność całego katalogu dla tego samego zestawu parametrów, zakres współrzędnych, dodatnie całkowite wagi, unikalność i porządek indeksów startowych, przypadki p=1 i p=m oraz sumy SHA-256. Potwierdzono identycznych kandydatów i starty między rozkładami oraz identyczną geometrię po zmianie wag na jednostkowe. Sprawdzono odrzucanie niepoprawnych parametrów i ochronę istniejącego katalogu. Wszystkie kontrole przeszły.

Wygenerowano `data/generated/uniform_demo` i `data/generated/clustered_demo`, każde z n=1000, m=100, p w {5,10,20}, seedem instancji 1 i 10 startami dla każdego p. Parametry i dokładna wersja Python są w manifestach. Pliki wynikowe są lokalne i ignorowane przez Git.
