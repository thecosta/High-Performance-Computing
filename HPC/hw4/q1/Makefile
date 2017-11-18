inverse: inversion.o
	g++ inversion.o -fopenmp -O3 -o inversion

inversion.o: inversion.cpp
	g++ -c inversion.cpp -fopenmp -O3

clean: 
	rm *.o inversion
