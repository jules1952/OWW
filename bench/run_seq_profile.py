"""Zapisuje porównanie czasu solverów i osobny profil faz S0/S1."""
import argparse
import csv
from pathlib import Path
import statistics
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("release", "profile", "instance", "start", "out"):
        parser.add_argument(name, type=Path)
    args = parser.parse_args()
    args.out.mkdir(parents=True)
    timings = {"s0": [], "s1": []}
    reference = None
    with (args.out / "phases.csv").open("w", newline="") as output:
        writer = csv.writer(output)
        writer.writerow(["stage", "phase", "calls", "seconds"])
        for repetition in [*range(6), "profile"]:
            stages = ("s1", "s0") if repetition in (1, 3, 5) else ("s0", "s1")
            for stage in stages:
                profiling = repetition == "profile"
                executable = (args.profile if profiling else args.release) / f"pm_{stage}.exe"
                path = args.out / f"{stage}-{repetition}.csv"
                run = subprocess.run(
                    [str(executable.resolve()), str(args.instance), str(args.start), str(path)],
                    check=True, capture_output=True, text=True,
                )
                with path.open(newline="") as result:
                    row = next(csv.DictReader(result))
                signature = tuple(row[key] for key in ("cost", "final_set_hash", "iterations"))
                if reference is None:
                    reference = signature
                if signature != reference:
                    raise ValueError(f"Rozbieżność wyniku w {path}")
                if profiling:
                    for phase in csv.reader(run.stderr.splitlines()):
                        writer.writerow([stage, *phase])
                elif repetition:
                    timings[stage].append(float(row["t_search"]))
    for stage, times in timings.items():
        q1, _, q3 = statistics.quantiles(times, method="inclusive")
        print(stage, "mediana", statistics.median(times), "IQR", q3 - q1)
    print("speedup", statistics.median(timings["s0"]) / statistics.median(timings["s1"]))
    print("wynik", reference)


if __name__ == "__main__":
    main()
