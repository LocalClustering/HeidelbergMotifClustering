# Heidelberg Motif Clustering [![Codacy Badge](https://app.codacy.com/project/badge/Grade/93d164647e654bf2a814f5101fdf3481)](https://www.codacy.com/gh/LocalClustering/HeidelbergMotifClustering/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=LocalClustering/HeidelbergMotifClustering&amp;utm_campaign=Badge_Grade)[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2FLocalClustering%2FHeidelbergMotifClustering.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2FLocalClustering%2FHeidelbergMotifClustering?ref=badge_shield)[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A widely-used operation on graphs is local clustering, i.e., extracting a well-characterized community around a seed node without the need to process the whole graph.  Recently local motif clustering has been proposed: it looks for a local cluster based on the distribution of motifs.  Since this local clustering perspective is relatively new, most approaches proposed for it are extensions of statistical and numerical methods previously used for edge-based local clustering, while the available combinatorial approaches are still few and relatively simple.  In this work, we build a hypergraph and a graph model which both represent the motif-distribution around the seed node. We solve these models using  sophisticated combinatorial algorithms designed for (hyper)graph partitioning as well as an adapted version of the max-flow quotient-cut improvement algorithm (MQI).

This repository is associated with the following papers:

 - "**Local Motif Clustering via (Hyper)Graph Partitioning**", which has been published as an extended abstract at [SoCS 2022](https://ojs.aaai.org/index.php/SOCS/article/view/21779) and as a full paper at [SIAM ALENEX 2023](https://doi.org/10.1137/1.9781611977561.ch9).
Additionally, you can find a [technical report](https://arxiv.org/pdf/2205.06176.pdf) and our SoCS 2022 [poster](misc/SoCS_Poster.pdf).

 - "**Faster Local Motif Clustering via Maximum Flows**", which has been accept for publication as a full paper at ESA 2023.
Here you can find a [technical report](https://arxiv.org/pdf/2301.07145.pdf).

If you publish results using our algorithms, please acknowledge our work by citing the corresponding papers:

```
@inproceedings{LocMotifClusHyperGraphPartition2023,
  author    = {Adil Chhabra and
               Marcelo Fonseca Faraj and
               Christian Schulz},
  title     = {Local Motif Clustering via (Hyper)Graph Partitioning},
  booktitle = {Symposium on Algorithm Engineering and Experiments (ALENEX 23), January 22-23, 2023},
  publisher = {{SIAM}},
  doi       = {10.1137/1.9781611977561.ch9},
  year      = {2023}
}
```

```
@inproceedings{FastLocMotifClusMaxFlows2023,
  author    = {Adil Chhabra and
               Marcelo Fonseca Faraj and
               Christian Schulz},
  title     = {Faster Local Motif Clustering via Maximum Flows},
  booktitle = {European Symposium on Algorithms ({ESA} 2023), September 4-6, 2023},
  publisher = {Schloss Dagstuhl - Leibniz-Zentrum f{\"{u}}r Informatik},
  series    = {LIPIcs},
  year      = {2023}
}
```

Repository Structure
-----------

This repository contains the following subfolders and files:

 - [LMCHGP/](LMCHGP/) - Graph-based version of our algorithm proposed in "**Local Motif Clustering via (Hyper)Graph Partitioning**".
 - [SOCIAL/](SOCIAL/) - Clique-expansion version of our algorithm proposed in "**Faster Local Motif Clustering via Maximum Flows**".
 - [experimental_data/](experimental_data/) - Full experimental data, code and scripts.


Requirements to Compile Algorithms
-----------

 - A 64-bit Linux operating system.
 - A modern, ![C++17](https://img.shields.io/badge/C++-17-blue.svg?style=flat)-ready compiler such as `g++` version 7 or higher or `clang` version 11.0.3 or higher.
 - The cmake build system (>= 3.16).
 - The Boost - Program Options library and the boost header files (>= 1.48).
 - The Intel Thread Building Blocks library (TBB)
 - The Portable Hardware Locality library (hwloc)
 - An MPI implementation, e.g., [OpenMPI](https://www.open-mpi.org/) or [MPICH](https://www.mpich.org)

Cloning Repository
-----------

Clone this repository with the following command to include all necessary submodules:

```console
git clone --depth=2 --recursive https://github.com/LocalClustering/HeidelbergMotifClustering.git
```

Building
-----------

First of all, install the required depedencies using the following command (for linux):

```console
sudo apt-get install libboost-program-options-dev libnuma-dev numactl libhwloc-dev moreutils linux-tools-common linux-tools-generic libtbb-dev openmpi-bin
```

Then, just run run the following command inside the folder of the desired algorithm:

```console
./compile.sh
```

Running 
-----------

For running the algorithms, see instructions in the respective folders

```console
./run_sample.sh
```

