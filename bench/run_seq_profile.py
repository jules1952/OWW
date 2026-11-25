"""Mierzy pracę solverów i pełne uruchomienie oraz zapisuje osobny profil faz."""
import argparse
import csv
import os
from pathlib import Path
import platform
import statistics
import subprocess
from time import perf_counter

from configs.record_build import build_manifest, file_record, write_manifest

REPETITIONS = 5
METRICS = ("t_search", "t_total")


def run_solver(executable, instance, start, path, repetition):
    command = [str(executable.resolve()), str(instance), str(start), str(path)]
    begin = perf_counter()
    run = subprocess.run(command, capture_output=True, text=True)
    total = perf_counter() - begin
    if run.returncode:
        raise RuntimeError(f"{executable} zakończył się kodem {run.returncode}: {run.stderr.strip()}")

    with path.open(newline="", encoding="utf-8") as result:
        reader = csv.DictReader(result)
        row, = reader
        fields = reader.fieldnames

    # Zegar obejmuje proces C++, łącznie z jego zapisem plików. Uzupełnienie CSV
    # przez Pythona odbywa się po pomiarze, tak samo jak odczyt wyników do walidacji.
    row["rep"] = "" if repetition == "profile" else str(repetition)
    row["t_total"] = format(total, ".17g")
    updated = path.with_suffix(".tmp")
    with updated.open("x", newline="", encoding="utf-8") as result:
        writer = csv.DictWriter(result, fieldnames=fields, lineterminator="\n")
        writer.writeheader()
        writer.writerow(row)
    updated.replace(path)

    stations = path.with_suffix(".csv.stations.txt").read_text().split()
    signature = (row["final_set_hash"], int(row["iterations"]), tuple(map(int, stations[1:])))
    return row, signature, run.stderr


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
    write_manifest(args.out / "manifest.json", manifest)
    timings = {stage: {metric: [] for metric in METRICS} for stage in release}
    reference = None
    with (args.out / "phases.csv").open("x", newline="", encoding="utf-8") as output:
        writer = csv.writer(output, lineterminator="\n")
        writer.writerow(["stage", "phase", "calls", "seconds"])
        for repetition in [*range(REPETITIONS + 1), "profile"]:
            stages = ("s0", "s1")
            if repetition != "profile" and repetition % 2:
                stages = ("s1", "s0")
            for stage in stages:
                profiling = repetition == "profile"
                executable = (profile if profiling else release)[stage]
                path = args.out / f"{stage}-{repetition}.csv"
                row, signature, stderr = run_solver(executable, args.instance, args.start, path, repetition)
                cost = float(row["cost"])
                if reference is None:
                    reference = (cost, signature)
                reference_cost, reference_signature = reference
                if signature != reference_signature or abs(cost - reference_cost) > 1e-6 * abs(reference_cost):
                    raise ValueError(f"Rozbieżność wyniku w {path}")
                if profiling:
                    phases = list(csv.reader(stderr.splitlines()))
                    if not phases or any(len(phase) != 3 for phase in phases):
                        raise ValueError(f"Niepoprawny profil faz z {executable}")
                    writer.writerows([stage, *phase] for phase in phases)
                elif repetition:
                    for metric in METRICS:
                        timings[stage][metric].append(float(row[metric]))

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
