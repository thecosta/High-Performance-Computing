# environment
GCC := mpiCC

# Compiler flags:
GccFLAGS = -fopenmp -O3 -std=c++11 -Wno-deprecated

# The build target executable:
TARGET = motion

all: build

build: $(TARGET)

$(TARGET): src/main.cpp
	$(GCC) $(GccFLAGS) -o $@ src/main.cpp -fopenmp

run: $(TARGET)
	mpirun -np 64 -prot -TCP ./$(TARGET) 

clean:
	rm $(TARGET)
