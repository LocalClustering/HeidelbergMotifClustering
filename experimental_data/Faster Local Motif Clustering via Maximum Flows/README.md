This folder is associated with the paper "**Faster Local Motif Clustering via Maximum Flows**", which has been accept for publication as a full paper at ESA 2023.
Here you can find a [technical report](https://arxiv.org/pdf/2301.07145.pdf).

Folder Structure
-----------

This folder contains the following subfolders and files:

 - algorithms/ - folder containing the code of SOCIAL as well as MAPPR and the graph-based version of LMCHGP.
 - database/ - containing a sample of the graphs used in our experiments.
 - experiments/ - folder containing metascripts to generate and execute a sample of our experiments.
 - full_results/ - folder containing full experimental data of the comparison of SOCIAL against LMCHGP and MAPPR.
 - full_plotting/ - folder containing scripts to plot all charts and the table contained in our paper.
 - build_all.sh - script to automatically compile the algorithms contained in algorithms/.
 - run_sample.sh - script to generate, run, and analyze a sample of our experiments.
 - clean_all.sh - script to clean the generated sample of experiments and the binary files of the algorithms contained in algorithms/.

Description
-----------

This reproducibility folder is associated with the manuscript "Local Motif Clustering via (Hyper)Graph Partitioning".
It contains a sample graph (database/), a code base containing SOCIAL, MCHGP, and MAPPR (algorithms/), and a set of scripts to generate sample experiments (experiments/). These sample experiments can be easily executed as shown below.
Additionally, this folder contains the full experimental data for comparisons against MAPPR (full_results/) and full scripts to generate the charts and the table contained (full_plotting/) in the paper.
Please follow the instructions below to compile the code, run the sample experiments, and plot the charts.


Requirements to Compile Algorithms
-----------

 - A 64-bit Linux operating system.
 - A modern, ![C++17](https://img.shields.io/badge/C++-17-blue.svg?style=flat)-ready compiler such as `g++` version 7 or higher or `clang` version 11.0.3 or higher.
 - The [cmake][cmake] build system (>= 3.16).
 - The [Boost - Program Options][Boost.Program_options] library and the boost header files (>= 1.48).
 - The [Intel Thread Building Blocks][tbb] library (TBB)
 - The [Portable Hardware Locality][hwloc] library (hwloc)
 - An MPI implementation, e.g., OpenMPI (https://www.open-mpi.org/) or MPICH (https://www.mpich.org)

Building the Code Base (algorithms/)
-----------

First of all, install the required depedencies using the following command (for linux):

```console
sudo apt-get install libboost-program-options-dev libnuma-dev numactl libhwloc-dev moreutils linux-tools-common linux-tools-generic libtbb-dev openmpi-bin
```

Then, just run the command below:

```console
./build_all.sh
```

Running Sample Experiments
-----------

Run the command below to create a set of experiments, run them, extract their results to a CSV file, and print statistics:

```console
./run_sample.sh
```

The executed experiments will be located in the folder experiments/sample_experiments/

Cleaning All
-----------

Run the command below to clean executables, sample experiments, and plots

```console
./clean_all.sh
```

Plotting Full Charts (full_plotting/)
-----------

Enter the folder full_plotting/ and follow the instructions contained in README

