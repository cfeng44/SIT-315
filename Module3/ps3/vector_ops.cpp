#include <stdio.h>
#include <stdlib.h>
#include <OpenCL/opencl.h>

#define PRINT 1

int SIZE = 8; // Vector size.
int *v; // Vector for data on GPU.
int err; 

// --- OpenCL objects ---

cl_mem bufV; // The data for the GPU.
cl_device_id device_id; // Used to keep track of the device.
cl_context context; // For containing devices and kernels with memory and syncing.
cl_program program; // A program has kernels.
cl_kernel kernel; // The function to be run.
cl_command_queue queue; // Data structure holding instructions.
cl_event event = NULL; // cl_event's allow us to sync operations in contexts.

// --- Function Declarations (implemented after main() function in this file) ---

// Note: I have changed a few functions so that the accept a variable by
// reference so that global variables don't have to be used. Most of the 
// time this is not necessary because a lot of the OpenCL types are
// typedefs for pointers. However, to be safe and not be required to
// check every type the functions pass by reference with the ampersand 
// character.

cl_device_id create_device();
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);
void setup_kernel_memory(cl_mem &buf);
void copy_kernel_args();
void free_memory();
void setup_openCL_device_context_queue_kernel(cl_kernel &kernel, char *filename,
                                              char *kernel_name);

// These two are unrelated to OpenCl and are just helper functions.

void init(int *&A, int size);
void print(int *A, int size);

int main(int argc, char **argv) 
{
    // User can choose their own SIZE value.
    if (argc > 1) 
    {
        SIZE = atoi(argv[1]);
    }
    init(v, SIZE);

    // This is a single element array with its element the same size as the 
    // vector.
    size_t global[1] = {(size_t)SIZE};

    // Initial vector
    print(v, SIZE);

    setup_openCL_device_context_queue_kernel(kernel, (char *)"./vector_ops.cl",
                                             (char *)"square_magnitude");
    setup_kernel_memory(bufV);
    copy_kernel_args();

    // This call enqueues the specified kernel and associates its execution on the
    // device with a cl_event. The host thread then waits until this event status
    // is appropriate.
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event);

    // The reason we waited above for the event to finish was so that we 
    // read the updated buffer. We then read from buffer into the pointer 
    // to our vector, v.
    clEnqueueReadBuffer(queue, bufV, CL_TRUE, 0, SIZE * sizeof(int), &v[0], 0,
                        NULL, NULL);

    // Result vector
    print(v, SIZE);

    // Frees memory for device, kernel, queue, etc.
    // you will need to modify this to free your own buffers.
    free_memory();
}

void init(int *&A, int size)
{
    A = (int *)malloc(sizeof(int) * size);

    for (long i = 0; i < size; i++)
    {
        A[i] = rand() % 100; // any number less than 100
    }
}

void print(int *A, int size)
{
    if (PRINT == 0)
    {
        return;
    }

    if (PRINT == 1 && size > 15)
    {
        for (long i = 0; i < 5; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
        printf(" ..... ");
        for (long i = size - 5; i < size; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
    }
    else
    {
        for (long i = 0; i < size; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
    }
    printf("\n----------------------------\n");
}

void free_memory(cl_mem &buf, cl_kernel &, )
{
    // Free the buffers
    clReleaseMemObject(bufV);

    // Free opencl objects
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(v);
}


void copy_kernel_args()
{
    clSetKernelArg(kernel, 0, sizeof(int), (void *)&SIZE);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufV);

    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

void setup_kernel_memory(cl_mem &buf)
{
    buf = clCreateBuffer(context, CL_MEM_READ_WRITE, SIZE * sizeof(int), NULL,
                         NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, buf, CL_TRUE, 0, SIZE * sizeof(int), &v[0], 0,
                         NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(cl_kernel &kernel, char *filename, 
                                              char *kernel_name)
{
    device_id = create_device();

    cl_int err;
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0)
    {
        perror("Couldn't create a context");
        exit(1);
    }

    program = build_program(context, device_id, filename);

    queue = clCreateCommandQueue(context, device_id, 0, &err);
    if (err < 0)
    {
        printf("ehy");
        perror("Couldn't create a command queue");
        exit(1);
    }


    kernel = clCreateKernel(program, kernel_name, &err);
    if (err < 0)
    {
        perror("Couldn't create a kernel");
        printf("error =%d", err);
        exit(1);
    }
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename)
{

    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;

    // Read program file and place content into buffer
    program_handle = fopen(filename, "r");
    if (program_handle == NULL)
    {
        perror("Couldn't find the program file");
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
    if (err < 0)
    {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    // Build program: 
    // The fourth parameter accepts options that configure the compilation. 
    // These are similar to the flags used by gcc. For example, you can 
    // define a macro with the option -DMACRO=VALUE and turn off optimization 
    // with -cl-opt-disable.
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0)
    {

        // Find size of log and print to std output
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}

cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   // Identify a platform
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   // GPU
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      // CPU
      printf("GPU not found\n");
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}