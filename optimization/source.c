#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>

void update_cl(float *in, float *out) {
	
	// Load the program source
	char* program_text = load_source_file("kernel.cl");

	// Create the program
	cl_program program;
	program = clCreateProgramWithSource(opencl_context, 1,
							(const char**)&program_text, NULL, &error);
	
	// Compile the program and check for errors
	clBuildProgram(program, 1, &device, NULL, NULL, NULL);

	// Create the computation kernel
	cl_kernel kernel = clCreateKernel(program, "update", &error);

	// Create the data objects
	cl_mem in_buffer, out_buffer;
	in_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE_BYTES,
															NULL, &error);
	out_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, SIZE_BYTES,
															NULL, &error);
	
	// Copy data to the device
	clEnqueueWriteBuffer(queue, in_buffer, CL_FALSE, 0, SIZE_BYTES, in, 0,
															NULL, NULL);
	clEnqueueWriteBuffer(queue, out_buffer, CL_FALSE, 0, SIZE_BYTES, out, 0,
															NULL, NULL);
	
	// Set the kernel arguments
	clSetKernelArg(kernel, 0, sizeof(in_buffer), &in_buffer);
	clSetKernelArg(kernel, 1, sizeof(out_buffer), &out_buffer);

	// Enqueue the kernel
	size_t global_dimensions[] = {SIZE, SIZE, 0};
	clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_dimensions,
													NULL, 0, NULL, NULL);

	// Enqueue a read to get the data back
	clEnqueueReadBuffer(queue, out_buffer, CL_FALSE, 0, SIZE_BYTES, out, 0,
																NULL, NULL);
	
	// Wait for it to finish
	clFinish(queue);

	// Cleanup
	clReleaseMemObject(out_buffer);
	clReleaseMemObject(in_buffer);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	free(program_text);
}

int main (int argc, const char * argv[]) {
	float range = BIG_RANGE;
	float *in, *out;

	// ======== Initialize
	create_data(&in, &out);
	
	// ======== Setup OpenCL
	setup_cl(argc, argv, &device, &context, &queue);

	// ======== Compute
	while (range > LIMIT) {
		
		// Calculation
		update_cl(in, out);

		// Compute Range
		range = find_range(out, SIZE*SIZE);

		iterations++;
		swap(&in, &out);

		printf("Iteration %d, range=%f.\n", iterations, range);
	}
}
