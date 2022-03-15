#!/bin/bash
pjsub --interact -L "node=12" -L "rscgrp=int" -L "elapse=6:00:00" --sparam "wait-time=600" --mpi "proc=48" --llio sharedtmp-size=20Gi
