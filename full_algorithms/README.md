# Motif Clustering

Requirements
-----------

 - A 64-bit Linux operating system.
 - A modern, ![C++17](https://img.shields.io/badge/C++-17-blue.svg?style=flat)-ready compiler such as `g++` version 7 or higher or `clang` version 11.0.3 or higher.
 - The [cmake][cmake] build system (>= 3.16).
 - The [Boost - Program Options][Boost.Program_options] library and the boost header files (>= 1.48).
 - The [Intel Thread Building Blocks][tbb] library (TBB)
 - The [Portable Hardware Locality][hwloc] library (hwloc)
 - An MPI implementation, e.g., OpenMPI (https://www.open-mpi.org/) or MPICH (https://www.mpich.org)

Building the Motif Clustering Application
-----------

First of all, install the required depedencies using the following command (for linux):
```console
sudo apt-get install libboost-program-options-dev libnuma-dev numactl libhwloc-dev moreutils linux-tools-common linux-tools-generic libtbb-dev openmpi-bin
```

Then, just run the command below:
```console
./build_all.sh

```
Running the Motif Clustering Application
-----------

Find out the parameters available for each of our applications with the command below, where <program> should be substituted by each of our applications

```console
./deploy/<program> --help
```
