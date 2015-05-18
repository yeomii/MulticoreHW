#!/usr/bin/env python

import sys
from subprocess import Popen

def repeat():
	processes = [Popen(["thorq", "--add", "--device", "gpu", "./mat_mul", "-n", "10000", "-g", "10000", "-w", str(num)]) 
		for num in range(2, 10, 1)]
	map (lambda p: p.wait(), processes)

repeat()
