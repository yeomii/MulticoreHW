#!/usr/bin/env python

import os, math, sys

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "Usage: ./eval experiment_name"
		sys.exit()

time_records = {}

for f in os.listdir(os.getcwd()):
	if f.startswith("task") and f.endswith(".stdout"):
		fin = open(f, 'r')
		l = fin.readline().split(' ')
		t = float(l[3])
		l = fin.readline().split(' ')
		b = int(l[3])
		if str(b) in time_records:
			time_records[str(b)].append(t)
		else:
			time_records[str(b)] = [t]
		fin.close()
"""
sum = 0
for i in time_records:
	sum += i

avg = sum/len(time_records)

stdev = 0
for i in time_records:
	stdev += pow(i - avg, 2)
stdev /= (len(time_records) - 1)
stdev = math.sqrt(stdev)
"""
fout = open("%s.out"%sys.argv[1], "w")
for k in sorted(time_records.keys(), key=int):
	lst = time_records[k]
	sum = 0
	for record in lst:
		sum += record
	avg = sum / len(lst)
	stdev = 0
	for i in lst:
		stdev += pow(i - avg, 2)
	stdev /= (len(lst) - 1)
	stdev = math.sqrt(stdev)

	fout.write("%s, %f, %f\n" % (k, avg, stdev))
fout.close()
"""
fout.write("# of experiment : %d\n" % len(time_records))
fout.write("avg %f\n" % avg)
fout.write("stdev %f" % stdev)
"""
