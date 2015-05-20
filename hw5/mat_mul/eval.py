#!/usr/bin/env python

import sys, os, re

recs = { }
failure = 0

for f in os.listdir(os.getcwd()):
	if f.startswith("task") and f.endswith("stdout"):
		fin = open(f, 'r')
		gs = 0
		ws = 0
		device = "gpu"
		rec = -1
		for l in fin:
			if l.startswith("global_size"):
				gs = int(l.split(' ')[2])
			elif l.startswith("work_group_size"):
				ws = int(l.split(' ')[2])
			elif l.startswith("device"):
				device = re.sub(r'\W+', '', l.split(' ')[2])
			elif l.startswith("Time"):
				rec = float(l.split(' ')[3])

		if rec < 0:
			failure += 1
		else:
			if device not in recs:
				recs[device] = {}
			if gs not in recs[device]:
				recs[device][gs] = {}
			if ws not in recs[device][gs]:
				recs[device][gs][ws] = []
			recs[device][gs][ws].append(rec)

fout = open("execution.result", 'w')
fout.write("failure : %d\n" % failure)
for device in sorted(recs.keys()):
	for gs in sorted(recs[device].keys()):
		for ws in sorted(recs[device][gs].keys()):
			lst = recs[device][gs][ws]
			mean = sum(lst) / len(lst)
			fout.write("%s, %d, %d, %f, %f, %d\n" % (device, gs, ws, min(lst), mean, len(lst)))
fout.close()

