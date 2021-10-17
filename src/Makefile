# change this to the folder where gatb-core is
# GATB=/home/cursed/gitgud/gatb-core/gatb-core
GATB		=$(PWD)/gatb-core

GATB_LIB	=$(GATB)/build/lib
GATB_LIB=$(GATB)/build/lib

CXXFLAGS =  -std=c++0x -O3 -I./meddly/include  #-I$(GATB)/src  -I$(GATB)/build/include -I$(GATB)/thirdparty 
LDFLAGS=  meddly/lib/libmeddly.a   -lpthread -lz -std=c++0x -ldl #-L$(GATB_LIB) -lgatbcore -lhdf5   #-static


SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%,$(SRCS))

all: $(OBJS)

clean:
	rm -fv $(OBJS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@  $(LDFLAGS)
