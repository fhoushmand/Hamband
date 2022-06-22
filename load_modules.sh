# install conan and ninja first
# make sure to use batch partition to compile and run the experiments

#module load cmake;
#module remove miniconda3/py39_4.12.0
#module remove python;
#module load python/3;
#module load extra;
#module load GCCcore/7.4.0;
module load gcc/9.2.1
export PATH=$PATH:/rhome/fhous001/farzin/FastChain/ninja;
# make sure you already created the mu_env virtualenv 
source ../band_env/bin/activate

# run interactive shell on different nodes to test the program:
# srun -p batch --mem 50gb --cpus-per-task 32 --ntasks 1 --pty bash -l 
