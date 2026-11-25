# Parser formatu v1

Instancję i zestaw początkowy wczytuje biblioteka `pmedian_common`. Kolejność kandydatów w pliku jest podstawą ich numeracji od zera. Parser sprawdza każdy wiersz osobno, dzięki czemu pole przeniesione do następnego wiersza daje błąd w miejscu, w którym go brakuje. Start musi mieć dokładnie `p` ściśle rosnących indeksów z zakresu od 0 do `m - 1`. [Dokładny format plików](../../data/README.md)

## Walidacja

Testy przeprowadzono 24 listopada 2026 r. w Windows, w konfiguracji Release x64 z MSVC 19.52.37029, CMake i Ninja. CTest potwierdził poprawny odczyt wartości i kolejności kandydatów, obsługę zakończeń wierszy LF i CRLF oraz przypadki `p = 1` i `p = m`. Testy sprawdziły też komunikaty dla brakujących, nadmiarowych i niepoprawnych danych.

Dodatkowa kontrola wczytała obie instancje demonstracyjne (`n = 1000`, `m = 100`) i wszystkie 60 zestawów początkowych. Po tych testach parser stał się wspólnym wejściem dla solverów.
