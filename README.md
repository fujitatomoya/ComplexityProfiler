# ComplexityProfiler
Simple Complexity Checker Library using system clock for C++

### Motivation
sometimes we like to see the primitive time consumption for each function.
at least, this easy library can help me sometimes...

### Preconditions
Linux Ubuntu14/16 (only confirmed)

### How to build
```bash
mkdir build
cd build
cmake ../
make
```

### How to use
see test/ComplexityProfiler_test.cpp

### Limitation
once prove starts with _PERF_PROVE_START, it must be ended with _PERF_PROVE_END.
(or you can cancel the prove with _PERF_PROVE_CANCEL.)
this is because that profiler internally uses mutex_lock for each prove name.

### Contribution
N.A

### Author
[TomoyaFujita](https://github.com/tomoyafujita)
tomoya.fujita825@gmail.com
