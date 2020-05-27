# Simulation Program

Exam ID: B156924

This program can be executed on the computing nodes of the Cirrus platform with Linux system  

The GCC version: 4.8.5

## Step of running the program on Cirrus  

1. load the modules:
```bash
module load mpt intel-compilers-18
```

2. compile the files:  
```bash
make
```

3. run on the computing node: open the bin folder, then input
```
qsub simulation.pbs
```

4. clean all .o files and execution file:
```bash
make clean
```

## Change parameters  

please open the 'include/simulation.h' file, you could change:

*  INIT_SQUIRRELS: the number of initial healthy squirrels
*  INIT_INFECTED: the number of infected squirrels
*  MODEL_MONTH: the overall months that the simulation takes
*  TIMEGAP: how long will a month take in this simulation (units: second)
*  CN: the square of CN is the provided number of cells, if you change this parameter, you should also change the NUM_OF_CELL to the square of CN