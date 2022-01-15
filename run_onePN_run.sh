#!/bin/bash

# Make directory to save data file
Time=`date '+%m%d%H%M%S'`
RESULT_DIR="./one_result/"
VOLTAGE_DIR="${RESULT_DIR}${Time}/voltage"
SPIKE_DIR="${RESULT_DIR}${Time}/spike"
OUT="${RESULT_DIR}${Time}/out"
echo "DATA DIRECTORY : ${VOLTAGE_DIR}"
mkdir -p ${VOLTAGE_DIR}
mkdir -p ${SPIKE_DIR}

NRNOPT=\
" -c STOPTIME=600"\
" -c INTERVAL=1000"\
" -c SAVEDIR=${Time}"\
" -c WRUP=0.5"\
" -c WRMP=1.0"\
" -c WRL=1.0"\
" -c WUPR=1.0"\
" -c WUPMP=1.0"\
" -c WUPL=1.0"\
" -c WMPR=1.0"\
" -c WMPUP=1.0"\
" -c WMPMP=1.0"\
" -c WMPL=1.0"\
" -c WLR=1.0"\
" -c WLUP=1.0"\
" -c WLMP=1.0"\
" -c WLL=1.0"\
" -c GLOMID=8"\
" -c USE_STDP=0"\
" -c SAVE_ALL=0"

NRNIV="/home/hp200177/u00690/neuron_kplus/specials/sparc64/special -mpi"
HOC_NAME="./onePN_networkSimulation.hoc"
MPIEXEC="mpiexec -n 28 "

echo "${MPIEXEC} ${NRNIV} ${NRNOPT} ${HOC_NAME}"
echo ${NRNOPT} | tee "${RESULT_DIR}${Time}/NRNOPT.param"
time ${MPIEXEC} ${NRNIV} ${NRNOPT} ${HOC_NAME} | tee $OUT


#python ./result/analyze/drawGraph.py $VOLTAGE_DIR
#python ./result/analyze/drawISF.py $SPIKE_DIR
#eog $VOLTAGE_DIR + "/11225.png"
##python ./result/analyze/countSpike.py $SPIKE_DIR

# sync
