#!/usr/bin/env python

import sys, os, re

recs = { }
failure = 0

for f in os.listdir(os.getcwd()):
  if f.startswith("result") and f.endswith("stdout"):
    fin = open(f, 'r')
    node = int(f.split('_')[1])
    slot = int(f.split('_')[2].split('.')[0])
    
    if node not in recs:
      recs[node] = {}
    if slot not in recs[node]:
      recs[node][slot] = {}

    for l in fin:
      if l.startswith("Time"):
        recs[node][slot]['time'] = float(l.split(' ')[3])
      elif l.startswith("Matrix"):
        recs[node][slot]['mat'] = float(l.split(' ')[3])
      elif l.startswith("Comm"):
        recs[node][slot]['comm'] = float(l.split(' ')[2])

fout = open("results", 'w')
#fout.write("failure : %d\n" % failure)
for node in sorted(recs.keys()):
  for slot in sorted(recs[node].keys()):
    tmp = recs[node][slot]
    fout.write("%d, %d, %f, %f, %f\n" % (node, slot, tmp['time'], tmp['mat'], tmp['comm']))

fout.close()

