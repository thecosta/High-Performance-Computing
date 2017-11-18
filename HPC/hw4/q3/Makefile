#################
#
#
# Simple Makefile example for MPI on Discovery Cluster
# by Julian Gutierrez
# modified by Bruno Costa Rendon
#
#
#################

Compiler := mpiCC

# Remove function
RM = rm -f
 
# Compiler flags:
CompilerFLAGS = -fopenmp -O3 -Wno-deprecated

# -g    debugging information
# -Wall turns on most compiler warnings
debug: CompilerFLAGS += -DDEBUG -g -Wall
debug: all

# The build target executable:
TARGET = pi_estimation

all: build

build: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(Compiler) $(CompilerFLAGS) $^ -o $@ 
    
run: $(TARGET)
	mpirun -np 20 -prot -TCP ./$(TARGET) 
    
clean:
	$(RM) $(TARGET) *.o *.tar* *.core* *.err *.out
