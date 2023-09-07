#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include "../lib/lightening/lightening.h"

static jit_state_t *j;

typedef unsigned long (*loop_jit_t)(unsigned long);
struct jit_info {
	jit_word_t code_size;
	loop_jit_t func;
};

struct jit_info compile()
{
	jit_reloc_t recurse;

	jit_word_t code_size = 4096;
	uint8_t *code_base = mmap(NULL, code_size,
			PROT_EXEC | PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	init_jit();
	j = jit_new_state(NULL, NULL);
	jit_begin(j, code_base, code_size);
	size_t align = jit_enter_jit_abi(j, 2, 0, 0);

	/* implement */
	jit_load_args_1(j, jit_operand_gpr(JIT_OPERAND_ABI_WORD, JIT_V0));
	recurse = jit_bgti(j, JIT_V0, 2); /* n <= 2 */
	jit_leave_jit_abi(j, 2, 0, align);
	jit_reti(j, 1);

	jit_patch_here(j, recurse);
	/* fib(n - 1) */
	jit_subi(j, JIT_V0, JIT_V0, 1);
	jit_calli_1(j, code_base, jit_operand_gpr(JIT_OPERAND_ABI_WORD, JIT_V0));
	jit_retval_l(j, JIT_V1);

	/* fib(n - 2) */
	jit_subi(j, JIT_V0, JIT_V0, 1);
	jit_calli_1(j, code_base, jit_operand_gpr(JIT_OPERAND_ABI_WORD, JIT_V0));
	jit_retval_l(j, JIT_R0);

	/* fib(n - 1) + fib(n - 2) */
	jit_addr(j, JIT_R0, JIT_V1, JIT_R0);
	jit_leave_jit_abi(j, 2, 0, align);
	jit_retr(j, JIT_R0); /* return sum; */

	size_t size = 0;
	void *loop = jit_end(j, &size);

	//jit_reset(j);
	jit_destroy_state(j);

	return (struct jit_info){code_size, loop};
}

int main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(stderr, "Usage: %s compile_num fib_num\n", argv[0]);
		return -1;
	}

	size_t compile_num = strtoull(argv[1], 0, 0);
	struct jit_info *info = (struct jit_info *)calloc(sizeof(struct jit_info), compile_num);

	clock_t t = clock();
	for(size_t i = 0; i < compile_num; ++i){
		info[i] = compile();
	}
	t = clock() - t;

	double compile_time_total = ((double)t) / CLOCKS_PER_SEC;
	double compile_time_one = compile_time_total / compile_num;
	printf("Compilation for n = %lu took %fs (1/%f).\n",
			compile_num, compile_time_total, compile_time_one);

	size_t run_num = strtoull(argv[2], 0, 0);
	t = clock();
	size_t result = info[0].func(run_num);
	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running fib for n = %lu took %fs with res %lu\n", 
			run_num, run_time_total, result);

	for(size_t i = 0; i < compile_num; ++i)
		munmap(info[0].func, info[0].code_size);

	return 0;
}
