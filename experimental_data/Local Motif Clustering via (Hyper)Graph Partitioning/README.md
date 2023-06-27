This folder is associated with the paper "**Local Motif Clustering via (Hyper)Graph Partitioning**", which has been published as an extended abstract at [SoCS 2022](https://ojs.aaai.org/index.php/SOCS/article/view/21779) and as a full paper at [SIAM ALENEX 2023](https://doi.org/10.1137/1.9781611977561.ch9).
Additionally, you can find a [technical report](https://arxiv.org/pdf/2205.06176.pdf) and our SoCS 2022 [poster](misc/SoCS_Poster.pdf).

Folder Structure
-----------

This folder contains the following subfolders and files:

 - [algorithms/](algorithms/) - folder containing the code of the graph-based version of our algorithm as well as MAPPR
 - [database/](database/) - containing a sample of the graphs used in our experiments
 - [experiments/](experiments/) - folder containing metascripts to generate and execute a sample of our experiments
 - [full_results/](full_results/) - folder containing full experimental data of the comparison of our graph-based algorithm against MAPPR
 - [full_algorithms/](full_algorithms/) - folder containing all our algorithms (graph- and hypergraph-based versions, both sequential and parallel) as well as MAPPR
 - [full_plotting/](full_plotting/) - folder containing scripts to plot all charts and the table contained in our paper
 - [build_all.sh](build_all.sh) - script to automatically compile the algorithms contained in algorithms/
 - [run_sample.sh](run_sample.sh) - script to generate, run, and analyze a sample of our experiments
 - [clean_all.sh](clean_all.sh) - script to clean the binary files of the algorithms contained in algorithms/

Description
-----------

This folder contains a sample graph ([database/](database/)), a simplified code base containing just the sequential graph-based version of our algorithm and MAPPR ([algorithms/](algorithms/)), and a set of scripts to generate sample experiments ([experiments/](experiments/)). These sample experiments can be easily executed as shown below.
Additionally, this folder contains the full code used in the paper ([full_algorithms/](full_algorithms/)), the full experimental data for comparisons agains MAPPR ([full_results/](full_results/)), and full scripts to generate the charts and the table contained ([full_plotting/](full_plotting/)) in the paper.
Please follow the instructions below to compile the code, run the sample experiments, and plot the charts.


Requirements to Compile Algorithms
-----------

 - A 64-bit Linux operating system.
 - A modern, ![C++17](https://img.shields.io/badge/C++-17-blue.svg?style=flat)-ready compiler such as `g++` version 7 or higher or `clang` version 11.0.3 or higher.
 - The cmake build system (>= 3.16).
 - The Boost - Program Options library and the boost header files (>= 1.48).
 - The Intel Thread Building Blocks library (TBB)
 - The Portable Hardware Locality library (hwloc)
 - An MPI implementation, e.g., [OpenMPI](https://www.open-mpi.org/) or [MPICH](https://www.mpich.org)

Building Simplified Code Base (algorithms/)
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

Building Complete Code Base (full_algorithms/)
-----------

Enter the folder full_algorithms/ and follow the instructions contained in README.md

Plotting Full Charts (full_plotting/)
-----------

Enter the folder full_plotting/ and follow the instructions contained in README

