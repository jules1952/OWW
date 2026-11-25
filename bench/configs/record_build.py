"""Zapisuje powiązanie plików źródłowych z właśnie zbudowanymi programami."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stages', nargs='+', choices=['S0','S1','PROFILE'], required=True)
    args = parser.parse_args()
    path = ROOT/'build/seq_build_manifest.json'
    data = json.loads(path.read_text()) if path.exists() else {}
    common = ['src/common/input.cpp','src/common/input.hpp','src/seq/s0.hpp']
    sources = {
        'S0': common+['src/seq/s0.cpp','src/seq/s0_main.cpp'],
        'S1': common+['src/seq/s0.cpp','src/seq/s1.cpp','src/seq/s1.hpp','src/seq/s1_main.cpp'],
        'PROFILE': common+['src/seq/s0.cpp','profiles/generated/s0_profiled.cpp','profiles/s0_profile.hpp','src/seq/s0_main.cpp']}
    for stage in args.stages:
        name = {'S0':'pm_s0','S1':'pm_s1','PROFILE':'pm_s0_profile'}[stage]
        exe = ROOT/'build'/(name+('.exe' if os.name=='nt' else ''))
        data[stage] = {'built_at_utc':datetime.now(timezone.utc).isoformat(),
                       'executable':str(exe.relative_to(ROOT)), 'executable_sha256':digest(exe),
                       'source_sha256':{s:digest(ROOT/s) for s in sources[stage]},
                       'build_info':(ROOT/'build/seq_build_info.txt').read_text(encoding='utf-8',errors='replace')}
    path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
    print('Zapisano build/seq_build_manifest.json: '+', '.join(args.stages))


if __name__ == '__main__':
    main()
