#!/usr/bin/env python

import sys
from subprocess import Popen

def repeat():
	processes = []

	for nodes in [1, 2, 4]:
		for slots in [1, 2, 4, 8, 16]:
			processes.append(Popen(["thorq", "--add", "--mode", "mpi", "--nodes", str(nodes), "--slots", str(slots),
															"--name", "result_%d_%d"%(nodes, slots), "./mat_mul"]))

	map (lambda p: p.wait(), processes)

repeat()
