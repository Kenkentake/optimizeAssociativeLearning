#!/bin/bash

# Make directory to save data file
Time=`date '+%m%d%H%M%S'`
RESULT_DIR="./al_result/"
VOLTAGE_DIR="${RESULT_DIR}${Time}/voltage"
SPIKE_DIR="${RESULT_DIR}${Time}/spike"
CMAES_DIR="${RESULT_DIR}${Time}/cmaes"
OUT="${RESULT_DIR}${Time}/out"
echo "DATA DIRECTORY : ${VOLTAGE_DIR}"
mkdir -p ${VOLTAGE_DIR}
mkdir -p ${SPIKE_DIR}
mkdir -p ${CMAES_DIR}

echo share
# copy input file from Second-layer storage to Shared temporary area
mkdir -p ${PJM_SHAREDTMP}/input/
time cp -r ../input/synlist ${PJM_SHAREDTMP}/input/synlist
echo copied neuronlit
time cp -r ../input/neuronlist ${PJM_SHAREDTMP}/input/neuronlist
echo copied neuronlit
time cp -r ../input/spiketiming ${PJM_SHAREDTMP}/input/spiketiming
echo copied spiketiming
time cp -r ../input/swc ${PJM_SHAREDTMP}/input/swc
echo copied swc
time cp -r ../cmaes_input ${PJM_SHAREDTMP}/cmaes_input
echo copied cmaes_input

# argv for estimate_main
NUM_OF_POP=2
MU=1
NUM_OF_CHILD_PROCS=2
MAXEVAL=1
NUM_OF_GRANDCHILD_PROCS=20
EXEC_NEURON="/vol0004/hp200177/u00690/neuron_kplus/specials/sparc64/special"
NEURON_OPTION="{SAVEDIR=${Time}}"
EXEC_HOC="../al_networkSimulation.hoc"
DIM_CON_MAT=6
CON_MAT_FILENAME="${PJM_SHAREDTMP}/cmaes_input/conMat_6.txt";
PARAMETER_FILENAME="${PJM_SHAREDTMP}/cmaes_input/params_pn.txt";

# minimal estimate_main
EXEC_FILE="./estimate_main ${NUM_OF_POP} ${MU} ${NUM_OF_CHILD_PROCS} ${MAXEVAL} ${NUM_OF_GRANDCHILD_PROCS} ${EXEC_NEURON} ${NEURON_OPTION} ${EXEC_HOC} ${DIM_CON_MAT} ${CON_MAT_FILENAME} ${PARAMETER_FILENAME} ${CMAES_DIR}"

# execute job
mpiexec -np 1  ${EXEC_FILE} 2>&1 | tee stdout.log
