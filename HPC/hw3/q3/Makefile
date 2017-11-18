simple: matrix_multiplication.o
	g++ matrix_multiplication.o -fopenmp -o simple

reduction: matrix_multiplication_reduction.o
	g++ matrix_multiplication_reduction.o -fopenmp -o reduction

matrix_multiplication.o: matrix_multiplication.cpp
	g++ -c matrix_multiplication.cpp -fopenmp

matrix_multiplication_reduction.o: matrix_multiplication_reduction.cpp
	g++ -c matrix_multiplication_reduction.cpp -fopenmp

clean: 
	rm *.o simple reduction

