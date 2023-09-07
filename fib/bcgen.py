g = BCGen(r = 5, f = 0)

g.rule('li',  '_R___I', 'R0 = IMM0;')
g.rule('add', '_RRR__', 'R0 = R1 + R2;')
g.rule('jle', 'rRR__I', 'if (R0 <= R1) JUMP(IMM0);')
g.rule('jge', 'rRR__I', 'if (R0 >= R1) JUMP(IMM0);')
g.rule('jmp', 'r____I', 'JUMP(IMM0);')

g.rule('mov',  '_RR___', 'R0 = R1;')
g.rule('subi', '_RR__I', 'R0 = R1 - IMM0;')

# skip current pc and return to next one
g.rule('call', 'r____I',
        '*(ubcval_t *)r4 = pc + 1; r4 += sizeof(ubcval_t); JUMP(IMM0);')

# assumes there aren't more returns than calls, as otherwise we might run out of
# stack space
g.rule('ret',  '______',
        """
        r4 -= sizeof(ubcval_t);
        ubcval_t dst = *(ubcval_t *)r4;
        if (dst != 0) {
                JUMP(dst);
        }""")

g.rule('pop',  '_R____', 'r4 -= sizeof(ubcval_t); R0 = *(ubcval_t *)r4;')
g.rule('push', '_R____', '*(ubcval_t *)r4 = R0; r4 += sizeof(ubcval_t);')

g.write()
