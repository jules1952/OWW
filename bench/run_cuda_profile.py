"""Mierzy fazy G1 w osobnej kompilacji, korzystając z referencji zapisanych przez E5."""
import argparse
import csv
import json
import math
import os
from pathlib import Path
from tempfile import TemporaryDirectory

from configs.record_build import checkout_record, executable_record, file_record, write_manifest
from run_omp_scaling import read_dataset
from run_seq_profile import check_solution, run_solver, write_rows


PHASES = {"cuda_device_query", "cuda_device_alloc", "host_distance_matrix", "kernel_and_sync"}


def reference_cases(baseline, validation_path):
    with validation_path.open(newline="", encoding="utf-8") as source:
        references = {row["start"]: row for row in csv.DictReader(source) if row["stage"] == "s0"}
    cases = baseline["parameters"]["scaling"]
    if isinstance(cases, dict):
        cases = [cases]
    for case in cases:
        folder = Path(case["directory"])
        dataset = read_dataset(folder)
        if dataset != baseline["datasets"][folder.as_posix()]:
            raise ValueError(f"Dane {folder} różnią się od danych serii bazowej.")
        start = folder / case["start"]
        reference = references[start.as_posix()]
        stations = tuple(map(int, reference["station_indices"].split()))
        signature = (reference["final_set_hash"], int(reference["iterations"]), stations)
        yield folder / dataset["instance"]["file"], start, (float(reference["cost"]), signature)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("build", "baseline", "out"):
        parser.add_argument(name, type=Path)
    args = parser.parse_args()
    executable = args.build / ("pm_g1.exe" if os.name == "nt" else "pm_g1")
    binary = executable_record(executable)
    if binary["configuration"] != "Release" or not binary["profiling"]:
        raise ValueError("Użyj kompilacji Release z PMEDIAN_PROFILE_CUDA=ON.")

    manifest_path = args.baseline / "manifest.json"
    validation_path = args.baseline / "validation.csv"
    baseline = json.loads(manifest_path.read_text(encoding="utf-8"))
    repetitions = baseline["parameters"]["repetitions"]

    manifest = {
        "checkout": checkout_record(("bench/run_omp_scaling.py", "bench/run_cuda_profile.py",
                                    "src/cuda/g1.cu", "src/cuda/g1.hpp", "src/cuda/g1_main.cpp")),
        "executable": binary,
        "baseline": {"manifest": file_record(manifest_path), "validation": file_record(validation_path)},
        "warmups": 1,
        "repetitions": repetitions,
    }
    args.out.mkdir(parents=True)
    runs = []
    phases = []
    with TemporaryDirectory(dir=args.out) as temporary:
        for instance, start, expected in reference_cases(baseline, validation_path):
            expected_calls = dict.fromkeys(PHASES, 1)
            # Każde uruchomienie startuje nowy proces, również podczas rozgrzewki.
            for rep in range(repetitions + 1):
                path = Path(temporary) / f"{len(runs)}.csv"
                row, result, stderr = run_solver(executable, instance, start, path, rep)
                check_solution(expected, (float(row["cost"]), result), start)
                expected_calls["kernel_and_sync"] = int(row["iterations"]) + 1
                entries = list(csv.reader(stderr.splitlines()))
                if len(entries) != len(PHASES) or {name: int(calls) for name, calls, _ in entries} != expected_calls:
                    raise ValueError(f"Niepełny profil faz G1 dla {start}.")
                if any(not math.isfinite(float(value)) or float(value) < 0 for _, _, value in entries):
                    raise ValueError(f"Nieprawidłowy czas fazy G1 dla {start}.")
                for name, calls, seconds in entries:
                    phases.append({"start": start.as_posix(), "rep": rep, "phase": name,
                                   "calls": calls, "seconds": seconds})
                runs.append({"start": start.as_posix(), **row})

    write_rows(args.out / "profile-runs.csv", runs)
    write_rows(args.out / "phases.csv", phases)
    write_manifest(args.out / "manifest.json", manifest)


if __name__ == "__main__":
    main()
