#!/usr/bin/env python

import sys
from subprocess import Popen

def repeat():
	processes = []
	
	n = 0

	for ws in [16, 32, 64, 128, 256]:
		for mode in ['c', 'g']:
			'''
			processes.append(Popen(["./gen_data.py", "centroid", "16", "centroid%d.point"%n]))
			processes.append(Popen(["./gen_data.py", "data", "1048576", "data%d.point"%n, "16"]))
			'''
			processes.append(Popen(["thorq", "--add", "--device", "gpu",
															"./kmeans_cl", "centroid%d.point"%n, "data%d.point"%n,
															"result%d.class"%n, "final_centroid%d.point"%n, "1024", str(ws), mode]))
			n += 1
	map (lambda p: p.wait(), processes)

repeat()
