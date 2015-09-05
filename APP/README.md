Locality Sensitive Hashing(LSH)
======

## Introduction  
This is the CPU and DFE version of LSH  
In src/CPUCode/lsh_dfe.cpp, the function 'fit' includes the test of accuracy and performance of LSH.  
Data is randomly generated with in a fixed range, you can also read input data from file.  

## Algorithms  
Implement p-stable LSH on CPU and DFE  

## Compile & run  
Use the makefile in build directory, this project does not use the makefile template provided by MaxAppTemplate  
>  Make a reservation of DFE and change Makefile.Maia.hardware
>  $cd build  
>  $make (show all commands)  
>  $make runsim  
>  $make build  


## Dependencies  
#### Platform and libraries  
1. g++/icpc  
2. boost  (boost/numeric/ublas)  
3. MaxCompiler2014.1, MAIA dataflow engine environment  
