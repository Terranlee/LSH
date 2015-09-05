Locality Sensitive Hashing(LSH)
======

## Introduction  
Locality Sensitive Hashing(LSH) is a number of hash functions that similar items map to the same 'bucket' with a higher possibility. Locality sensitive hashing can be used in approximate nearest neighbor search or high dimension data indexing.   
LSH requires a large amount of calculation, and building a LSH index for a large amount of data may take a long time. However, LSH is very suitable for dataflow engine acceleration, because the length of the loop is fixed and the calculation is uncorrelated.  

## Algorithms  
LSH function has the following characters to guarantee the 'locality'  
* 1. if d(p1, p2) < R, then h(p1)=h(p2) with possibility at least Pr1  
* 2. if d(p1, p2) > R, then h(p1)=h(p2) with possibility at most Pr2  
* 3. Pr1 > Pr2  

In this dataflow version of LSH, we implement p-stable LSH, which is a random hyperplane and the hash value is the distance between point and hyperplane.  
For more details, please visit  
* 1. [LSH introduction](https://en.wikipedia.org/wiki/Locality-sensitive_hashing)  
* 2. [p-stable LSH](http://web.mit.edu/andoni/www/LSH/)  
* 3. [LSH.pdf](lsh.pdf)  

## Notice  
This implementation is just a simple version of p-stable LSH, in some other versions, LSH also needs a rehash procedure or other kernel function. You can just define another kernel hashing function and combine it with our manager.  
