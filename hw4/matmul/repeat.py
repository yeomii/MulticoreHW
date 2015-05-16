#!/usr/bin/env python

import sys
from subprocess import Popen

def repeat():
	N = 5
	row = str(10)
	processes = [Popen(["thorq", "--add", "./mat_mul", "-v", "-r", row, "-c", "1"]) for num in range(N)]
	map (lambda p: p.wait(), processes)

if __name__ == "__main__":
	repeat()
