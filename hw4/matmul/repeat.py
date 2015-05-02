#!/usr/bin/env python

import sys
from subprocess import Popen

def repeat():
	N = 200
	processes = [Popen(["thorq", "--add", "./mat_mul", "-b", str(int(num / 5) + 1), "-v"]) for num in range(N)]
	map (lambda p: p.wait(), processes)

if __name__ == "__main__":
	repeat()
