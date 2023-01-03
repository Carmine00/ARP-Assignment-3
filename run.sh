#!/bin/bash
#Script to compile all the processes and run the master
gcc ./src/master.c -o ./bin/master 
gcc ./src/processA.c -lbmp -lncurses -lm -lrt -o ./bin/processA 
gcc ./src/processAS.c -lbmp -lncurses -lm -lrt -o ./bin/processAS 
gcc ./src/processAC.c -lbmp -lncurses -lm -lrt -o ./bin/processAC
gcc ./src/processB.c -lbmp -lncurses -lm -lrt -o ./bin/processB 
./bin/master

