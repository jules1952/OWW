# Etapy pracy i wyniki

| Etap | Co opisuje notatka |
|---|---|
| [Generator danych](00-generator.md) | Losowanie instancji, zestawy początkowe i testy powtarzalności. |
| [Parser](01-parser.md) | Format wejścia, reguły walidacji i testy odczytu. |
| [S0 i S1](S0-S1.md) | Pierwsze pomiary, wąskie gardło S0 i sposób zmniejszenia liczby obliczeń. |
| [Pomiar całego uruchomienia](02-seq-end-to-end.md) | Osobne czasy odczytu, wyszukiwania i całego procesu dla S0 i S1. |
| [OpenMP P1](03-p1-openmp.md) | Podział pracy między wątki, zgodność z S0 i wyniki skalowania. |
| [CUDA G1](04-g1-cuda.md) | Pierwszy wariant GPU, zgodność z S0 oraz pomiar przygotowania, wyszukiwania i transferów. |
| [Opłacalność CUDA w E5](05-e5-cuda.md) | Porównanie pełnych czasów S1, P1 i G1 na sześciu instancjach i profil faz CUDA. |
