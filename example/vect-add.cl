__kernel void litmus_test(__global uint *a, __global uint *b, __global uint *c) {
	uint id = get_global_id(0);
	c[id] = a[id] + b[id];
}
