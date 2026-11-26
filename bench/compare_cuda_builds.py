"""Porównuje dwie kompilacje Release G1 na instancjach i startach z serii E5."""
import argparse
import json
import os
from pathlib import Path
from tempfile import TemporaryDirectory

from configs.record_build import checkout_record, executable_record, file_record, write_manifest
from run_cuda_profile import reference_cases
from run_seq_profile import check_solution, run_solver, write_rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("before", "after", "baseline", "out"):
        parser.add_argument(name, type=Path)
    args = parser.parse_args()
    name = "pm_g1.exe" if os.name == "nt" else "pm_g1"
    executables = {"before": args.before / name, "after": args.after / name}
    builds = {stage: executable_record(path) for stage, path in executables.items()}
    if any(info["configuration"] != "Release" or info["profiling"] for info in builds.values()):
        raise ValueError("Użyj dwóch kompilacji Release z wyłączonym profilowaniem.")
    for key in ("compiler", "cuda", "cuda_architectures"):
        if builds["before"][key] != builds["after"][key]:
            raise ValueError(f"Niezgodne ustawienie kompilacji: {key}.")
    if {info["cuda_block_size"] for info in builds.values()} != {"64", "128"}:
        raise ValueError("Porównaj kompilację z 64 wątkami na blok z kompilacją z 128 wątkami.")

    manifest_path = args.baseline / "manifest.json"
    validation_path = args.baseline / "validation.csv"
    baseline = json.loads(manifest_path.read_text(encoding="utf-8"))
    # Parzysta liczba pomiarów daje obu kompilacjom tyle samo startów jako pierwsza.
    repetitions = 6
    manifest = {
        "checkout": checkout_record(("bench/compare_cuda_builds.py", "bench/run_omp_scaling.py",
                                    "bench/run_cuda_profile.py", "src/cuda/g1.cu", "src/cuda/g1.hpp",
                                    "src/cuda/g1_main.cpp")),
        "executables": builds,
        "baseline": {"manifest": file_record(manifest_path), "validation": file_record(validation_path)},
        "warmups_per_build": 1,
        "repetitions_per_build": repetitions,
    }
    args.out.mkdir(parents=True)
    warmups = []
    measurements = []
    with TemporaryDirectory(dir=args.out) as temporary:
        for instance, start, expected in reference_cases(baseline, validation_path):
            print(start.as_posix(), flush=True)
            for rep in range(repetitions + 1):
                order = ("after", "before") if rep % 2 else ("before", "after")
                for stage in order:
                    path = Path(temporary) / f"{len(warmups) + len(measurements)}.csv"
                    row, signature, _ = run_solver(executables[stage], instance, start, path, rep)
                    check_solution(expected, (float(row["cost"]), signature), (start, stage, rep))
                    (measurements if rep else warmups).append({"build": stage, "start": start.as_posix(), **row})

    write_rows(args.out / "warmup.csv", warmups)
    write_rows(args.out / "measurements.csv", measurements)
    write_manifest(args.out / "manifest.json", manifest)


if __name__ == "__main__":
    main()
