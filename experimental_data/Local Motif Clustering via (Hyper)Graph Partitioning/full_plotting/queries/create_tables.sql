CREATE TABLE IF NOT EXISTS "times"(
  "index" INTEGER,
  "seednode" INTEGER,
  "seed" INTEGER,
  "graph" TEXT,
  "algorithm" TEXT,
  "time_clus" REAL,
  "time_enum" REAL
);

CREATE TABLE IF NOT EXISTS "results"(
  "index" INTEGER,
  "seednode" INTEGER,
  "seed" INTEGER,
  "graph" TEXT,
  "algorithm" TEXT,
  "motif_conduc" REAL,
  "cluster_size" INTEGER
);
