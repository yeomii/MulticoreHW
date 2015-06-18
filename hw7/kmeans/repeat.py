#!/usr/bin/env python

import sys
from subprocess import Popen

def repeat():
  processes = []
  
  n = 0

  for node in [1, 2, 4]:
    for slot in [1, 2, 4, 8, 16]:
      n+=1
      processes.append(Popen(["thorq", "--add", "--mode", "mpi", "--nodes", str(node), "--slots", str(slot), 
                              "--name", "result_%d_%d" % (node, slot), 
                              "./kmeans_mpi", "centroid%d.point"%n, "data%d.point"%n,
                              "result%d.class"%n, "final_centroid%d.point"%n, "1024"]))
  map (lambda p: p.wait(), processes)

repeat()
