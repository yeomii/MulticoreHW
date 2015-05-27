
CXX=g++
CXXFLAGS=-Wall -O3 -fopenmp


LIBS = -lrt
LDFLAGS = ${LIBS}


all: seq omp

.PHONY: all seq omp clean test


seq: kmeans_seq

kmeans_seq: kmeans_seq.o kmeans_main.o
	${CXX} $^ -o $@ ${LDFLAGS}


omp: kmeans_omp

kmeans_omp: kmeans_omp.o kmeans_main.o
	${CXX} -fopenmp $^ -o $@ ${LDFLAGS}

run_all: 
	thorq --add kmeans_seq centroid.point data.point result_seq.class final_centroid_seq.point 1024
	thorq --add kmeans_omp centroid.point data.point result_omp.class final_centroid_omp.point 1024 4

run_seq:
	thorq --add kmeans_seq centroid.point data.point result_seq.class final_centroid_seq.point 1024

run: 
	thorq --add kmeans_omp centroid.point data.point result_omp.class final_centroid_omp.point 1024 4
	
data:
	./gen_data.py centroid 64 centroid.point
	./gen_data.py data 65536 data.point 64

plot:
	./plot_data.py result final_centroid_omp.point data.point result_omp.class omp.png
	./plot_data.py result final_centroid_seq.point data.point result_seq.class seq.png

clean:
	rm -f kmeans_seq kmeans_omp kmeans_main.o kmeans_seq.o kmeans_omp.o centroid.point data.point final_centroid.point result.class task*
