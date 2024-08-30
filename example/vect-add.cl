__kernel void litmus_test(__global uint *a, __global float *b, __global float *c) {
	uint id = get_global_id(0);
	c[id] = (float) a[id] + b[id];
}
