# jit-comparison

A quick and pretty naive comparison of some popular JIT frameworks. Frameworks tested:

+ GCCJIT
+ LibJIT
+ MIR
+ GNU Lightning

# Performance
All tests were to compile the C-function
```
unsigned long loop(unsigned long n)
{
  unsigned long sum = 0;
  for(unsigned long i = 0; i < n; ++i)
    sum += i;
    
   return sum;
}
```

`N` number of times, and run the compiled code with some `n`. All tests (except GCCJIT) were run with `N = 1 000 000` and `n = 1 000 000 000`.
GCCJIT was so slow that I decided to limit it to only `N = 1 000`.

| framework     | compilation time (s) | code execution (s) |
|---------------|----------------------|--------------------|
| GCCJIT        | 37.19*               | 0.270              |
| GNU Lightning | 9.62                 | 0.420              |
| LibJIT        | 15.22                | 0.276              |
| MIR           | 43.60                | 0.273              |

# Conclusions
Besides the fact that a single test point is hardly enough to conclusively say anything, we can still see some patterns. GNU Lightning,
which is the lowest-level JIT framework compiles code the quickest, with very little to no optimisation as overhead. This results
in comparatively slow code execution, though still miles ahead of an interpreter. GNU Lightning also has the advantage that it's
been ported to a large number of architectures, though it still pales in comparison to GCC, which can compile code for essentially all
architectures supported by GCC. Note however, that the JIT in this example is hand-optimised to an extent and if used in, let's say,
a scripting lanugage, the implementer will be responsible for more sensible JIT usage, and try to avoid unnecessary load/stores among other things.
The other frameworks are more forgiving, with higher level abstractions and some optimization passes.

GCC has the best optimisation routines out of all the tested frameworks, though that isn't too obvious in this simple example. What is obvious,
is that optimisations and a full-blown compiler leads to _slow_ code compilation. (If you find issues with how I've used GCCJIT that would cause this kind
of massive slowdown, please let me know). GCCJIT might be suitable when you absolutely need to produce the fastest code possible, and compilation speed is
almost irrelevant. This does make is unsuitable for scripting language use.

LibJIT provides reasonably quick compilation times and pairs it with decent code execution times. Some drawbacks of the framework include limited machine code
architecture support, but it does come with an interpreter which is architecture agnostic. Currently only x86/64 and arm seems to be fully supported, with alpha
support being in alpha. Would be cool to see RISC-V or something, but I digress.

MIR compilations is the slowest of the three pure JIT projects, but does offer more optimisations than LibJIT. Architecture machine code support
seems to also be a lot better, though only Linux as an OS seems to be supported at the moment. MIR is very much experimental at the moment, but does look very
promising.

For a smaller hobby-like project I would probably go for LibJIT, as it's fairly easy to work with and seems to be fairly forgiving when used programatically.
I don't like the fact that it doesn't support that many architectures, and if I was aiming for a more widely-used but still small software project
I might go for Lightning. MIR seems to be best suited for larger projects, where there's enough resources to mitigate the slower compilation with some
kind of hybrid interpreted/compiled system. MIR seems to provide this OOTB, though I don't know what kind of latencies interpreted MIR has at the moment.
