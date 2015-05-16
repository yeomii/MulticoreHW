#!/usr/bin/env python

import sys
from subprocess import Popen

def repeat():
	N = 20
	commands = []
	#processes = [Popen(["./gen_data.py", "data", str((1+num)*16384), "data%d.point" % (num+1), "64"]) for num in range(N)]
	#map (lambda p: p.wait(), processes)
	
	for i in range(N):
		command = "thorq --add kmeans_pthread centroid.point data%d.point result_par%d.class final_centroid_par%d.point 1024 %d" % (i+1, i+1, i+1, i+1)
		commands.append(command.split(' '))
	processes = [Popen(commands[num % N]) for num in range(N*2)]
	map (lambda p: p.wait(), processes)

if __name__ == "__main__":
	repeat()
