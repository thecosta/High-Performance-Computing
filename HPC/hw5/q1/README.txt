README text file. 
    .input/             => contains sample images to use. 
    .src/               => contains source files.
        .src/main.cpp   => main execution file. 
        .src/sobel.cu   => CUDA file to run Sobel mask on input image. 
    .Makefie            => run 'make' to create binary file 'sobel'. 
                           example: ./sobel .input/lake.jpg output.jpg 
