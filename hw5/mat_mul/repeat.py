#!/usr/bin/env python

import sys
from subprocess import Popen

def repeat():
	processes = []

	for gs in [1024, 2048, 4096, 10000]:
		for ws in [2, 4, 8, 16]:
			for mode in ['-c', '']:
				process.append(Popen(["thorq", "--add", "--device", "gpu", 
															"./mat_mul", "-n", "10000", "-g", str(gs), "-w", str(ws), mode]))
	map (lambda p: p.wait(), processes)

repeat()
