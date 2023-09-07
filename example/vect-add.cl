__kernel void litmus_test(__global uint *a, __global double *b, __global double *c) {
	uint id = get_global_id(0);
	c[id] = (double) a[id] + b[id];
}
