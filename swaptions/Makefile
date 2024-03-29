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
  ifeq "$(version)" "tbb"
    DEF := $(DEF) -DENABLE_THREADS -DTBB_VERSION
    LIBS := $(LIBS) -ltbb
  endif
 ifeq "$(version)" "cpu"
    OBJS := timers.o opencl_util.o opencl_Securities.o
    EXEC := cpu_swaptions
    DEF := $(DEF) -DCPU
    CXXFLAGS := $(CXXFLAGS) -lOpenCL
  endif
  ifeq "$(version)" "gpu"
    OBJS := timers.o opencl_util.o opencl_Securities.o
    EXEC := gpu_swaptions
    CXXFLAGS := $(CXXFLAGS) -lOpenCL
  endif
  ifeq "$(version)" "mpi"
    OBJS := timers.o opencl_util.o opencl_Securities.o
    EXEC := mpi_swaptions
    DEF := $(DEF) -DMPI -DOMPI_SKIP_MPICXX
    CXX = mpic++
    CXXFLAGS := $(CXXFLAGS) -lOpenCL
  endif
  ifeq "$(version)" "snucl"
    OBJS := timers.o opencl_util.o opencl_Securities.o
    EXEC := snucl_swaptions
    DEF := $(DEF) -DSNUCL -DOMPI_SKIP_MPICXX
    CXX = mpic++
    INCLUDE := $(INCLUDE) -I$(SNUCLROOT)/inc
    LDFLAGS = -L$(SNUCLROOT)/lib -lsnucl_cluster -lOpenCL
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
	rm -f $(OBJS) $(EXEC) snucl_swaptions mpi_swaptions gpu_swaptions cpu_swaptions opencl_Securities.o opencl_util.o

