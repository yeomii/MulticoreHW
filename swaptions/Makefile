CXX = g++
DEF = 
INCLUDE =
CXXFLAGS = -std=c++0x -O3

EXEC = swaptions 

OBJS= timers.o \
	CumNormalInv.o MaxFunction.o RanUnif.o nr_routines.o icdf.o \
	HJM_SimPath_Forward_Blocking.o HJM.o HJM_Swaption_Blocking.o  \
	HJM_Securities.o

ifdef version
  ifeq "$(version)" "pthread" 
    DEF := $(DEF) -DENABLE_THREADS
    CXXFLAGS := $(CXXFLAGS) -pthread
  endif
  ifeq "$(version)" "cpu"
    OBJS := timers.o opencl_util.o opencl_Securities.o
    EXEC := cpu_swaptions
    CXXFLAGS := $(CXXFLAGS) -lOpenCL -DCPU
  endif
  ifeq "$(version)" "gpu"
    OBJS := timers.o opencl_util.o opencl_Securities.o
    EXEC := gpu_swaptions
    CXXFLAGS := $(CXXFLAGS) -lOpenCL
  endif
  ifeq "$(version)" "mpi"
    OBJS := timers.o opencl_util.o opencl_Securities.o
    EXEC := mpi_swaptions
    CXX = mpic++
    CXXFLAGS := $(CXXFLAGS) -lOpenCL -DMPI -DOMPI_SKIP_MPICXX 
  endif
  ifeq "$(version)" "test"
    OBJS := timers.o test.o
    EXEC := test_swaption
    CXXFLAGS := $(CXXFLAGS) -lm
  endif
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEF) $(OBJS) $(INCLUDE) $(LIBS) -o $(EXEC)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEF) -c $*.cpp -o $*.o

.c.o:
	$(CXX) $(CXXFLAGS) $(DEF) -c $*.c -o $*.o

clean:
	rm -f $(OBJS) $(EXEC) mpi_swaptions gpu_swaptions cpu_swaptions opencl_Securities.o opencl_util.o

