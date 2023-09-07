#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include "../lib/copyjit/src/copyjit.h"

void compile(ctx_t *ctx)
{
	compile_start(ctx);

	// limit passed as an argument in o
	compile_fast_lix(ctx, 0); // total in x
	compile_fast_liy(ctx, 0); // iter in y
	void *top = compile_add(ctx); // iter + total in a
	compile_movxa(ctx); // move total to x

	compile_incy(ctx); // increment iter
	compile_subyo(ctx); // 'check' if we're above limit

	// branch to top if limit not reached
	reloc_t r = compile_placeholder(ctx, compile_bani);
	compile_patch(r, top);

	compile_end(ctx); // 'return'

	compile_finish(ctx);
}

int main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(stderr, "Usage: %s compile_num loop_num\n", argv[0]);
		return -1;
	}

	size_t compile_num = strtoull(argv[1], 0, 0);
	ctx_t *ctxs = calloc(sizeof(ctx_t), compile_num);

	clock_t t = clock();
	for(size_t i = 0; i < compile_num; ++i){
		compile(&ctxs[i]);
	}
	t = clock() - t;

	double compile_time_total = ((double)t) / CLOCKS_PER_SEC;
	double compile_time_one = compile_time_total / compile_num;
	printf("Compilation for n = %lu took %fs (1/%f).\n",
			compile_num, compile_time_total, compile_time_one);

	size_t run_num = strtoull(argv[2], 0, 0);
	t = clock();
	size_t result = run(&ctxs[0], 0, 0, 0, run_num);
	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running loop for n = %lu took %fs with res %lu\n", 
			run_num, run_time_total, result);

	/*
	for(size_t i = 0; i < compile_num; ++i)
		compile_destroy(&ctxs[i]);
		*/

	return 0;
}
