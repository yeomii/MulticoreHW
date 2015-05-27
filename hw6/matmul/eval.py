#!/usr/bin/env python

import sys, os, re

recs = { }
failure = 0

for f in os.listdir(os.getcwd()):
	if f.startswith("task") and f.endswith("stdout"):
		fin = open(f, 'r')
		loop = None
		th = 0
		rec = -1
		for l in fin:
			if l.startswith("Number"):
				th = int(l.split(' ')[4])
			elif l.startswith("parallel"):
				loop = l.split(' ')[3][0]
			elif l.startswith("Time"):
				rec = float(l.split(' ')[3])

		if rec < 0:
			failure += 1
		else:
			if loop not in recs:
				recs[loop] = {}
			if th not in recs[loop]:
				recs[loop][th] = []
			recs[loop][th].append(rec)

fout = open("results", 'w')
#fout.write("failure : %d\n" % failure)
for loop in sorted(recs.keys()):
	for th in sorted(recs[loop].keys()):
		lst = recs[loop][th]
		fout.write("%s, %d, %f, %f %d\n" % (loop, th, min(lst), sum(lst) / len(lst), len(lst)))
fout.close()

