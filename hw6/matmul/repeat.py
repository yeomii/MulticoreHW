#!/usr/bin/env python

import sys
from subprocess import Popen

def repeat():
	processes = []

	for mode in ['i', 'j']:
		for t in [4, 8, 16]:
			processes.append(Popen(["thorq", "--add", 
															"./mat_mul_" + mode, "-t", str(t)]))

	map (lambda p: p.wait(), processes)

repeat()
