#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include "../lib/copyjit/src/copyjit.h"

void compile(ctx_t *ctx)
{
	compile_start(ctx);

	/* arg in x */
	void *fib = compile_xle2(ctx);
	reloc_t recurse = compile_placeholder(ctx, compile_bani);

	compile_liom1(ctx);
	compile_addxo(ctx);
	compile_pushx(ctx);

	compile_call(ctx, (unsigned long)fib);

	// get previous n into o
	compile_popo(ctx);

	// store return value in x
	compile_pushx(ctx);

	// get new n
	compile_lixm1(ctx);
	compile_addxo(ctx);
	compile_call(ctx, (unsigned long)fib);
	compile_popo(ctx); // get previous sum in o
	compile_addxo(ctx); // calculate new result plus old result in x
	compile_end(ctx); // 'return'

	void *bottom = compile_fast_lix(ctx, 1);
	compile_patch(recurse, bottom);
	compile_end(ctx); // 'return'

	compile_finish(ctx);
}

int main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(stderr, "Usage: %s compile_num fib_num\n", argv[0]);
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
	size_t result = run(&ctxs[0], 0, run_num, 0, 0);
	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running fib for n = %lu took %fs with res %lu\n", 
			run_num, run_time_total, result);

	/*
	for(size_t i = 0; i < compile_num; ++i)
		compile_destroy(&ctxs[i]);
		*/

	return 0;
}
