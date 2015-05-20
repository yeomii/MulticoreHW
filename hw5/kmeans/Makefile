
CXX=g++
CXXFLAGS=-Wall -O3


LIBS = -lrt
LDFLAGS = ${LIBS}


all: seq cl

.PHONY: all seq cl clean test

opencl_util: opencl_util.o
	${CXX} -lOpenCL opencl_util.cpp -o $@

seq: kmeans_seq

kmeans_seq: kmeans_seq.o kmeans_main.o
	${CXX} $^ -o $@ ${LDFLAGS}


cl: kmeans_cl

kmeans_cl: kmeans_cl.o kmeans_main.o opencl_util.o
	${CXX} $^ -o $@ ${LDFLAGS} -lOpenCL

data:
	./gen_data.py centroid 16 centroid.point
	./gen_data.py data 1048576 data.point 16

run_all:
	thorq --add kmeans_seq centroid.point data.point result_seq.class final_centroid_seq.point 1024
	thorq --add --device gpu kmeans_cl centroid.point data.point result_cl.class final_centroid_cl.point 1024

run_seq:
	thorq --add kmeans_seq centroid.point data.point result_seq.class final_centroid_seq.point 1024

run: data
	thorq --add --device gpu kmeans_cl centroid.point data.point result_cl.class final_centroid_cl.point 1024 256 g

plot_all: plot_cl plot_seq

plot_cl:
	./plot_data.py result final_centroid_cl.point data.point result_cl.class result_cl.png

plot_seq:
	./plot_data.py result final_centroid_seq.point data.point result_seq.class result_seq.png

clean:
	rm -f kmeans_seq kmeans_cl kmeans_main.o kmeans_seq.o kmeans_cl.o centroid.point data.point final_centroid.point result.class task*
