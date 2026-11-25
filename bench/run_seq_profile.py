"""Mierzy pracę solverów i pełne uruchomienie oraz zapisuje osobny profil faz."""
import argparse
import csv
import os
from pathlib import Path
import platform
import statistics
import subprocess
from tempfile import TemporaryDirectory
from time import perf_counter

from configs.record_build import build_manifest, file_record, write_manifest

REPETITIONS = 5
METRICS = ("t_search", "t_total")


def run_solver(executable, instance, start, path, repetition, *, solver_args=(), env=None):
    command = [str(executable.resolve()), str(instance), str(start), str(path), *solver_args]
    begin = perf_counter()
    run = subprocess.run(command, capture_output=True, text=True, env=env)
    total = perf_counter() - begin
    if run.returncode:
        raise RuntimeError(f"{executable} zakończył się kodem {run.returncode}: {run.stderr.strip()}")

    with path.open(newline="", encoding="utf-8") as result:
        row, = csv.DictReader(result)

    # Zegar obejmuje proces C++, łącznie z jego zapisem plików. Uzupełnienie CSV
    # przez Pythona odbywa się po pomiarze, tak samo jak odczyt wyników do walidacji.
    row["rep"] = "" if repetition == "profile" else str(repetition)
    row["t_total"] = format(total, ".17g")
    with path.with_suffix(".csv.stations.txt").open(encoding="utf-8") as result:
        size, *stations = map(int, result.read().split())
    if size != len(stations) or size != int(row["p"]):
        raise ValueError(f"Niezgodna liczba stacji w {path}")
    row["station_indices"] = " ".join(map(str, stations))
    signature = (row["final_set_hash"], int(row["iterations"]), tuple(stations))
    return row, signature, run.stderr


def check_solution(reference, result, context):
    reference_cost, reference_signature = reference
    cost, signature = result
    if signature != reference_signature or abs(cost - reference_cost) > 1e-6 * abs(reference_cost):
        raise ValueError(f"Rozbieżność wyniku w {context}")


def write_rows(path, rows):
    with path.open("x", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("release", "profile", "instance", "start", "out"):
        parser.add_argument(name, type=Path)
    args = parser.parse_args()
    suffix = ".exe" if os.name == "nt" else ""
    release, profile = (
        {stage: folder / f"pm_{stage}{suffix}" for stage in ("s0", "s1")}
        for folder in (args.release, args.profile)
    )
    manifest = build_manifest(release, profile)
    manifest.update({
        "instance": file_record(args.instance),
        "start": file_record(args.start),
        "warmups_per_stage": 1,
        "measured_repetitions_per_stage": REPETITIONS,
        "profile_runs_per_stage": 1,
        "timing": {
            "t_prep": "C++ instance and start file parsing",
            "t_search": "C++ solver call",
            "t_total": "Python wall clock around the C++ process including file and console I/O",
        },
        "host": {
            "system": platform.platform(),
            "processor": platform.processor(),
            "logical_cpus": os.cpu_count(),
            "python": platform.python_version(),
        },
    })
    args.out.mkdir(parents=True)
    measurements = []
    warmups = []
    profile_runs = []
    solver_metadata = {}
    timings = {stage: {metric: [] for metric in METRICS} for stage in release}
    reference = None
    phases = []
    # Solver nadal zapisuje własne pliki. Benchmark po walidacji publikuje tylko tabele serii.
    with TemporaryDirectory(dir=args.out) as temporary:
        for repetition in [*range(REPETITIONS + 1), "profile"]:
            stages = ("s0", "s1")
            if repetition != "profile" and repetition % 2:
                stages = ("s1", "s0")
            for stage in stages:
                profiling = repetition == "profile"
                executable = (profile if profiling else release)[stage]
                path = Path(temporary) / f"{stage}-{repetition}.csv"
                row, signature, stderr = run_solver(executable, args.instance, args.start, path, repetition)
                cost = float(row["cost"])
                if reference is None:
                    reference = (cost, signature)
                check_solution(reference, (cost, signature), path.name)
                details = path.with_suffix(".csv.meta.txt").read_bytes().decode("utf-8")
                if solver_metadata.setdefault(stage, details) != details:
                    raise ValueError(f"Niespójne metadane solvera w {path.name}")
                row = {"stage": stage, "start": args.start.as_posix(), **row}
                if profiling:
                    profile_rows = list(csv.reader(stderr.splitlines()))
                    if not profile_rows or any(len(phase) != 3 for phase in profile_rows):
                        raise ValueError(f"Niepoprawny profil faz z {executable}")
                    phases.extend({"stage": stage, "phase": name, "calls": calls, "seconds": seconds}
                                  for name, calls, seconds in profile_rows)
                    profile_runs.append(row)
                elif repetition:
                    measurements.append(row)
                    for metric in METRICS:
                        timings[stage][metric].append(float(row[metric]))
                else:
                    warmups.append(row)

    write_rows(args.out / "measurements.csv", measurements)
    write_rows(args.out / "warmup.csv", warmups)
    write_rows(args.out / "profile-runs.csv", profile_runs)
    write_rows(args.out / "phases.csv", phases)
    manifest["solver_metadata"] = solver_metadata
    write_manifest(args.out / "manifest.json", manifest)

    for metric in METRICS:
        medians = {}
        for stage, measurements in timings.items():
            times = measurements[metric]
            q1, _, q3 = statistics.quantiles(times, method="inclusive")
            medians[stage] = statistics.median(times)
            print(f"{metric} {stage}: mediana {medians[stage]:.6f} s, IQR {q3 - q1:.6f} s")
        print(f"{metric} speedup: {medians['s0'] / medians['s1']:.2f}x")
    print("wynik", reference)


if __name__ == "__main__":
    main()
