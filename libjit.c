#include <jit/jit.h>
#include <time.h>
#include <stdlib.h>

jit_function_t compile(jit_context_t context)
{
	jit_value_t i, sum, n, temp;
	jit_context_build_start(context);

	jit_type_t signature;
	jit_type_t params[1];
	params[0] = jit_type_ulong;

	signature = jit_type_create_signature(jit_abi_cdecl, jit_type_ulong, params, 1, 1);
	jit_function_t func = jit_function_create(context, signature);
	jit_type_free(signature);

	n = jit_value_get_param(func, 0);
	i = jit_value_create(func, jit_type_ulong);
	sum = jit_value_create(func, jit_type_ulong);

	jit_label_t cond = jit_label_undefined;
	jit_label_t out = jit_label_undefined;

	temp = jit_insn_load(func, jit_value_create_long_constant(func, jit_type_ulong, 0));
	jit_insn_store(func, i, temp);
	temp = jit_insn_load(func, jit_value_create_long_constant(func, jit_type_ulong, 0));
	jit_insn_store(func, sum, temp);

	jit_insn_label(func, &cond);

	temp = jit_insn_le(func, i, n);
	jit_insn_branch_if_not(func, temp, &out);

	/* body */
	temp = jit_insn_add(func, sum, i); /* sum += i */
	jit_insn_store(func, sum, temp);
	temp = jit_insn_add(func, i, jit_value_create_long_constant(func, jit_type_ulong, 1));
	jit_insn_store(func, i, temp);

	jit_insn_branch(func, &cond);

	jit_insn_label(func, &out);
	/* jump out */
	jit_insn_return(func, sum);

	/* compile */
	jit_function_compile(func);
	jit_context_build_end(context);

	return func;
}

int main(int argc, char **argv){

	if(argc != 3){
		fprintf(stderr, "Usage: %s compile_num loop_num\n", argv[0]);
		return -1;
	}

	jit_context_t context;
	context = jit_context_create();
	jit_function_t func = 0;

	size_t compile_num = strtoull(argv[1], 0, 0);

	clock_t t = clock();
	for(size_t i = 0; i < compile_num; ++i){
		func = compile(context);
	}
	t = clock() - t;

	double compile_time_total = ((double)t) / CLOCKS_PER_SEC;
	double compile_time_one = compile_time_total / compile_num;
	printf("Compilation for n = %lu took %fs (1/%f).\n",
			compile_num, compile_time_total, compile_time_one);

	size_t run_num = strtoull(argv[2], 0, 0);
	jit_ulong result = 0;
	void *args[1] = {&run_num};
	t = clock();
	jit_function_apply(func, args, &result);
	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running loop for n = %lu took %fs with res %lu\n", 
			run_num, run_time_total, result);

	jit_context_destroy(context);

	return 0;
}
