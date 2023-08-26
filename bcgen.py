g = BCGen(r = 4, f = 0)

g.rule('li',  '_R___I', 'R0 = IMM0;')
g.rule('add', '_RRR__', 'R0 = R1 + R2;')
g.rule('jge', 'rRR__I', 'if (R0 >= R1) JUMP(IMM0);')
g.rule('jmp', 'r____I', 'JUMP(IMM0);')

g.write()
