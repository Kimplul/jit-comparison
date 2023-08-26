#include <mir.h>
#include <mir-gen.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned long (*jit_loop_t)(unsigned long);
jit_loop_t compile(MIR_context_t ctx)
{
	MIR_new_module(ctx, "m");

	MIR_type_t res_type[1] = {MIR_T_U64};
	MIR_item_t func = MIR_new_func(ctx, "loop", 1, res_type, 1, MIR_T_U64, "n");

	MIR_reg_t n = MIR_reg(ctx, "n", func->u.func);
	MIR_reg_t i = MIR_new_func_reg(ctx, func->u.func, MIR_T_I64, "i");
	MIR_reg_t sum = MIR_new_func_reg(ctx, func->u.func, MIR_T_I64, "sum");

	MIR_label_t cond = MIR_new_label(ctx);
	MIR_label_t out = MIR_new_label(ctx);

	/* sum = 0 */
	MIR_append_insn(ctx, func,
			MIR_new_insn(ctx, MIR_MOV,
				MIR_new_reg_op(ctx, sum),
				MIR_new_uint_op(ctx, 0)));
	/* i = 0 */
	MIR_append_insn(ctx, func,
			MIR_new_insn(ctx, MIR_MOV,
				MIR_new_reg_op(ctx, i),
				MIR_new_uint_op(ctx, 0)));

	/* loop top */
	MIR_append_insn(ctx, func, cond);
	MIR_append_insn(ctx, func,
			MIR_new_insn(ctx, MIR_UBGE,
				MIR_new_label_op(ctx, out),
				MIR_new_reg_op(ctx, i),
				MIR_new_reg_op(ctx, n)));

	/* sum += i */
	MIR_append_insn(ctx, func,
			MIR_new_insn(ctx, MIR_ADD,
				MIR_new_reg_op(ctx, sum),
				MIR_new_reg_op(ctx, sum),
				MIR_new_reg_op(ctx, i)));

	/* ++i */
	MIR_append_insn(ctx, func,
			MIR_new_insn(ctx, MIR_ADD,
				MIR_new_reg_op(ctx, i),
				MIR_new_reg_op(ctx, i),
				MIR_new_uint_op(ctx, 1)));

	/* jump to cond */
	MIR_append_insn(ctx, func,
			MIR_new_insn(ctx, MIR_JMP, MIR_new_label_op(ctx, cond)));

	MIR_append_insn(ctx, func, out);

	MIR_append_insn(ctx, func,
			MIR_new_ret_insn(ctx, 1, MIR_new_reg_op(ctx, sum)));

	MIR_finish_func(ctx);
	MIR_finish_module(ctx);
	MIR_load_module(ctx, func->module);
	return MIR_gen(ctx, 0, func);
}

int main(int argc, char **argv)
{
	if(argc != 3){
		fprintf(stderr, "Usage: %s compile_num loop_num\n", argv[0]);
		return -1;
	}

	MIR_context_t ctx = MIR_init();
	MIR_gen_init(ctx, 0);
	MIR_gen_set_optimize_level(ctx, 0, 0);
	MIR_link(ctx, MIR_set_gen_interface, NULL);
	jit_loop_t func = 0;

	size_t compile_num = strtoull(argv[1], 0, 0);

	clock_t t = clock();
	for(size_t i = 0; i < compile_num; ++i){
		func = compile(ctx);
	}
	t = clock() - t;

	double compile_time_total = ((double)t) / CLOCKS_PER_SEC;
	double compile_time_one = compile_time_total / compile_num;
	printf("Compilation for n = %lu took %fs (1/%f).\n",
			compile_num, compile_time_total, compile_time_one);

	size_t run_num = strtoull(argv[2], 0, 0);
	t = clock();
	size_t result = func(run_num);
	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running loop for n = %lu took %fs with res %lu\n", 
			run_num, run_time_total, result);

	MIR_gen_finish(ctx);
	MIR_finish(ctx);
	return 0;
}
