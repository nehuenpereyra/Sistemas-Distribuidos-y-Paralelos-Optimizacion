#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH -o salida_secuencial/output.txt
#SBATCH -e salida_secuencial/errores.txt
./punto_uno_secuencial $1