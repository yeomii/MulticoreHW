
CXX=g++
CXXFLAGS=-Wall -g


LIBS = -lrt
LDFLAGS = ${LIBS}


all: seq pthread

.PHONY: all seq pthread clean test


seq: kmeans_seq

kmeans_seq: kmeans_seq.o kmeans_main.o
	${CXX} $^ -o $@ ${LDFLAGS}


pthread: kmeans_pthread

kmeans_pthread: kmeans_pthread.o kmeans_main.o
	${CXX} ${CXXFLAGS} $^ -o $@ ${LDFLAGS} -lpthread

run_all:
	./gen_data.py centroid 64 centroid.point
	./gen_data.py data 65536 data.point 64
	thorq --add kmeans_seq centroid.point data.point result_seq.class final_centroid_seq.point 1024
	thorq --add kmeans_pthread centroid.point data.point result_par.class final_centroid_par.point 1024 4

run_seq:
	./gen_data.py centroid 64 centroid.point
	./gen_data.py data 65536 data.point 64
	thorq --add kmeans_seq centroid.point data.point result_seq.class final_centroid_seq.point 1024

run:
	./gen_data.py centroid 64 centroid.point
	./gen_data.py data 65536 data.point 64
	thorq --add kmeans_pthread centroid.point data.point result_par.class final_centroid_par.point 1024 4
	
plot_all:
	./plot_data.py result final_centroid_seq.point data.point result_seq.class seq.png
	./plot_data.py result final_centroid_par.point data.point result_par.class par.png


clean:
	rm -f kmeans_seq kmeans_pthread kmeans_main.o kmeans_seq.o kmeans_pthread.o centroid.point data.point final_centroid.point result.class task*
