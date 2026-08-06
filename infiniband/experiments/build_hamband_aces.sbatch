#!/bin/bash
#SBATCH --account=156196634860
#SBATCH --partition=cpu
#SBATCH --job-name=hamband_ib_build
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=24G
#SBATCH --time=02:00:00

set -euo pipefail

ROOT=${HAMBAND_ROOT:-/scratch/user/u.js213354/Hamband/infiniband}
mkdir -p "$ROOT/results"
exec > >(tee "$ROOT/results/hamband_ib_build.${SLURM_JOB_ID}.log") 2>&1

cd "$ROOT"
module purge
module load gcc/9.2.1
export PATH="/scratch/user/u.js213354/bin:/scratch/user/u.js213354/python-local/bin:$PATH"

./build.py crash-consensus
cd wellcoordination
bash build.sh

test -x build/bin/band
test -x build/bin/band-crdt
sha256sum build/bin/band build/bin/band-crdt
