#! /bin/bash

#PBS -l walltime=14:00:00
#PBS -l select=1:ncpus=4:mem=4gb


# Load intel compiler
module load intel-suite/2017.1 

# Copy files to $TMPDIR
dir="$HOME"/MD/MD-simulation/src
cd "$dir"
cp vx.txt vy.txt vz.txt MD.h MD.cpp stat_analysis.h stat_analysis.cpp Source.cpp $TMPDIR

# IMPORTANT: SHELL DIR HAS TO BE SAME AS VX, VY, VZ
cd $TMPDIR
pwd
# Dir created in case of early termination
OUTDIR="$HOME"/MD/Data
mkdir "$OUTDIR"
LIB=/apps/intel/2017.1/compilers_and_libraries_2017.1.132/linux/compiler/include
# Compile
icpc -pthread -parallel -mkl=parallel -O3 -xHOST -std=c++17 -use-intel-optimized-headers -m64 -prec-sqrt -prec-div MD.cpp Source.cpp -o a.out
# Run
./a.out
# Copy files from $TMPDIR to $WORK

cp * "$OUTDIR"
