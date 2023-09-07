#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "bcode.h"

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define SP 4

static void compile(struct compile_state *cs)
{
	init(cs);
	bcval_t fib = label(cs);
	/* calling convention:
	 * R0 = argument register
	 * R1 = temp
	 * R2 = n
	 * R3 = sum
	 * R4 = stack pointer
	 *
	 * this is probably a pretty poor set of operations for a fibonacci
	 * sequence, but it does work
	 */
	select_li(cs, R1, 2);
	bcreloc_t recurse = select_jle(cs, R0, R1, 0); /* n <= 2 */

	/* fib(n - 1) */
	select_subi(cs, R0, R0, 1);
	select_mov(cs, R2, R0);
	/* store n */
	select_push(cs, R2);

	select_call(cs, fib.r);

	/* fetch previous n */
	select_pop(cs, R2);
	/* return value in R0, store it for sum */
	select_push(cs, R0);
	/* next n */
	select_subi(cs, R0, R2, 1);

	/* fib(n - 2) */
	select_call(cs, fib.r);

	/* get previous sum */
	select_pop(cs, R1);

	/* calculate final sum */
	select_add(cs, R0, R0, R1);
	select_ret(cs);
	end(cs);

	patch(cs, recurse, 0, label(cs));

	select_li(cs, R0, 1);
	select_ret(cs);

	end(cs);
}

int main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(stderr, "Usage: %s compile_num fib_num\n", argv[0]);
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

	/* register r4 is used as a stack pointer */
	/* slight hack, reserve a zero at bottom of stack for ret to know when
	 * to fall through and when to actually jump */
	ubcval_t stack[4096] = {0};
	ubcval_t ri[5] = {run_num, 0, 0, 0, (ubcval_t)&stack[1]};
	run(&info[0], ri, NULL);
	size_t result = ri[0];

	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running loop for n = %lu took %fs with res %lu\n",
			run_num, run_time_total, result);

	for(size_t i = 0; i < compile_num; ++i)
		destroy(&info[i]);

	free(info);

	return 0;
}
