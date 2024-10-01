//ToDo: Add Comment (what is the purpose of this function? Where its going to get executed?)
__kernel void square_magnitude(const int size, __global int* v) {
    // Thread identifiers
    const int globalIndex = get_global_id(0);   
 
    printf("Kernel process index :(%d)\n ", globalIndex);

    v[globalIndex] = v[globalIndex] * v[globalIndex];
}
