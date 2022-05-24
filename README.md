# ParallelMatrixMul
1. Matrix multiplication using MPI  
2. Block matrix multiplication (trying to optimize cache usage), debugging needed  
  
## Build & Run  
#### Matrix multiplication using MPI  
```
mpic++ -o mm.out main.cpp matrix.cpp  
```
or  
```
mpiicpc -O0 -o mm.out main.cpp matrix.cpp  
```
or  
```
"other mpi c++ compiler" mm.out main.cpp matrix.cpp  
```
**Run**:  
```
mpirun -np p ./mm.out  
```
where p is the number of mpi processes  

