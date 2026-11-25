"""Sprawdza P1 względem S0 i porównuje skalowanie OpenMP z S1."""
import argparse
import json
import os
from pathlib import Path
import platform
import statistics
from tempfile import TemporaryDirectory

from configs.record_build import checkout_record, executable_record, file_record, write_manifest
from run_seq_profile import check_solution, run_solver, write_rows


def read_dataset(folder):
    with (folder / "manifest.json").open(encoding="utf-8") as manifest:
        dataset = json.load(manifest)
    for entry in (dataset["instance"], *dataset["starts"]):
        path = folder / entry["file"]
        if file_record(path)["sha256"] != entry["sha256"]:
            raise ValueError(f"Niezgodna suma kontrolna {path}")
    return dataset


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("build", "config", "out"):
        parser.add_argument(name, type=Path)
    args = parser.parse_args()
    with args.config.open(encoding="utf-8") as source:
        config = json.load(source)
    counts = config["threads"]
    if not counts or any(type(count) is not int or count < 1 for count in counts) or len(set(counts)) != len(counts):
        raise ValueError("threads musi zawierać różne dodatnie liczby całkowite.")
    reps = config["repetitions"]
    if type(reps) is not int or reps < 2:
        raise ValueError("repetitions musi być liczbą całkowitą co najmniej 2.")

    suffix = ".exe" if os.name == "nt" else ""
    executables = {stage: args.build / f"pm_{stage}{suffix}" for stage in ("s0", "s1", "p1")}
    binaries = {stage: executable_record(path) for stage, path in executables.items()}
    if any(info["configuration"] != "Release" or info["profiling"] for info in binaries.values()):
        raise ValueError("Użyj kompilacji Release z wyłączonym profilowaniem.")
    env = {**os.environ, "OMP_DYNAMIC": "FALSE"}
    scaling = Path(config["scaling"]["directory"])
    validation = [Path(path) for path in config["validation"]]
    datasets = {folder: read_dataset(folder) for folder in (scaling, *validation)}
    scaling_start = config["scaling"]["start"]
    if scaling_start not in {entry["file"] for entry in datasets[scaling]["starts"]}:
        raise ValueError("Start do pomiaru skalowania musi występować w manifeście instancji.")

    manifest = {
        "checkout": checkout_record(("src/omp/p1.cpp", "src/omp/p1.hpp", "src/omp/p1_main.cpp",
                                     "bench/run_omp_scaling.py")),
        "executables": binaries,
        "config": file_record(args.config),
        "parameters": config,
        "datasets": {folder.as_posix(): dataset for folder, dataset in datasets.items()},
        "host": {"system": platform.platform(), "processor": platform.processor(),
                 "logical_cpus": os.cpu_count(), "python": platform.python_version()},
        "openmp_environment": {key: env[key] for key in ("OMP_DYNAMIC", "OMP_NUM_THREADS",
                                "OMP_THREAD_LIMIT", "OMP_PROC_BIND", "OMP_PLACES") if key in env},
        "timing": {"t_prep": "C++ instance and start file parsing", "t_search": "C++ solver call",
                   "t_total": "wall clock around the C++ process including file and console I/O"},
    }
    args.out.mkdir(parents=True)
    write_manifest(args.out / "manifest.json", manifest)

    variants = [("s1", 1), *(("p1", count) for count in counts)]
    correctness = []
    warmups = []
    measured = []
    with TemporaryDirectory(dir=args.out) as temporary:
        run_index = 0

        def run(stage, count, folder, start_name, rep):
            nonlocal run_index
            run_index += 1
            instance = folder / datasets[folder]["instance"]["file"]
            start = folder / start_name
            output = Path(temporary) / f"{run_index}.csv"
            thread_args = ("--threads", str(count)) if stage == "p1" else ()
            row, signature, _ = run_solver(executables[stage], instance, start, output, rep,
                                           solver_args=thread_args, env=env)
            if int(row["threads"]) != count:
                raise ValueError(f"{stage} użył {row['threads']} z {count} żądanych wątków dla {start}")
            return {"stage": stage, "start": start.as_posix(), **row}, (float(row["cost"]), signature)

        for folder in (scaling, *validation):
            names = [entry["file"] for entry in datasets[folder]["starts"]]
            if folder == scaling:
                names = [scaling_start]
            for start in names:
                row, reference = run("s0", 1, folder, start, 1)
                correctness.append(row)
                if folder == scaling:
                    scaling_reference = reference
                    continue
                for stage, count in variants:
                    row, outcome = run(stage, count, folder, start, 1)
                    check_solution(reference, outcome, (stage, count, folder, start))
                    correctness.append(row)

        for rep in range(reps + 1):
            # Zmieniamy kolejność, żeby ten sam wariant nie startował zawsze jako pierwszy.
            offset = rep % len(variants)
            for stage, count in variants[offset:] + variants[:offset]:
                row, outcome = run(stage, count, scaling, scaling_start, rep)
                check_solution(scaling_reference, outcome, (stage, count, rep))
                (warmups if rep == 0 else measured).append(row)

    write_rows(args.out / "validation.csv", correctness)
    write_rows(args.out / "warmup.csv", warmups)
    write_rows(args.out / "scaling.csv", measured)
    for metric in ("t_search", "t_total"):
        baseline = statistics.median(float(row[metric]) for row in measured if row["stage"] == "s1")
        for stage, count in variants:
            times = [float(row[metric]) for row in measured
                     if row["stage"] == stage and int(row["threads"]) == count]
            q1, _, q3 = statistics.quantiles(times, method="inclusive")
            median = statistics.median(times)
            print(f"{metric} {stage} {count:2}: {median * 1000:8.3f} ms, "
                  f"IQR {(q3 - q1) * 1000:7.3f} ms, speedup {baseline / median:.2f}x")


if __name__ == "__main__":
    main()
