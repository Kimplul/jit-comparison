#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgccjit.h>

typedef unsigned long (*loop_test_t) (unsigned long);

void compile(gcc_jit_context *ctxt)
{
	/*
	 * ulong loop_test(ulong n)
	 * {
	 * 	ulong sum = 0;
	 * 	for(ulong i = 0; i < n; ++i)
	 * 		sum += i;
	 *
	 * 	return sum;
	 * }
	 */

	/* initialize unsigned long type */
	gcc_jit_type *ulong_type =
		gcc_jit_context_get_type(ctxt, GCC_JIT_TYPE_UNSIGNED_LONG);

	/* function parameter */
	gcc_jit_param *n =
		gcc_jit_context_new_param(ctxt, NULL, ulong_type, "n");

	gcc_jit_param *params[1] = {n};

	gcc_jit_function *func =
		gcc_jit_context_new_function(ctxt, NULL,
				GCC_JIT_FUNCTION_EXPORTED,
				ulong_type,
				"loop_test",
				1, params, 0);

	/* build locals */
	gcc_jit_lvalue *sum =
		gcc_jit_function_new_local(func, NULL, ulong_type, "sum");
	gcc_jit_lvalue *i =
		gcc_jit_function_new_local(func, NULL, ulong_type, "i");

	/* prepare looping */
	gcc_jit_block *b_initial =
		gcc_jit_function_new_block(func, "initial");
	gcc_jit_block *b_loop_cond =
		gcc_jit_function_new_block(func, "loop_cond");
	gcc_jit_block *b_loop_body =
		gcc_jit_function_new_block(func, "loop_body");
	gcc_jit_block *b_after_loop =
		gcc_jit_function_new_block(func, "after_loop");

	/* initialize values */
	gcc_jit_block_add_assignment(b_initial, NULL, sum,
			gcc_jit_context_zero(ctxt, ulong_type));
	gcc_jit_block_add_assignment(b_initial, NULL, i,
			gcc_jit_context_zero(ctxt, ulong_type));

	/* connect initial block to condition block */
	gcc_jit_block_end_with_jump(b_initial, NULL, b_loop_cond);

	/* i < n */
	gcc_jit_block_end_with_conditional(b_loop_cond, NULL,
			gcc_jit_context_new_comparison(ctxt, NULL, GCC_JIT_COMPARISON_LT,
				gcc_jit_lvalue_as_rvalue(i),
				gcc_jit_param_as_rvalue(n)),
			b_loop_body,
			b_after_loop);

	/* sum += i */
	gcc_jit_block_add_assignment_op(b_loop_body, NULL,
			sum, GCC_JIT_BINARY_OP_PLUS,
			gcc_jit_lvalue_as_rvalue(i));

	/* ++i */
	gcc_jit_block_add_assignment_op(b_loop_body, NULL,
			i, GCC_JIT_BINARY_OP_PLUS,
			gcc_jit_context_one(ctxt, ulong_type));

	/* jump back up to condition */
	gcc_jit_block_end_with_jump(b_loop_body, NULL, b_loop_cond);

	/* return sum; */
	gcc_jit_block_end_with_return(b_after_loop, NULL,
			gcc_jit_lvalue_as_rvalue(sum));
}

int main(int argc, char **argv){
	if (argc != 3) {
		fprintf(stderr, "Usage: %s compile_num loop_num\n", argv[0]);
		return -1;
	}

	gcc_jit_context *ctxt = NULL;

	ctxt = gcc_jit_context_acquire();
	gcc_jit_context_set_int_option(ctxt, GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 2);
	gcc_jit_context_add_driver_option(ctxt, "-march=native");

	size_t compile_num = strtoull(argv[1], 0, 0);
	loop_test_t *loop_tests = calloc(sizeof(loop_test_t), compile_num);
	gcc_jit_result **results = calloc(sizeof(gcc_jit_result *), compile_num);

	clock_t t = clock();
	for(size_t i = 0; i < compile_num; ++i){
		gcc_jit_context *child_ctxt = NULL;
		child_ctxt = gcc_jit_context_new_child_context(ctxt);

		if(!child_ctxt){
			fprintf(stderr, "NULL ctxt");
			goto error;
		}

		compile(child_ctxt);
		results[i] = gcc_jit_context_compile(child_ctxt);

		if(!results[i]){
			fprintf(stderr, "NULL result");
			goto error;
		}

		loop_tests[i] =
		(loop_test_t)gcc_jit_result_get_code(results[i], "loop_test");

		if(!loop_tests[i]){
			fprintf(stderr, "NULL loop_test");
			goto error;
		}

error:
		gcc_jit_context_release(child_ctxt);

	}
	t = clock() - t;

	double compile_time_total = ((double)t) / CLOCKS_PER_SEC;
	double compile_time_one = compile_time_total / compile_num;
	printf("Compilation for n = %lu took %fs (1/%f).\n",
			compile_num, compile_time_total, compile_time_one);

	size_t run_num = strtoull(argv[2], 0, 0);
	t = clock();
	unsigned long result = loop_tests[0](run_num);
	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running loop for n = %lu took %fs with res %lu\n",
			run_num, run_time_total, result);

	return 0;
}
