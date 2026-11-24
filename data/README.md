# Generowanie danych

Generator `data/gen.py` wymaga Python 3.9 lub nowszego i korzysta wyłącznie z biblioteki standardowej. Uruchamiaj go z głównego katalogu repozytorium. W Windows użyj `py`, a w Linux `python3`.

```powershell
py data/gen.py --n 1000 --m 100 --p 5 10 20 --distribution uniform --seed 1 --out data/generated/uniform_demo
py data/gen.py --n 1000 --m 100 --p 5 10 20 --distribution clustered --seed 1 --out data/generated/clustered_demo
```

Każdy katalog zawiera `instance.txt`, pliki `start_p<P>_s<SEED>.txt` oraz `manifest.json`. Domyślnie powstaje 10 startów dla każdej wartości p (seedy 1–10). Wszystkie trzy backendy mają czytać dokładnie te same pliki. Generator odmawia zapisu do istniejącego katalogu, aby zachować dane i ich pochodzenie.

## Format tekstowy v1

Plik `instance.txt` jest zapisany w UTF-8, bez komentarzy. Pierwszy wiersz zawiera `n m`, następne n wierszy `x y w`, a kolejne m wierszy `x y`. Współrzędne są w metrach, separator dziesiętny to kropka, a wagi są dodatnimi liczbami całkowitymi. Format da się odczytać operatorem `>>` w C++.

Plik startowy zawiera p w pierwszym wierszu i p różnych, posortowanych indeksów kandydatów w drugim. Indeksy zaczynają się od 0 i odnoszą do kolejności kandydatów w pliku instancji.

## Parametry i porównania

- `--size 10000`: bok kwadratu, domyślnie 10 km.
- `--distribution uniform`: niezależne punkty równomiernie rozmieszczone w kwadracie.
- `--distribution clustered`: osiedla o różnej liczebności i rozproszeniu, z rozkładem normalnym uciętym do kwadratu przez odrzucanie punktów spoza obszaru.
- `--clusters 5 --sigma 500 --background 0.05`: liczba osiedli, bazowe odchylenie w metrach i prawdopodobieństwo punktu tła. Rzeczywisty odsetek tła może różnić się losowo od parametru.
- `--weights lognormal`: domyślne wagi `max(1, round(lognormal(3, 0.75)))`; `--weights unit` ustawia wszystkie wagi na 1. Wagi są niezależne od położenia, co pozwala oddzielnie badać wpływ wag i geometrii.
- `--seed 1`: seed instancji; `--start-seeds 1 2 3` pozwala wybrać seedy rozwiązań początkowych.

Kandydaci mają własny strumień losowania. Dla tego samego seeda, m i rozmiaru kwadratu są identyczni między rozkładami i zestawami wag. Zmiana p nie zmienia instancji. Start zależy wyłącznie od m, p i seeda startowego, więc jest wspólny dla porównań rozkładów oraz backendów. Zmiana typu wag zachowuje współrzędne mieszkańców.

Manifest zapisuje wszystkie parametry, wersję Python, identyfikator i sumę SHA-256 generatora oraz sumy plików wejściowych. Odtwarzając dane, użyj tego samego kodu, wersji Python i parametrów; nie zakładamy identyczności bajtowej między dowolnymi wersjami Python. Dane są deterministyczne w ustalonym środowisku i nie zawierają znaczników czasu.

Duże wygenerowane pliki w `data/generated/` są ignorowane przez Git. Na potrzeby badań zachowuj parametry z manifestu w wersjonowanej konfiguracji eksperymentu. Generator nie oblicza macierzy odległości ani nie rozwiązuje p-median.

## Jak czytać kod krok po kroku

Program zaczyna się od ostatniego bloku `if __name__ == "__main__"`. Wywołuje on `main()`, która najpierw odczytuje parametry przez `parse_args()`, a następnie przekazuje je do `write_dataset()`. Import pliku jako modułu udostępnia funkcje, ale nie tworzy danych.

| Funkcja | Co robi i dlaczego |
|---|---|
| `rng_for(seed, stream)` | Tworzy generator liczb pseudolosowych z seeda i nazwy strumienia. SHA-256 pozwala zamienić tę parę na stabilną liczbę, niezależną od zmiennego między sesjami `hash()` Pythona. |
| `generate(...)` | Losuje n punktów mieszkańców z wagami oraz m kandydatów na stacje. Zwraca również środki osiedli do manifestu. |
| `start_set(m, p, seed)` | Wybiera p różnych indeksów kandydatów, następnie je sortuje. To początek pracy solvera, a nie wynik optymalizacji. |
| `sha256(path)` | Wylicza sumę kontrolną bajtów pliku. Pomaga wykryć zmianę zawartości, ale sama nie dowodzi poprawności modelu. |
| `write_dataset(args)` | Generuje dane, tworzy nowy katalog i zapisuje instancję, wszystkie starty oraz manifest. |
| `parse_args(argv=None)` | Odczytuje parametry terminala, ustawia wartości domyślne i sprawdza poprawność argumentów. |
| `main(argv=None)` | Steruje przebiegiem, obsługuje błędy zapisu i wypisuje lokalizację utworzonych plików. |

