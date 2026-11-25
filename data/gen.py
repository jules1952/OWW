"""Generuje dane ważonego p-median oraz startowe zestawy stacji.

Kolejność pracy: odczyt i sprawdzenie parametrów -> losowanie danych ->
zapis instancji i startów -> zapis manifestu opisującego pochodzenie plików.
Generator korzysta wyłącznie z biblioteki standardowej Python.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path
import platform
import random
import sys


def rng_for(seed, stream):
    """Tworzy osobny generator losowy dla seeda i nazwy strumienia."""
    # SHA-256 daje to samo ziarno strumienia w kolejnych uruchomieniach Pythona.
    digest = hashlib.sha256(f"pmedian-v1:{seed}:{stream}".encode()).digest()
    return random.Random(int.from_bytes(digest, "big"))


def generate(n, m, size, distribution, weights, clusters, sigma, background, seed):
    """Zwraca mieszkańców (x, y, w), kandydatów (x, y) i środki osiedli."""
    # Rozdzielamy losowania: zmiana wag nie przesuwa punktów ani kandydatów.
    positions = rng_for(seed, "positions")
    candidates_rng = rng_for(seed, "candidates")
    weights_rng = rng_for(seed, "weights")
    centers = []
    if distribution == "clustered":
        centers = [(positions.uniform(0, size), positions.uniform(0, size))
                   for _ in range(clusters)]
        # Większa wartość populations zwiększa szansę wyboru osiedla.
        # spreads podaje odchylenie standardowe położenia w metrach.
        populations = [positions.uniform(0.5, 2) for _ in centers]
        spreads = [sigma * positions.uniform(0.5, 1.5) for _ in centers]

    demand = []
    for _ in range(n):
        # Każdy punkt może niezależnie trafić do równomiernego tła.
        if distribution == "uniform" or positions.random() < background:
            x, y = positions.uniform(0, size), positions.uniform(0, size)
        else:
            k = positions.choices(range(clusters), weights=populations, k=1)[0]
            # Ponowne losowanie wartości spoza kwadratu zapobiega skupiskom przy krawędziach.
            cx, cy = centers[k]
            x = positions.gauss(cx, spreads[k])
            while not 0 <= x <= size:
                x = positions.gauss(cx, spreads[k])
            y = positions.gauss(cy, spreads[k])
            while not 0 <= y <= size:
                y = positions.gauss(cy, spreads[k])
        # Waga reprezentuje liczbę mieszkańców, dlatego jest całkowita i wynosi co najmniej 1.
        w = 1 if weights == "unit" else max(1, round(weights_rng.lognormvariate(3, 0.75)))
        demand.append((x, y, w))

    # Tych samych kandydatów można użyć przy różnych rozkładach popytu.
    candidates = [(candidates_rng.uniform(0, size), candidates_rng.uniform(0, size))
                  for _ in range(m)]
    return demand, candidates, centers


def start_set(m, p, seed):
    """Losuje p różnych indeksów stacji spośród m kandydatów."""
    # Losujemy bez zwracania, a indeksy sortujemy zgodnie z formatem wejścia.
    return sorted(rng_for(seed, f"start:{m}:{p}").sample(range(m), p))


def sha256(path):
    """Zwraca sumę kontrolną zawartości pliku, zapisaną szesnastkowo."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_dataset(args):
    """Tworzy nowy katalog z instancją, startami i manifestem; zwraca manifest."""
    demand, candidates, centers = generate(
        args.n, args.m, args.size, args.distribution, args.weights,
        args.clusters, args.sigma, args.background, args.seed)
    # exist_ok=False chroni poprzednie dane także wtedy, gdy katalog powstał
    # już po wcześniejszym sprawdzeniu w parse_args().
    args.out.mkdir(parents=True, exist_ok=False)
    instance = args.out / "instance.txt"
    with instance.open("w", encoding="utf-8", newline="\n") as stream:
        # 17 cyfr znaczących pozwala odczytać te same współrzędne w double.
        stream.write(f"{args.n} {args.m}\n")
        for x, y, w in demand:
            stream.write(f"{x:.17g} {y:.17g} {w}\n")
        for x, y in candidates:
            stream.write(f"{x:.17g} {y:.17g}\n")

    starts = []
    for p in sorted(args.p):
        for seed in args.start_seeds:
            path = args.out / f"start_p{p}_s{seed}.txt"
            selected = start_set(args.m, p, seed)
            path.write_text(f"{p}\n" + " ".join(map(str, selected)) + "\n", encoding="utf-8")
            starts.append({"p": p, "seed": seed, "file": path.name, "sha256": sha256(path)})

    # Odtworzenie danych w innym katalogu ma dać tę samą konfigurację w manifeście.
    config = {key: value for key, value in vars(args).items() if key != "out"}
    manifest = {
        "schema_version": 1,
        "generator": "pmedian-stdlib-v1",
        # Nawet zmiana komentarza w gen.py zmienia tę sumę kontrolną.
        "generator_sha256": sha256(Path(__file__)),
        "python_version": platform.python_version(),
        "units": "metres",
        "distance": "euclidean",
        "index_base": 0,
        "config": config,
        "cluster_centers": centers,
        "instance": {"file": instance.name, "sha256": sha256(instance)},
        "starts": starts,
    }
    (args.out / "manifest.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return manifest


def parse_args(argv=None):
    """Odczytuje opcje terminala i odrzuca parametry, z których nie powstaną poprawne dane."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", type=int, default=1000, help="Number of demand points")
    parser.add_argument("--m", type=int, default=100, help="Number of candidate stations")
    parser.add_argument("--p", type=int, nargs="+", default=[10], help="Station counts to generate starts for")
    parser.add_argument("--distribution", choices=["uniform", "clustered"], default="uniform")
    parser.add_argument("--weights", choices=["unit", "lognormal"], default="lognormal")
    parser.add_argument("--size", type=float, default=10000, help="Square side length in metres")
    parser.add_argument("--clusters", type=int, default=5)
    parser.add_argument("--sigma", type=float, default=500, help="Base cluster spread in metres")
    parser.add_argument("--background", type=float, default=0.05, help="Uniform background probability")
    parser.add_argument("--seed", type=int, default=1, help="Instance seed")
    parser.add_argument("--start-seeds", type=int, nargs="+", default=list(range(1, 11)))
    parser.add_argument("--out", type=Path, required=True, help="New output directory (must not exist)")
    args = parser.parse_args(argv)
    if args.n < 1 or args.m < 1:
        parser.error("n and m must be positive")
    if any(p < 1 or p > args.m for p in args.p):
        parser.error("each p must satisfy 1 <= p <= m")
    if len(set(args.p)) != len(args.p) or len(set(args.start_seeds)) != len(args.start_seeds):
        # Powtórzona wartość utworzyłaby ponownie tę samą nazwę pliku startu.
        parser.error("p values and start seeds must not repeat")
    if args.seed < 0 or any(s < 0 for s in args.start_seeds):
        parser.error("seeds must be nonnegative")
    if not math.isfinite(args.size) or args.size <= 0:
        # Sam warunek > 0 nie wystarcza: odrzucamy też NaN i nieskończoność.
        parser.error("size must be finite and positive")
    if args.clusters < 1 or not math.isfinite(args.sigma) or args.sigma <= 0 or args.sigma > args.size:
        # sigma <= size jest ograniczeniem przyjętym dla naszych scenariuszy.
        # Parametry osiedli sprawdzamy także wtedy, gdy wybrano uniform.
        parser.error("clusters must be positive and sigma must satisfy 0 < sigma <= size")
    if not math.isfinite(args.background) or not 0 <= args.background <= 1:
        parser.error("background must be between 0 and 1")
    if args.out.exists():
        parser.error("output directory already exists; choose a new --out")
    return args


def main(argv=None):
    """Łączy odczyt parametrów z zapisem; zwraca kod zakończenia programu."""
    args = parse_args(argv)
    try:
        manifest = write_dataset(args)
    except OSError as error:
        # Po błędzie zapisu część plików może już istnieć.
        print(f"Cannot write dataset: {error}", file=sys.stderr)
        return 1
    print(f"Created {args.out / 'instance.txt'} and {len(manifest['starts'])} start sets.")
    print(f"Parameters and SHA-256 checksums: {args.out / 'manifest.json'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
