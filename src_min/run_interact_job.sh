#!/bin/bash

# Make directory to save data file
# Time=`date '+%m%d%H%M%S'`
# Time="0908000000"

Time="0123456789"
RESULT_DIR="./al_result/"
VOLTAGE_DIR="${RESULT_DIR}${Time}/voltage"
SPIKE_DIR="${RESULT_DIR}${Time}/spike"
CMAES_DIR="${RESULT_DIR}${Time}/cmaes"
OUT="${RESULT_DIR}${Time}/out"
echo "DATA DIRECTORY : ${VOLTAGE_DIR}"
mkdir -p ${VOLTAGE_DIR}
mkdir -p ${SPIKE_DIR}
mkdir -p ${CMAES_DIR}

echo ${NRNOPT} | tee "${RESULT_DIR}${Time}/NRNOPT.param"

# argv for estimate_main
NUM_OF_POP=2
MU=1
NUM_OF_CHILD_PROCS=2
MAXEVAL=1
NUM_OF_GRANDCHILD_PROCS=20
EXEC_PROG="/vol0004/hp200177/u00690/neuron_kplus/specials/sparc64/special"
DIM_CON_MAT=6
CON_MAT_FILENAME="../cmaes_input/conMat_6.txt";
PARAMETER_FILENAME="../cmaes_input/params_pn.txt";

# minimal estimate_main
EXEC_FILE="./estimate_main ${NUM_OF_POP} ${MU} ${NUM_OF_CHILD_PROCS} ${MAXEVAL} ${NUM_OF_GRANDCHILD_PROCS} ${EXEC_PROG} ${DIM_CON_MAT} ${CON_MAT_FILENAME} ${PARAMETER_FILENAME} ${CMAES_DIR}"

# execute job
mpiexec -np 1  ${EXEC_FILE} 2>&1 | tee stdout_seikai100.log
