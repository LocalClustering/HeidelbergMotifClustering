// localmotifclustermain.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "localmotifcluster.h"
#include<iostream>

int main(int argc, char* argv[]) {
  Env = TEnv(argc, argv, TNotify::StdNotify);
  Env.PrepArgs(TStr::Fmt("Local motif clustering. build: %s, %s. Time: %s",
       __TIME__, __DATE__, TExeTm::GetCurTm()));  
  TExeTm ExeTm;  
  Try

    std::string io_time;
    std::string enum_time;
    const bool IsDirected = 
    Env.GetIfArgPrefixBool("-d:", false, "Directed graph?");

  ProcessedGraph graph_p;
  if (IsDirected) {
    const TStr graph_filename =
      Env.GetIfArgPrefixStr("-i:", "C-elegans-frontal.txt", "Input graph file");
    const TStr motif =
      Env.GetIfArgPrefixStr("-m:", "triad", "Motif type");
    MotifType mt = ParseMotifType(motif, IsDirected);
    PNGraph graph;
    if (graph_filename.GetFExt().GetLc() == ".ngraph") {
      TFIn FIn(graph_filename);
      graph = TNGraph::Load(FIn);
    } else if (graph_filename.GetFExt().GetLc() == ".ungraph") {
      TExcept::Throw("Warning: input graph is an undirected graph!!");
    } else {
      graph = TSnap::LoadEdgeList<PNGraph>(graph_filename, 0, 1);
    }
    TSnap::DelSelfEdges(graph);
    io_time = std::string(ExeTm.GetTmStr()); 

    ExeTm = TExeTm();
    TExeTm ExeTmEnumeration;  
    graph_p = ProcessedGraph(graph, mt);
    enum_time = std::string(ExeTmEnumeration.GetTmStr()); 

  } else {

    const TStr graph_filename =
      Env.GetIfArgPrefixStr("-i:", "C-elegans-frontal.txt", "Input graph file");
    const TStr motif =
      Env.GetIfArgPrefixStr("-m:", "clique3", "Motif type");
    MotifType mt = ParseMotifType(motif, IsDirected);
    PUNGraph graph;
    if (graph_filename.GetFExt().GetLc() == ".ungraph") {
      TFIn FIn(graph_filename);
      graph = TUNGraph::Load(FIn);
    } else if (graph_filename.GetFExt().GetLc() == ".ngraph") {
      TExcept::Throw("Warning: input graph is a directed graph!!");
    } else {
      graph = TSnap::LoadEdgeList<PUNGraph>(graph_filename, 0, 1);
    }
    TSnap::DelSelfEdges(graph);
    io_time = std::string(ExeTm.GetTmStr()); 

    ExeTm = TExeTm();
    TExeTm ExeTmEnumeration;  
    graph_p = ProcessedGraph(graph, mt);
    enum_time = std::string(ExeTmEnumeration.GetTmStr()); 
  }

  const TInt seed =
    Env.GetIfArgPrefixInt("-s:", 1, "Seed");
  const TFlt alpha =
    Env.GetIfArgPrefixFlt("-a:", 0.98, "alpha");
  const TFlt eps =
    Env.GetIfArgPrefixFlt("-e:", 0.0001, "eps");
  const bool write_community =
    Env.GetIfArgPrefixBool("-w:", false, "write_community");
  const TStr filename =
    Env.GetIfArgPrefixStr("-o:", "community", "filename");
  const TStr map_filename =
    Env.GetIfArgPrefixStr("-M:", "", "map from metis format to edge-list");


  printf("\nio time: %s\n", io_time.c_str());
  printf("Total Enumeration Time = %s\n", enum_time.c_str());
  MAPPR mappr;
  mappr.computeAPPR(graph_p, seed, alpha, eps / graph_p.getTotalVolume() * graph_p.getTransformedGraph()->GetNodes());
  mappr.sweepAPPR(0); // the parameter 0 indicates that you get the global optimum cluster
  printf("Total Clustering Time: %s\n", ExeTm.GetTmStr());
  if (write_community) {
	  /* mappr.printProfile(); */
	  mappr.writeCommunity(filename, map_filename);
  }
  mappr.printConductance();
  printf("Size of cluster: %d\n", mappr.getCluster().Len());

  
  Catch
  /* printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), */
  /*  TSecTm::GetCurTm().GetTmStr().CStr()); */
  return 0;
}
