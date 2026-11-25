# Generator danych

Generator przygotowuje instancje równomierne i skupione, z wagami jednostkowymi lub lognormalnymi. Dla każdej podanej wartości `p` zapisuje zestawy początkowe, których później używają wszystkie porównywane solvery. Wybór oddzielnych strumieni losowania dla punktów, kandydatów i wag pozwala zmienić rodzaj wag przy tej samej geometrii. [Format danych i parametry](../../data/README.md)

## Walidacja

W Pythonie 3.12 sprawdzono, że ponowne uruchomienie z tymi samymi parametrami daje te same pliki. Kontrola objęła zakres współrzędnych, dodatnie całkowite wagi, poprawne sumy SHA-256 oraz rosnące i unikalne indeksy startowe, również dla `p = 1` i `p = m`. Zmiana rozkładu zachowała kandydatów i indeksy startowe, a zmiana wag zachowała współrzędne. Sprawdzono również odrzucanie niepoprawnych parametrów i ochronę istniejącego katalogu przed nadpisaniem.

Instancje demonstracyjne `uniform_demo` i `clustered_demo` miały po 1000 punktów i 100 kandydatów. Dla każdej wartości `p` ze zbioru 5, 10 i 20 wygenerowano 10 startów. Ziarno obu instancji wynosiło 1, a pliki utworzono lokalnie w `data/generated/`.
