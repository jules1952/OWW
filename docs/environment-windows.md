# Konfiguracja Windows dla projektu p-median

Ta konfiguracja używa WSL2, Ubuntu 24.04 LTS, C++17, OpenMP, Python i CUDA. Wykryty komputer ma kartę NVIDIA GeForce RTX 3070 z 8 GB pamięci oraz sterownik Windows 621.01. CUDA Toolkit instalujemy w Ubuntu/WSL, a sterownik NVIDIA zostaje po stronie Windows.

## 1. Przygotuj Windows

1. Zainstaluj aktualizacje Windows i uruchom ponownie komputer.
2. Sprawdź w Menedżerze zadań, w sekcji **Wydajność → CPU**, czy pozycja **Wirtualizacja** ma wartość **Włączona**. Jeśli jest wyłączona, włącz Intel VT-x lub AMD SVM w BIOS/UEFI.
3. Otwórz **Terminal lub PowerShell jako administrator** i wykonaj:

```powershell
wsl --install -d Ubuntu-24.04
```

Jeśli Windows nie rozpoznaje nazwy dystrybucji, sprawdź `wsl --list --online` i wybierz Ubuntu 24.04 z listy. Jeśli WSL prosi o restart, uruchom ponownie komputer.

4. Po restarcie uruchom Ubuntu z menu Start. Utwórz nazwę użytkownika i hasło Linux; podczas wpisywania hasła terminal nie pokazuje znaków.
5. W PowerShell sprawdź i zaktualizuj WSL:

```powershell
wsl --update
wsl --set-default-version 2
wsl --list --verbose
```

Dystrybucja Ubuntu powinna mieć wersję `2`. Gdy pokazuje `1`, wykonaj `wsl --set-version Ubuntu-24.04 2`.

Kontrola dostępna w tym środowisku nie pokazała gotowej dystrybucji WSL: `wsl --list --verbose` zwróciło stronę pomocy zamiast listy. Jeśli `wsl --install` również wyświetli tylko pomoc, zaktualizuj Windows i WSL przez Windows Update/Microsoft Store, uruchom ponownie komputer i spróbuj ponownie. Samo polecenie `wsl --install` wymaga Windows 10 w wersji 2004 (kompilacja 19041) lub nowszej. Dla CUDA w WSL NVIDIA wymaga Windows 10 21H2 lub nowszego; zaleca Windows 11, ponieważ standardowe wsparcie Windows 10 zakończyło się w 2025 roku.

## 2. Zostaw sterownik NVIDIA w Windows

Windows wykrywa RTX 3070 i sterownik 621.01. Nie instaluj sterownika Linux w Ubuntu. WSL przekazuje do Linuxa sterownik z hosta Windows; dodatkowy `nvidia-driver`, `cuda-drivers` albo `nvidia-open` w Ubuntu może zepsuć CUDA. Nie instaluj też Windowsowej wersji CUDA Toolkit — kompilator `nvcc` będzie działał w Ubuntu.

Jeżeli po konfiguracji Ubuntu komenda `nvidia-smi` nie widzi GPU, najpierw zaktualizuj sterownik NVIDIA dla Windows, a potem wykonaj `wsl --update` i restart. RTX 3070 jest wystarczającą kartą do wariantu CUDA; przy 8 GB VRAM trzeba jednak dobierać rozmiary danych tak, aby zostawić miejsce na bufory i redukcje.

## 3. Zainstaluj narzędzia w Ubuntu

Otwórz Ubuntu i wykonaj:

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install -y build-essential cmake ninja-build git pkg-config gdb python3 python3-venv python3-pip wget
```

Pakiety dostarczą `g++`, `make`, narzędzia C++17, Git i Pythona. OpenMP dla GCC będzie dostępne przez `g++ -fopenmp`.

Zainstaluj CUDA Toolkit dla Ubuntu 24.04:

```bash
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2404/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
sudo apt update
sudo apt install -y cuda-toolkit
```

Dodaj kompilator CUDA do `PATH`:

```bash
echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}' >> ~/.bashrc
source ~/.bashrc
```

NVIDIA obecnie zaleca w WSL standardowe pakiety Linux i meta-pakiet `cuda-toolkit`. Ubuntu 24.04 jest wspierane przez CUDA 13.5. Nie wybieraj `cuda-drivers` ani pakietów sterownika Linux.

## 4. Zainstaluj biblioteki Python

Utwórz środowisko poza repozytorium, aby nie zapisywać dużego `venv` na dysku Windows:

```bash
mkdir -p ~/venvs
python3 -m venv ~/venvs/pmedian
source ~/venvs/pmedian/bin/activate
python -m pip install --upgrade pip
python -m pip install numpy pandas matplotlib jupyterlab pyyaml
```

Aktywuj je w kolejnych sesjach przez `source ~/venvs/pmedian/bin/activate`. Jeśli projekt później dostanie `requirements.txt`, użyj wersji zależności zapisanych w tym pliku.

## 5. Dostęp do repozytorium

Repozytorium jest obecnie w Windows pod `C:\Users\Konrad\Desktop\Rowery`. Z Ubuntu otworzysz je tak:

```bash
cd /mnt/c/Users/Konrad/Desktop/Rowery
```

To najprostsze na początek i pozwala Codexowi oraz Ubuntu używać tych samych plików. Microsoft wskazuje jednak, że Linuxowe kompilacje i intensywne operacje na plikach są szybsze w systemie plików WSL niż pod `/mnt/c`. Przed dużymi, końcowymi pomiarami przenieś lub sklonuj repozytorium do katalogu Linux, na przykład `~/projects/Rowery`, i otwieraj tę kopię w edytorze obsługującym WSL. Utrzymuj jedną aktywną kopię kodu podczas pomiarów.

## 6. Sprawdź instalację

W Ubuntu uruchom:

```bash
nvidia-smi
nvcc --version
g++ --version
python3 --version
```

`nvidia-smi` powinien pokazać RTX 3070, `nvcc` wersję CUDA Toolkit, a `g++` kompilator Linux. Samo `nvidia-smi` nie potwierdza jeszcze poprawnej kompilacji projektu CUDA; do tego potrzebny będzie mały program próbny lub docelowy wariant `pm` po jego implementacji.

## 7. Ustawienia do pomiarów

- Na czas badań podłącz laptop/komputer do zasilania, w Windows ustaw **Ustawienia → System → Zasilanie → Tryb zasilania: Najlepsza wydajność** i zamknij inne intensywne zadania CPU/GPU.
- Na start pozostaw domyślne przydziały pamięci WSL. Nie znamy całkowitej ilości RAM tego komputera i nie ma potrzeby ustawiać limitów w `.wslconfig` przed pierwszymi pomiarami.
- Przy pomiarach OpenMP ustaw w powłoce Ubuntu `OMP_PROC_BIND=close` i `OMP_PLACES=cores`; zapisuj liczbę wątków.
- Porównuj `seq`, `omp` i `cuda` na tej samej maszynie. Dla CUDA raportuj także transfery H2D/D2H oraz czas całkowity.

## Linki do dokumentacji

- [Instalacja WSL](https://learn.microsoft.com/en-us/windows/wsl/install)
- [Środowisko programistyczne WSL i położenie plików](https://learn.microsoft.com/en-us/windows/wsl/setup/environment)
- [CUDA w WSL — instrukcja NVIDIA](https://docs.nvidia.com/cuda/wsl-user-guide/index.html)
- [Instalacja CUDA Toolkit 13.5 dla Linux](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html)
