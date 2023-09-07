# jit-comparison

A quick and pretty naive comparison of some popular JIT frameworks.
Frameworks tested:

+ GCCJIT
+ LibJIT
+ MIR
+ GNU Lightning
+ Lightening

Additionally, some of my projects:

+ copyjit
+ BCGen bytecode generator
+ Custom bytecode

# Performance/results
Tests are split between compiling the C functions
```
unsigned long loop(unsigned long n)
{
  unsigned long sum = 0;
  for(unsigned long i = 0; i < n; ++i)
    sum += i;

   return sum;
}
```

and

```
unsigned long fib(unsigned long n)
{
  if (n <= 2)
    return 1;

  return fib(n - 1) + fib(n - 2);
}
```

`N` number of times, and run the compiled code with some `n`.
All tests (except GCCJIT) were run with `N = 1 000 000` and `n = 1 000 000 000` for `loop`
and `N = 1 000 000` and `n = 42` for `fib`.

GCCJIT was so slow that I decided to limit it to only `N = 1 000` and the final
value in the table below is `time(N) * 1000`.

`loop`:
| framework     | compilation time (s) | runtime (s) |
|---------------|----------------------|-------------|
| GCCJIT        | 7602*                | 0.217       |
| GNU Lightning | 4.224                | 0.217       |
| lightening    | 1.495                | 0.217       |
| LibJIT        | 5.788                | 0.215       |
| MIR           | 18.614               | 0.218       |
| copyjit       | 1.366                | 0.430       |
| bcgen         | 0.108                | 1.721       |
| bytecode      | 0.773                | 5.598       |

For reference, the equivalent `C` code compiled with '-O0' executes
in 0.701s and with '-O2' in 0.223s.

`fib`:
| framework     | compilation time (s) | runtime (s) |
|---------------|----------------------|-------------|
| GCCJIT        | 26418*               | 0.233       |
| GNU Lightning | TODO                 | TODO        |
| lightening    | 3.491                | 0.595       |
| LibJIT        | TODO                 | TODO        |
| MIR           | TODO                 | TODO        |
| copyjit       | 3.247                | 0.997       |
| bcgen         | 0.328                | 1.943       |
| bytecode      | TODO                 | TODO        |

For reference, the equivalent `C` code compiled with '-O0' executes
in 0.667s and with '-O2' in 0.240s.

# Conclusions

Besides the fact that a single test point is hardly enough to conclusively say anything,
we can still see some patterns. First and foremost, a single loop is fairly
obviously too simple of a construct as most of the projects are able to produce
near optimal machine code. More tests and more varied constructs would be required
for a more accurate comparison.

Moreover, the ease of use of each framework is not really taken into account.
Generally the higher abstraction level a framework provides, the easier it is to
use, but with likely higher compilation costs. This can be seen with MIR and
LibJIT, both of which try to provide a C-like programming interface being
generally slower than projects like Lightning, lightening and copyjit, which
present a more assembly language like interface.

How a library is integrated into a project seems to play a fairly major part
in compilation speed. `bcgen` seems to reach its impressive speeds by being easily
inlined by the compiler, minimizing extra function calls. Other projects tend to
be more complicated and more difficult to inline, leading to the compiler
choosing to output calls rather than inlining them. GNU Lightning and
lightening share a lot of code, but lightening is generally compiled
alongside whatever project it is inteded to be included in whereas GNU Lightning
is generally distributed as a separate library, potentially disallowing some inlining
optimizations. Although it should be noted that lightening doesn't have a
separate compile phase, whereas GNU lightning does. Especially with link-time
optimizations, the best way to increase compilation speed seems to be trying to
inline as much as possible. This comparison uses only simple '-O2' compilation
flags, with some trickery I suspect you could convince the compiler to inline
more parts.

Testing methodology could probably be improved, for example with randomized
data layouts and so on. The results should be taken with a grain of salt.

The tests were run on an AMD Ryzen 5 5600, your results will likely vary.

# Missing comparisons

There are some notable jit frameworks missing, such as Clang, Cranelift and
DynASM. Adding them would be useful, but I haven't had the time/motivation to do
so.
