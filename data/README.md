# Dane wejściowe

Generator przygotowuje sztuczne rozmieszczenie punktów popytu i kandydatów na stacje. Z katalogu projektu można uruchomić go tak.

```powershell
python data/gen.py --n 1000 --m 100 --p 5 10 20 --distribution uniform --seed 1 --out data/generated/uniform_demo
python data/gen.py --n 1000 --m 100 --p 5 10 20 --distribution clustered --seed 1 --out data/generated/clustered_demo
```

W każdym katalogu powstaje `instance.txt`, pliki `start_p<P>_s<SEED>.txt` i `manifest.json`. Domyślnie zapisuje się 10 zestawów początkowych na każdą wartość `p`, dla ziaren od 1 do 10. Generator wymaga nowego katalogu `--out`, żeby zachować dane z wcześniejszych uruchomień.

## Format plików

W `instance.txt` pierwszy wiersz zawiera `n m`, następne `n` wierszy zawiera położenia i wagi punktów popytu (`x y w`), a ostatnie `m` wierszy położenia kandydatów (`x y`). Współrzędne są podane w metrach, z kropką dziesiętną. Wagi są dodatnimi liczbami całkowitymi.

Przykład dwóch punktów popytu i trzech kandydatów:

```text
2 3
0 0 2
4 0 1
0 0
2 0
4 0
```

Plik startowy zawiera liczbę stacji `p` w pierwszym wierszu i dokładnie `p` różnych, rosnących indeksów w drugim. Indeksy zaczynają się od zera i odpowiadają kolejności kandydatów w instancji. W powyższym przykładzie start dla dwóch stacji może mieć postać:

```text
2
0 2
```

## Jak powstają dane

- `uniform` losuje punkty popytu równomiernie w kwadracie o boku `--size` (domyślnie 10 000 m).
- `clustered` losuje punkty wokół środków osiedli. `--clusters` ustawia ich liczbę, `--sigma` bazowe rozproszenie w metrach, a `--background` prawdopodobieństwo wylosowania punktu z równomiernego tła. Współrzędną spoza kwadratu losuje się ponownie.
- `--weights unit` ustawia wszystkie wagi na 1. Domyślne `lognormal` losuje wagę niezależnie od położenia, zaokrągla ją i stosuje minimum 1. Parametry 3 i 0,75 opisują rozkład logarytmu wagi.

Współrzędne punktów popytu, położenia kandydatów i wagi mają osobne strumienie losowania. Przy tym samym ziarnie instancji, `m` i rozmiarze kwadratu zmiana rozkładu punktów popytu pozostawia tych samych kandydatów. Zmiana rodzaju wag pozostawia te same współrzędne. Zestaw początkowy zależy od `m`, `p` i ziarna podanego w `--start-seeds`.

## Jak odtworzyć dane

Parametry `n` i `m` muszą być dodatnie, każda wartość `p` musi należeć do przedziału od 1 do `m`, a ziarna losowania muszą być nieujemne. Powtórzone wartości `p` lub ziarna startowe prowadziłyby do tych samych nazw plików, dlatego generator je odrzuca. Sprawdza też dodatni bok kwadratu, co najmniej jedno osiedle, `0 < sigma <= size` oraz `0 <= background <= 1`.

Manifest zapisuje parametry, wersję Pythona, sumę SHA-256 generatora oraz sumy plików instancji i startów. Do odtworzenia użyj zapisanych parametrów i porównaj sumy plików z manifestem. Zmiana komentarza w generatorze zmienia sumę jego kodu. Sumy wygenerowanych plików pozwalają wtedy osobno sprawdzić, czy dane pozostały takie same. Katalog `data/generated/` jest ignorowany przez Git. [Kontrole generatora](../docs/iterations/00-generator.md)
