/*******************************************************************************
 * This file is part of KaHyPar.
 *
 * Copyright (C) 2019 Sebastian Schlag <tobias.heuer@kit.edu>
 *
 * KaHyPar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KaHyPar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KaHyPar.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "mt-kahypar/macros.h"
#include "mt-kahypar/definitions.h"
#include "mt-kahypar/partition/context.h"
#include "mt-kahypar/partition/metrics.h"
#include "mt-kahypar/io/hypergraph_io.h"

using namespace mt_kahypar;
namespace po = boost::program_options;

void readPartitionFile(const std::string& partition_file, PartitionedHypergraph& hypergraph) {
  ASSERT(!partition_file.empty(), "No filename for partition file specified");
  std::ifstream file(partition_file);
  if (file) {
    PartitionID part_id;
    HypernodeID node = 0;
    while (file >> part_id) {
      hypergraph.setOnlyNodePart(node, part_id);
      node++;
    }
    if (node != hypergraph.initialNumNodes()) {
      std::cerr << "Number of nodes doesnt match partition file size" << std::endl;
    }
    hypergraph.initializePartition();
    file.close();
  } else {
    std::cerr << "Error: File not found: " << std::endl;
  }
}

int main(int argc, char* argv[]) {
  Context context;

  po::options_description options("Options");
  options.add_options()
          ("hypergraph,h",
           po::value<std::string>(&context.partition.graph_filename)->value_name("<string>")->required(),
           "Hypergraph Filename")
          ("partition-file,b",
           po::value<std::string>(&context.partition.graph_partition_filename)->value_name("<string>")->required(),
           "Partition Filename")
          ("blocks,k",
           po::value<PartitionID>(&context.partition.k)->value_name("<int>")->required(),
           "Number of Blocks");

  po::variables_map cmd_vm;
  po::store(po::parse_command_line(argc, argv, options), cmd_vm);
  po::notify(cmd_vm);

  // Read Hypergraph
  Hypergraph hg = mt_kahypar::io::readHypergraphFile(context.partition.graph_filename, true);
  PartitionedHypergraph phg(context.partition.k, hg, parallel_tag_t());

  // Setup Context
  context.partition.epsilon = 0.03;
  context.setupPartWeights(hg.totalWeight());

  // Read Partition File
  readPartitionFile(context.partition.graph_partition_filename, phg);

  std::cout << "RESULT"
            << " graph=" << context.partition.graph_filename
            << " k=" << context.partition.k
            << " imbalance=" << metrics::imbalance(phg, context)
            << " cut=" << metrics::hyperedgeCut(phg)
            << " km1=" << metrics::km1(phg)
            << " soed=" << metrics::soed(phg) << std::endl;
  return 0;
}