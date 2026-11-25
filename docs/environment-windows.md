# Środowisko kompilacji i pomiarów

Pomiary S0, S1 i P1 wykonano natywnie w Windows, w konfiguracji Release x64. Do budowania użyto MSVC, CMake i Ninja. Skrypty pomiarowe i generator działają z biblioteką standardową Pythona. Dokładne wersje narzędzi, sprzęt i warunki pomiaru są zapisane przy wynikach. [Pomiary sekwencyjne](iterations/02-seq-end-to-end.md) · [Pomiary OpenMP](iterations/03-p1-openmp.md)

## Windows

W terminalu deweloperskim Visual Studio dla x64 uruchom `cl`, `cmake --version`, `ninja --version` i `python --version`, aby sprawdzić narzędzia. Konfiguracja z `PMEDIAN_ENABLE_OPENMP=ON` wymaga obsługi OpenMP przez kompilator C++. Polecenia konfiguracji, kompilacji i CTest są w [README](../README.md).

## CUDA

G1 zbudowano natywnie w Windows przy użyciu MSVC 19.52.37029.0 i CUDA Toolkit 13.5.37. Testy wykonano na GeForce RTX 3070 z 8 GiB pamięci, ze sterownikiem 621.01. Wersje lokalnego sterownika i kompilatora można sprawdzić poleceniami `nvidia-smi` i `nvcc --version`. `pm_g1 --build-info` podaje użyty kompilator CUDA i architekturę. [Polecenia kompilacji](../README.md#cuda)

WSL2 pozostaje alternatywnym środowiskiem. Przy jego wyborze konfigurację sterownika i narzędzi warto oprzeć na instrukcjach dostawców. [Instalacja WSL](https://learn.microsoft.com/en-us/windows/wsl/install) · [CUDA w WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html)
