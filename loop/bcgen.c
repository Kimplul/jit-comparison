#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "bcode.h"

#define R0 0
#define R1 1
#define R2 2
#define R3 3

static void compile(struct compile_state *cs)
{
	init(cs);

	/* R0 = n */
	select_li(cs, R1, 0); /* sum = 0; */
	select_li(cs, R2, 0); /* i = 0; */
	select_li(cs, R3, 1); /* temp value for 1 */
	bcval_t top = label(cs); /* top */
	bcreloc_t r = select_jge(cs, R2, R0, 0); /* i >= n => out */

	select_add(cs, R1, R1, R2); /* sum += i */
	select_add(cs, R2, R2, R3); /* i += 1 */
	bcreloc_t j = select_jmp(cs, 0); /* jump to top */

	bcval_t out = label(cs);

	patch(cs, r, 0, out);
	patch(cs, j, 0, top);

	end(cs);
}

int main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(stderr, "Usage: %s compile_num loop_num\n", argv[0]);
		return -1;
	}

	size_t compile_num = strtoull(argv[1], 0, 0);

	struct compile_state *info = calloc(sizeof(struct compile_state),
			compile_num);

	clock_t t = clock();
	for(size_t i = 0; i < compile_num; ++i){
		compile(&info[i]);
	}
	t = clock() - t;

	double compile_time_total = ((double)t) / CLOCKS_PER_SEC;
	double compile_time_one = compile_time_total / compile_num;
	printf("Compilation for n = %lu took %fs (1/%f).\n",
			compile_num, compile_time_total, compile_time_one);

	size_t run_num = strtoull(argv[2], 0, 0);
	t = clock();

	ubcval_t ri[4] = {run_num};
	run(&info[0], ri, NULL);
	size_t result = ri[1];

	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running loop for n = %lu took %fs with res %lu\n",
			run_num, run_time_total, result);

	for(size_t i = 0; i < compile_num; ++i)
		destroy(&info[i]);

	free(info);

	return 0;
}
