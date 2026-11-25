# Środowisko kompilacji i pomiarów

Pomiary S0, S1 i P1 wykonano natywnie w Windows, w konfiguracji Release x64. Do budowania użyto MSVC, CMake i Ninja. Skrypty pomiarowe i generator działają z biblioteką standardową Pythona. Dokładne wersje narzędzi, sprzęt i warunki pomiaru są zapisane przy wynikach. [Pomiary sekwencyjne](iterations/02-seq-end-to-end.md) · [Pomiary OpenMP](iterations/03-p1-openmp.md)

## Windows

W terminalu deweloperskim Visual Studio dla x64 uruchom `cl`, `cmake --version`, `ninja --version` i `python --version`, aby sprawdzić narzędzia. Konfiguracja z `PMEDIAN_ENABLE_OPENMP=ON` wymaga obsługi OpenMP przez kompilator C++. Polecenia konfiguracji, kompilacji i CTest są w [README](../README.md).

## CUDA

Na komputerze wykryto RTX 3070 z 8 GB pamięci. Przed implementacją G1 trzeba sprawdzić sterownik i kompilator CUDA poleceniami `nvidia-smi` i `nvcc --version`, a następnie zbudować i uruchomić mały program na GPU. Pomiary wersji CUDA będzie można zestawić z S1 i P1 po uruchomieniu ich na tej samej maszynie w porównywalnym środowisku.

WSL2 pozostaje alternatywnym środowiskiem. Przy jego wyborze konfigurację sterownika i narzędzi warto oprzeć na instrukcjach dostawców. [Instalacja WSL](https://learn.microsoft.com/en-us/windows/wsl/install) · [CUDA w WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html)
