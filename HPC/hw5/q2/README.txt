README text file. 
    .input/                 => contains sample images to use. 
    .src/                   => contains source files.
        .src/main.cpp       => main execution file. 
        .src/classify.cu    => CUDA file to output image with selected ranged of gray colors,
                               and console histogram representation. 
    .Makefie                => run 'make' to create binary file 'classify'. 
                               example: ./classify .input/lake.jpg output.jpg 50 100
