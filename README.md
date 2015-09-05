=============================
LSH
=============================

Dataflow engine acceleration of Locality Sensitive Hashing(LSH)


Introduction
------------

Locality Sensitive Hashing(LSH) is a useful technique in nearest neighbor search, high dimension data indexing and so on. It requires a large amount of calculation with no dependencies with each other. So, this project tries to make use of the character of Maxeler Dataflow Engine to accelerate this hashing procedure.  


Usage
-----

* APP directory consists of the CPU version and dataflow version of LSH, it also includes some tests to compare the performance and make sure of the accuracy. See APP/README.md for details about compiling and running the code.   
* ORIG directory consists of the original CPU version of LSH, you can use code in this directory to get familiar with LSH. See ORIG/README.md for details about compiling and running the code.   
* DOC directory consists of a brief description of LSH method, and the hashing function we use. It also includes some analysis about the performance.  


Features
--------

LSH is completely a stream based algorithm with no dependencies, and can be accelerated well using dataflow engine.  
We can get tens of times of acceleration when the dataset is very large, and the resource use is not very large(less than 20% of the total resource for the LSHKernel in APP directory), so the performance can be better if we duplicate kernels in one FPGA.  

To-Do
-----

In this project, we only implement p-stable LSH function in LSHKernel.maxj, you can also implement other hash functions such as random hyperplane, bit sampling or TLSH.  
The current hashing result is represented by 128bits for any output dimensions. But if the output dimension is very high or hash value is very large, it may cause an overflow and lead to collisions. You can either increase number of bits, or let the length of hashing result to increase with output dimension.   
