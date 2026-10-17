# Równoległa optymalizacja rozmieszczenia stacji rowerowych

Projekt dotyczy ważonego problemu p-median: wyboru określonej liczby
stacji spośród dopuszczalnych lokalizacji tak, aby minimalizować
sumę odległości mieszkańców do najbliższej stacji, z uwzględnieniem
liczby mieszkańców.

## Metoda

Algorytm Vertex Substitution poprawia rozmieszczenie przez zamianę
jednej wybranej lokalizacji na niewybraną. Wykonuje najlepszą
poprawiającą zamianę i powtarza proces do braku poprawy.
Metoda nie gwarantuje globalnego optimum.

## Planowane warianty

- Jeden rdzeń CPU.
- Wiele rdzeni CPU.
- GPU.
- Jednoczesna praca wielu rdzeni CPU i GPU.

## Dane i badania

Wykorzystamy generowane współrzędne punktów zamieszkania,
wagi oznaczające liczbę mieszkańców i możliwe miejsca stacji.
Odległości będą liczone w linii prostej.

Na podstawie profilowania będziemy optymalizować implementację
oraz porównywać poprawność wyników, czas wykonania, zużycie pamięci
i skalowanie dla różnych rozmiarów danych.