 # HDCat

Here is a hot data identification algorithm called HDCat, which can accurately capture the temporal locality of data access patterns and achieve a high hit ratio with low cache capacity and runtime overhead.

## The HDCat paper

Chen J, Deng Y, Huang Z. HDCat: Effectively Identifying Hot Data in Large-Scale I/O Streams with Enhanced Temporal Locality[C]//International Conference on Algorithms and Architectures for Parallel Processing. Springer, Cham, 2015: 120-133. 

## Setting up experimental environment
### 1) Trace
The trace in the paper is downloaded at http://iotta.snia.org/tracetypes/3,.You need to add a folder in the computer D drive TestData to store the trace. As shown below:
![Image text](https://github.com/love-light/imag/blob/master/image/1.png)
![Image text](https://github.com/love-light/imag/blob/master/1%EF%BC%892.png)

### 2) Operating environment
Program running environment need to install the database mySQL, and use VC or VS series compiler to compile the program. You need to add a database called cm in mySQL software, and in this database to establish the table rack1, rack2, rack3, three tables are designed as follows:
![Image text](https://github.com/love-light/imag/blob/master/2%EF%BC%891.png)

### 3) Operating environment
Test program interface as follows, first load the data, and then use the various strategies to test.
![Image text](https://github.com/love-light/imag/blob/master/2%EF%BC%891.png)
