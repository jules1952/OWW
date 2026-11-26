"""Sprawdza P1 względem S0, porównuje skalowanie z S1 i opcjonalnie mierzy G1."""
import argparse
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
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
    parser.add_argument("--include-cuda", action="store_true", help="Sprawdź i zmierz G1 razem z CPU.")
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
    stages = ("s0", "s1", "p1", "g1") if args.include_cuda else ("s0", "s1", "p1")
    executables = {stage: args.build / f"pm_{stage}{suffix}" for stage in stages}
    binaries = {stage: executable_record(path) for stage, path in executables.items()}
    if any(info["configuration"] != "Release" or info["profiling"] for info in binaries.values()):
        raise ValueError("Użyj kompilacji Release z wyłączonym profilowaniem.")
    env = {**os.environ, "OMP_DYNAMIC": "FALSE"}
    scaling_config = config["scaling"]
    if isinstance(scaling_config, dict):
        scaling_config = [scaling_config]
    scaling_cases = [(Path(case["directory"]), case["start"]) for case in scaling_config]
    if not scaling_cases or len(set(scaling_cases)) != len(scaling_cases):
        raise ValueError("scaling musi wskazywać co najmniej jedną unikalną parę katalogu i startu.")
    validation = [Path(path) for path in config["validation"]]
    folders = dict.fromkeys(folder for folder, _ in scaling_cases)
    folders.update(dict.fromkeys(validation))
    datasets = {folder: read_dataset(folder) for folder in folders}
    for folder, start in scaling_cases:
        if start not in {entry["file"] for entry in datasets[folder]["starts"]}:
            raise ValueError(f"Brak startu {start} w manifeście instancji {folder}.")

    sources = ("src/omp/p1.cpp", "src/omp/p1.hpp", "src/omp/p1_main.cpp", "bench/run_omp_scaling.py")
    if args.include_cuda:
        sources += ("src/cuda/g1.cu", "src/cuda/g1.hpp", "src/cuda/g1_main.cpp")
    manifest = {
        "checkout": checkout_record(sources),
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
    if args.include_cuda:
        manifest["gpu"] = subprocess.check_output(
            ["nvidia-smi", "--query-gpu=name,driver_version,memory.total", "--format=csv,noheader"],
            text=True).strip().splitlines()
        manifest["cuda_timing"] = {
            "t_prep": "input parsing and solver setup including distances and GPU allocations",
            "t_h2d": "blocking copies of distances, weights and iteration state",
            "t_search": "solver call excluding solver setup and H2D/D2H",
            "t_d2h": "blocking copies of swap costs after kernel completion",
            "gpu_mem_mb": "device buffers allocated by G1 in MiB",
        }
    args.out.mkdir(parents=True)
    write_manifest(args.out / "manifest.json", manifest)

    variants = [("s1", 1), *(("p1", count) for count in counts)]
    if args.include_cuda:
        variants.append(("g1", 1))
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
            if args.include_cuda:
                row.update(h2d_bytes="", d2h_bytes="")
                if stage == "g1":
                    meta = dict(line.split("=", 1) for line in
                                output.with_suffix(".csv.meta.txt").read_text(encoding="utf-8").splitlines())
                    row.update(h2d_bytes=meta["h2d_bytes"], d2h_bytes=meta["d2h_bytes"])
            return {"stage": stage, "start": start.as_posix(), **row}, (float(row["cost"]), signature)

        references = {}

        def reference_for(folder, start):
            # Instancja może wystąpić zarówno w serii czasowej, jak i w walidacji.
            key = (folder, start)
            if key not in references:
                row, references[key] = run("s0", 1, folder, start, 1)
                correctness.append(row)
            return references[key]

        for folder in validation:
            for entry in datasets[folder]["starts"]:
                start = entry["file"]
                reference = reference_for(folder, start)
                for stage, count in variants:
                    row, outcome = run(stage, count, folder, start, 1)
                    check_solution(reference, outcome, (stage, count, folder, start))
                    correctness.append(row)

        for folder, start in scaling_cases:
            print(f"{folder}/{start}", flush=True)
            reference = reference_for(folder, start)
            case_rows = []
            for rep in range(reps + 1):
                # Zmieniamy kolejność, żeby ten sam wariant nie startował zawsze jako pierwszy.
                offset = rep % len(variants)
                for stage, count in variants[offset:] + variants[:offset]:
                    row, outcome = run(stage, count, folder, start, rep)
                    check_solution(reference, outcome, (stage, count, folder, start, rep))
                    (warmups if rep == 0 else case_rows).append(row)
            measured.extend(case_rows)
            print_summary(case_rows, variants)

    write_rows(args.out / "validation.csv", correctness)
    write_rows(args.out / "warmup.csv", warmups)
    write_rows(args.out / "scaling.csv", measured)


def print_summary(measured, variants):
    for metric in ("t_search", "t_total"):
        baseline = statistics.median(float(row[metric]) for row in measured if row["stage"] == "s1")
        for stage, count in variants:
            times = [float(row[metric]) for row in measured
                     if row["stage"] == stage and int(row["threads"]) == count]
            q1, _, q3 = statistics.quantiles(times, method="inclusive")
            median = statistics.median(times)
            # G1 przenosi obliczenie odległości do t_prep. Speedup GPU liczymy dla całego procesu.
            speedup = "" if stage == "g1" and metric == "t_search" else f", speedup {baseline / median:.2f}x"
            print(f"{metric} {stage} {count:2}: {median * 1000:8.3f} ms, "
                  f"IQR {(q3 - q1) * 1000:7.3f} ms{speedup}")


if __name__ == "__main__":
    main()