W `generate()` występują trzy niezależne generatory: `positions` dla położeń mieszkańców, `candidates_rng` dla kandydatów i `weights_rng` dla wag. Dzięki temu zmiana rodzaju wag nie zmienia geometrii, a zmiana rozkładu mieszkańców nie zmienia kandydatów. Rozwiązania startowe mają dodatkowe generatory zależne od m, p i seeda startowego. Zmiana samego seeda instancji nie zmienia startowych indeksów dla tych samych m, p i seedów startów, choć miejsca wskazane przez indeksy mogą się zmienić.

W wariancie `uniform` współrzędne mieszkańców są losowane w całym kwadracie. W wariancie `clustered` program najpierw losuje środki osiedli, ich względną popularność (`populations`) i rozproszenie (`spreads`). Następnie dla każdego mieszkańca wybiera tło albo osiedle. W osiedlu współrzędne losowane są wokół jego środka z rozkładu normalnego. Wartość spoza kwadratu jest losowana ponownie, aby nie tworzyć sztucznych skupisk na krawędziach.

`populations` określa prawdopodobieństwo wyboru osiedla, nie dokładną liczbę punktów. `spreads` mieści się między 0.5 i 1.5 wartości `sigma`. Parametr `background` również jest prawdopodobieństwem, dlatego przy 1000 punktach i `background=0.05` nie musi powstać dokładnie 50 punktów tła. Przy `unit` każda waga to 1. Przy `lognormal` parametry 3 i 0.75 odnoszą się do rozkładu logarytmu wagi; po zaokrągleniu wynik jest ograniczany od dołu do 1.

Zapis współrzędnych używa `.17g`, czyli 17 cyfr znaczących, aby nie tracić dokładności typowych liczb `double` podczas odczytu. `sorted()` w startach porządkuje indeksy; nie ocenia jakości rozmieszczenia. Liczba startów wynosi `liczba wartości p * liczba seedów startowych`.

## Co program sprawdza przy uruchomieniu

Najpierw `argparse` kontroluje obecność wymaganej opcji `--out`, typy liczb oraz dozwolone wartości `--distribution` i `--weights`. Dalej `parse_args()` wykonuje następujące kontrole:

| Warunek | Powód |
|---|---|
| `n >= 1` i `m >= 1` | Instancja musi zawierać mieszkańców i kandydatów. |
| Dla każdego p: `1 <= p <= m` | Nie można wybrać zerowej liczby stacji ani więcej różnych stacji niż jest kandydatów. |
| Brak powtórzeń w p i seedach startowych | Powtórzenia prowadziłyby do tych samych nazw plików. |
| Seedy są nieujemne | To przyjęta konwencja generatora, ułatwiająca zapis konfiguracji i nazw plików. |
| `size` jest skończone i dodatnie | Kwadrat musi mieć określony, dodatni bok; `NaN` i nieskończoność nie opisują poprawnego obszaru. |
| `clusters >= 1` i skończone `0 < sigma <= size` | W scenariuszu osiedlowym potrzebny jest środek i dodatnie rozproszenie. Górny limit sigma jest decyzją projektową. |
| Skończone `0 <= background <= 1` | To prawdopodobieństwo, więc musi należeć do przedziału od 0 do 1. |
| Katalog `--out` nie istnieje | Chroni wcześniejsze dane oraz manifest przed nadpisaniem. |

Parametry osiedli są sprawdzane również przy rozkładzie `uniform`, chociaż nie wpływają wtedy na położenia. Dodatkowo samo tworzenie katalogu ma `exist_ok=False`, więc nie nadpisze katalogu utworzonego pomiędzy sprawdzeniem parametrów a zapisem.

Unikalność startowych indeksów zapewnia `random.sample()` (losowanie bez zwracania). Zakres indeksów zapewnia `range(m)`. Generator nie wczytuje ponownie wszystkich plików po zapisie w celu ich pełnej kontroli; opisane niżej sprawdzenia wykonano osobno podczas tworzenia programu.

## Manifest i błędy

`manifest.json` zawiera konfigurację, środki osiedli, jednostki, metrykę, wersję formatu, numerację indeksów od 0, wersję Python i sumy kontrolne plików. Lista `starts` łączy każdą nazwę pliku z p i seedem. Ścieżka wyjściowa nie jest częścią konfiguracji, aby można było odtworzyć dane w innym katalogu.

`generator_sha256` jest sumą całego `gen.py`, więc zmienia się także po dodaniu komentarzy. Istniejące manifesty pozostają zapisem użytej wcześniej wersji pliku; zmiana komentarzy nie zmienia wygenerowanych współrzędnych, wag ani startów przy tych samych parametrach i środowisku.

Błędne argumenty kończą program kodem 2. Błąd systemowy podczas zapisu (np. brak miejsca lub uprawnień) daje kod 1, a udany zapis kod 0. Po błędzie zapisu może pozostać częściowy katalog; generator nie usuwa go automatycznie. Przed ponowieniem wybierz nową ścieżkę lub świadomie usuń niepełne dane.

## Co sprawdzono podczas tworzenia generatora

Osobna kontrola wykonana przy pierwszej implementacji potwierdziła powtarzalność całych plików przy tych samych parametrach, zakres współrzędnych, dodatnie całkowite wagi, poprawność startów (także dla p=1 i p=m) oraz sumy kontrolne. Sprawdzono też wspólnych kandydatów i starty między rozkładami, zachowanie geometrii po zmianie wag oraz odrzucanie niepoprawnych argumentów i ochronę danych przed nadpisaniem. Szczegóły są w `docs/iterations/00-generator.md`.
