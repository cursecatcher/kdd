# change this to the folder where gatb-core is
# GATB=/home/cursed/gitgud/gatb-core/gatb-core
GATB		=$(PWD)/gatb-core

GATB_LIB	=$(GATB)/build/lib

DEBUG		= -g -ggdb 
CXXFLAGS	= -I$(GATB)/src  -I$(GATB)/build/include -I$(GATB)/thirdparty -I./meddly/include
LDFLAGS		= -L$(GATB_LIB) -lgatbcore  -lpthread -lhdf5 -lz -ldl meddly/lib/libmeddly.a # -lmeddly #-static -std=c++0x
SETTINGS 	= -std=c++0x  $(DEBUG)  #-O3 

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%,$(SRCS))

all: $(OBJS)

clean:
	rm -fv $(OBJS) *.o 

# %: %.o 
# 	$(CXX) -o $@ $^ $(LDFLAGS) $(SETTINGS)

# %.o: %.cpp 
# 	$(CXX) -c $^ $(CXXFLAGS) $(SETTINGS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@  $(LDFLAGS) $(SETTINGS)

