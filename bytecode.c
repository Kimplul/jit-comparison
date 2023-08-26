#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

struct {
	uint8_t *ptr;
	size_t size;
	size_t max_size;
} code_buffer;

enum regs {
	R0,
	R1,
	R2,
	R3
};

enum ops {
	LI,
	ADD,
	JMP,
	JGE,
	RET
};

static void start_compile()
{
	code_buffer.ptr = (uint8_t *)calloc(1, 4096);
	code_buffer.size = 0;
	code_buffer.max_size = 4096;
}

static void end_compile() {}

static void insert(uint8_t op, uint8_t f0, uint8_t f1, uint8_t f2)
{
	if((code_buffer.size += 4) >= code_buffer.max_size){
		code_buffer.ptr = (uint8_t *)realloc(code_buffer.ptr,
				(code_buffer.max_size += 4096));
	}

	code_buffer.ptr[code_buffer.size - 1] = f2;
	code_buffer.ptr[code_buffer.size - 2] = f1;
	code_buffer.ptr[code_buffer.size - 3] = f0;
	code_buffer.ptr[code_buffer.size - 4] = op;
}

static void compile_li(uint8_t r, uint16_t i)
{
	insert(LI, r, i >> 8, i & 0xff);
}

static void compile_add(uint8_t r0, uint8_t r1, uint8_t r2)
{
	insert(ADD, r0, r1, r2);
}

static void compile_jmp(uint16_t o)
{
	insert(JMP, 0, o >> 8, o & 0xff);
}

static void compile_jge(uint8_t r0, uint8_t r1, uint16_t o)
{
	insert(JGE, r0, r1, o & 0xff);
}

static void compile_ret(uint8_t r)
{
	insert(RET, 0, 0, r);
}

static void *bytecode_buffer()
{
	return code_buffer.ptr;
}

static void *compile()
{
	start_compile();
	/* R0 = n */
	compile_li(R1, 0); /* sum = 0; */
	compile_li(R2, 0); /* i = 0 */
	compile_li(R3, 1); /* temp value for 1 */
	compile_jge(R2, R0, 12); /* i >= n => out */
	compile_add(R1, R1, R2); /* sum += i */
	compile_add(R2, R2, R3); /* i += 1 */
	compile_jmp(-16);
	compile_ret(R1);

	end_compile();

	return bytecode_buffer();
}

static size_t run(void *f, size_t a0)
{
	size_t registers[256] = {0};
	registers[R0] = a0;
	uint8_t *p = (uint8_t *)f;
top:
	uint8_t op = *p++;
	uint8_t f0 = *p++;
	uint8_t f1 = *p++;
	uint8_t f2 = *p++;
	switch(op){
		case LI:
			registers[f0] = ((int8_t)f1 << 8) + f2;
			goto top;

		case ADD:
			registers[f0] = registers[f1] + registers[f2];
			goto top;

		case JMP:
			p += ((int8_t)f1 << 8) + f2;
			goto top;

		case JGE:
			if(registers[f0] >= registers[f1])
				p += (int8_t)f2;

			goto top;

		case RET:
			return registers[f2];
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(stderr, "Usage: %s compile_num loop_num\n", argv[0]);
		return -1;
	}

	size_t compile_num = strtoull(argv[1], 0, 0);

	void **info = (void *)calloc(sizeof(void *), compile_num);

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
	size_t result = run(info[0], run_num);
	t = clock() - t;

	double run_time_total = ((double)t) / CLOCKS_PER_SEC;
	printf("Running loop for n = %lu took %fs with res %lu\n",
			run_num, run_time_total, result);

	for(size_t i = 0; i < compile_num; ++i)
		free(info[i]);

	return 0;
}
