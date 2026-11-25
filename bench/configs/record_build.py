"""Zapisuje informacje zgłaszane przez binaria i stan katalogu źródłowego."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SOURCES = (
    "CMakeLists.txt",
    "bench/configs/record_build.py",
    "bench/run_seq_profile.py",
    "data/gen.py",
    "src/common/input.cpp",
    "src/common/input.hpp",
    "src/common/nearest_search.cpp",
    "src/common/nearest_search.hpp",
    "src/common/solution.cpp",
    "src/common/solution.hpp",
    "src/common/solver_cli.cpp",
    "src/common/solver_cli.hpp",
    "src/seq/build_info.hpp",
    "src/seq/phase_profile.hpp",
    "src/seq/s0.cpp",
    "src/seq/s0.hpp",
    "src/seq/s0_main.cpp",
    "src/seq/s1.cpp",
    "src/seq/s1.hpp",
    "src/seq/s1_main.cpp",
)


def file_record(path):
    path = path.resolve(strict=True)
    saved_path = path.relative_to(ROOT) if path.is_relative_to(ROOT) else path
    return {
        "path": saved_path.as_posix(),
        "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
    }


def git(*args):
    return subprocess.check_output(["git", "-C", str(ROOT), *args], text=True).strip()


def executable_record(executable):
    # Metadane odczytujemy z pliku wykonywalnego użytego w serii.
    executable = executable.resolve(strict=True)
    output = subprocess.check_output([str(executable), "--build-info"], text=True)
    info = dict(line.split("=", 1) for line in output.splitlines())
    return {
        **file_record(executable),
        "profiling": bool(int(info.pop("profile"))),
        **info,
    }


def checkout_record(extra_sources=()):
    # Stan źródeł opisuje katalog w chwili pomiaru. Hash binarium identyfikuje użyty program.
    return {
        "commit": git("rev-parse", "HEAD"),
        "dirty": bool(git("status", "--porcelain")),
        "source_sha256": {name: file_record(ROOT / name)["sha256"] for name in (*SOURCES, *extra_sources)},
    }


def build_manifest(release, profile):
    builds = {}
    for name, executables, profiling in (("release", release, False), ("profile", profile, True)):
        builds[name] = {stage: executable_record(executable) for stage, executable in executables.items()}
        for info in builds[name].values():
            if info["profiling"] != profiling:
                raise ValueError(f"Nieprawidłowe ustawienie profilowania w {info['path']}")
    return {"checkout": checkout_record(), "executables": builds}


def write_manifest(path, data):
    with path.open("x", encoding="utf-8") as output:
        json.dump(data, output, ensure_ascii=False, indent=2)
        output.write("\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("release-s0", "release-s1", "profile-s0", "profile-s1", "out"):
        parser.add_argument(f"--{name}", type=Path, required=True)
    args = parser.parse_args()
    write_manifest(args.out, build_manifest(
        {"s0": args.release_s0, "s1": args.release_s1},
        {"s0": args.profile_s0, "s1": args.profile_s1},
    ))


if __name__ == "__main__":
    main()
