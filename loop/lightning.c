#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>

#include <lightning.h>

static jit_state_t *_jit;

typedef unsigned long (*loop_jit_t)(unsigned long);
struct jit_info {
	jit_word_t code_size;
	loop_jit_t func;
};

struct jit_info compile()
{
	jit_node_t *cond;
	jit_node_t *jump;
	jit_node_t *out;
	jit_node_t *n;

	jit_uint8_t *code = 0;
	jit_word_t code_size = 0;

	loop_jit_t loop = 0;

	_jit = jit_new_state();
	jit_prolog();

	/* implement */
	n = jit_arg();
	jit_getarg(JIT_R2, n);

	jit_movi(JIT_R0, 0); /* sum = 0; */
	jit_movi(JIT_R1, 0); /* i = 0; */

	cond = jit_label();
	out = jit_bger(JIT_R1, JIT_R2); /* i >= n => out */
	jit_addr(JIT_R0, JIT_R0, JIT_R1); /* sum += i */
	jit_addi(JIT_R1, JIT_R1, 1); /* i += 1 */
	jump = jit_jmpi();

	jit_patch_at(jump, cond);
	jit_patch(out);
	jit_retr(JIT_R0); /* return sum; */

	jit_realize();

	/* disable external data segment */
	jit_get_data(NULL, NULL);
	jit_set_data(NULL, 0, JIT_DISABLE_DATA | JIT_DISABLE_NOTE);

	jit_get_code(&code_size);

	code_size = (code_size + 4095) & - 4096;

	do {
		code = mmap(NULL, code_size,
				PROT_EXEC | PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANON, -1, 0);

		jit_set_code(code, code_size);

		if((loop = jit_emit()) == NULL){
			munmap(code, code_size);
			code_size += 4096;
		}

	} while(loop == NULL);


	jit_clear_state();
	jit_destroy_state();

	return (struct jit_info){code_size, loop};
}

int main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(stderr, "Usage: %s compile_num loop_num\n", argv[0]);
		return -1;
	}

	size_t compile_num = strtoull(argv[1], 0, 0);
	struct jit_info *info = (struct jit_info *)calloc(sizeof(struct jit_info), compile_num);

	init_jit("loop");

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
	printf("Running loop for n = %lu took %fs with res %lu\n", 
			run_num, run_time_total, result);

	for(size_t i = 0; i < compile_num; ++i)
		munmap(info[i].func, info[i].code_size);

	finish_jit();
	return 0;
}
