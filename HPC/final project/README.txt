Files for High-Performance Computing final project. 
Hybrid Programming with MPI and OpenMP on motion detection. 
Video has 485 frames. 448 are used to run on 64 processes. 
Each frame has 288x384 pixels. 
MPI runs 64 processes. Each process is assigned 7 frames. 
Each process runs 2 threads when reading and writing the data. 

Files:
	.discovery/ => has all source files including video frames
		.discovery/enter/* => original video frames
		.discovery/enter_text/* => original video frames saved as decimal gray values
		.discovery/filtered_text/* => filtered video frames saved as 0s or 1s. 
		.discovery/Makefile => execute ‘make run’ to compile source files.
		.discovery/src => source file and old versions
			.discovery/src/trials/ => abandoned attempts of hybrid implementation
	.enter_motion.gif => gif created from read_original_files.m
	.motion.gif => gif created from read_filtered_files.m
	.project.pptx => project power point presentation
	.read_filtered_files.m => create gif of filtered frames
	.read_original_files.m => create gif of original frames
	.README => this