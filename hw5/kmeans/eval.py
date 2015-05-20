#!/usr/bin/env python

import sys, os, re

recs = {}
failure = 0

for f in os.listdir(os.getcwd()):
	if f.startswith("task") and f.endswith("stdout"):
		fin = open(f, 'r')
		ws = 0
		device = "gpu"
		rec = -1
		for l in fin:
			if l.startswith("work_group_size"):
				ws = int(l.split(' ')[2])
			elif l.startswith("device"):
				device = re.sub(r'\W+', '', l.split(' ')[2])
			elif l.startswith("Time"):
				rec = float(l.split(' ')[2])

		if rec < 0:
			failure += 1
		else:
			if device not in recs:
				recs[device] = {}
			if ws not in recs[device]:
				recs[device][ws] = []
			recs[device][ws].append(rec)

fout = open("execution.result", 'w')
fout.write("failure : %d\n" % failure)
for device in sorted(recs.keys()):
	for ws in sorted(recs[device].keys()):
		lst = recs[device][ws]
		mean = sum(lst) / len(lst)
		fout.write("%s, %d, %f, %d\n" % (device, ws, mean, len(lst)))
fout.close()

