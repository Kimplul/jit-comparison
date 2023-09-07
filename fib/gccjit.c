#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgccjit.h>

typedef unsigned long (*fib_test_t) (unsigned long);

void compile(gcc_jit_context *ctxt)
{
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
				"fib",
				1, params, 0);

	gcc_jit_block *b_fib_cond =
		gcc_jit_function_new_block(func, "fib_cond");

	gcc_jit_block *b_fib_simple =
		gcc_jit_function_new_block(func, "fib_simple");

	gcc_jit_block *b_fib_recurse =
		gcc_jit_function_new_block(func, "fib_recurse");

	/* n <= 2 */
	gcc_jit_block_end_with_conditional(b_fib_cond, NULL,
			gcc_jit_context_new_comparison(ctxt, NULL, GCC_JIT_COMPARISON_LE,
				gcc_jit_param_as_rvalue(n),
				gcc_jit_context_new_rvalue_from_long(ctxt, ulong_type, 2)),
			b_fib_simple,
			b_fib_recurse);

	/* return 1; */
	gcc_jit_block_end_with_return(b_fib_simple, NULL,
			gcc_jit_context_new_rvalue_from_long(ctxt, ulong_type, 1));

	/* return fib(n - 1) + fib(n - 2); */
	gcc_jit_rvalue *first_args[1] = {gcc_jit_context_new_binary_op(ctxt, NULL,
			GCC_JIT_BINARY_OP_MINUS, ulong_type,
			gcc_jit_param_as_rvalue(n),
			gcc_jit_context_new_rvalue_from_long(ctxt, ulong_type, 1))};

	gcc_jit_rvalue *first_call = gcc_jit_context_new_call(ctxt, NULL,
			func, 1, first_args);

	gcc_jit_rvalue *second_args[1] = {gcc_jit_context_new_binary_op(ctxt, NULL,
			GCC_JIT_BINARY_OP_MINUS, ulong_type,
			gcc_jit_param_as_rvalue(n),
			gcc_jit_context_new_rvalue_from_long(ctxt, ulong_type, 2))};

	gcc_jit_rvalue *second_call = gcc_jit_context_new_call(ctxt, NULL,
			func, 1, second_args);

	gcc_jit_block_end_with_return(b_fib_recurse, NULL,
			gcc_jit_context_new_binary_op(ctxt, NULL,
				GCC_JIT_BINARY_OP_PLUS, ulong_type,
				first_call, second_call));
}

int main(int argc, char **argv){
	if (argc != 3) {
		fprintf(stderr, "Usage: %s compile_num fib_num\n", argv[0]);
		return -1;
	}

	gcc_jit_context *ctxt = NULL;

	ctxt = gcc_jit_context_acquire();
	gcc_jit_context_set_int_option(ctxt, GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 2);
	gcc_jit_context_add_driver_option(ctxt, "-march=native");

	size_t compile_num = strtoull(argv[1], 0, 0);
	fib_test_t *fib_tests = calloc(sizeof(fib_test_t), compile_num);
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

		fib_tests[i] =
			(fib_test_t)gcc_jit_result_get_code(results[i], "fib");

		if(!fib_tests[i]){
			fprintf(stderr, "NULL fib");
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
	unsigned long result = fib_tests[0](run_num);
	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running fib for n = %lu took %fs with res %lu\n",
			run_num, run_time_total, result);

	return 0;
}
