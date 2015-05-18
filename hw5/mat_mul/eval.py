#!/usr/bin/env python

import sys, os

rec = {}
val = {}

for f in os.listdir(os.getcwd()):
	if f.startswith("task") and f.endswith("stdout"):
		fin = open(f, 'r')
		work_size = 0
		time = 0.0
		res = True
		for l in fin:
			if l.startswith("work"):
				work_size = int(l.split(' ')[2])
			elif l.startswith("Time"):
				time = float(l.split(' ')[3])
			elif l.startswith("c["):
				res = False
				break
		rec[work_size] = time
		val[work_size] = res
		fin.close()

fout = open("work_group_size_result", 'w')
for k in sorted(rec.keys()):
	fout.write("%d : %f, %s\n" % (k, rec[k], "T" if val[k] else "F"))
fout.close()

