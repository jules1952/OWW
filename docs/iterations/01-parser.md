# Wspólny parser formatu v1

## Identyfikacja

- Etap przygotowania do S0, 24 listopada 2026.
- Walidacja na lokalnym Windows x64 z MSVC 19.52.37029, C++17, CMake i Ninja, konfiguracja Release.
- Instancje `uniform_demo` i `clustered_demo`, każda z `n=1000`, `m=100` i 10 startami dla każdego `p` ze zbioru {5, 10, 20}.

## Zmiana

Biblioteka `pmedian_common` wczytuje osobno instancję i start. Przechowuje współrzędne i wagi w `double`, a kolejność kandydatów z pliku zachowuje jako podstawę indeksowania od zera. Odczyt wierszami pozwala wskazać miejsce błędu i wykryć pola przeniesione do innego wiersza. Start przechodzi walidację, jeśli ma dokładnie `p` ściśle rosnących indeksów z zakresu `[0, m)`.

## Pomiar i poprawność

- Czasy, liczniki profilu, koszt względem S0 i `final_set_hash` są niezmierzone. Etap obejmuje odczyt danych.
- CTest potwierdził odczyt wartości i kolejności kandydatów, obsługę LF i CRLF, przypadki `p=1` i `p=m` oraz diagnostykę brakujących, nadmiarowych i niepoprawnych danych.
- Dodatkowe uruchomienie wczytało obie instancje demonstracyjne i wszystkie 60 wygenerowanych startów.

## Decyzja

Przyjęto parser jako wspólne wejście do S0 i kolejnych wariantów.
