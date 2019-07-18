# WebAssembly-banchmark

Easy JavaScript vs WebAssembly performance comparison test-set.

## Demo

[Online Demo](https://takahirox.github.io/WebAssembly-benchmark)

## Develop locally

```sh
$ git clone https://github.com/mavrukin/WebAssembly-benchmark.git
$ cd WebAssembly-benchmark
$ ./scripts/run_server.sh
# access http://localhost:8080
```

## Note

- In WebAssembly performance measurement, I include data copying between WASM and JS so far because I first wanted to know if we can simply replace JS functions with WASM functions. And even if I allocate WASM memory beforehand and remove copying, WASM is still slower in most of the micro tests. (06/09/2017)

- mavrukin (07/18/19): Additional changes are now live that help improve the performacne of WebAssembly benchmarks further:
  - The compilation scripts for the failing benchmarks from the previous version have been simplified in the following ways:
    - Only export the necessary function, not _malloc or _free, also don't export ccall or cwrap, as they are not necessary anymore
    - Use -ffast-math in order to help optimize calculations even further
    - Reduce all the other flags that are not needed for a small, manually initialized binary
    - Output file is now .wasm and now .js (-o $0.wasm and not -o $0.js)
  - The HTML now initializes the wasm environment manually with a dedicated memory object that is computed ahead of time in 
    terms of its memory requirements.  This memory object is then used in order to provide a view onto the internall arrays
    in the benchmark.  This is done so that no memory needs to be copied between JS and WASM, i.e., no need to copy bytes
    but just pass the pointers into the right memory location in the generated buffer
  - Remove the usage of the .js library and do everything manually.  Cleanup the code to use ES6 style.  
  - New spreadsheet w/ benchmark results will be published shortly.  
- mavrukin (07/02/19): This version of the code makes a number of changes to the C/C++ executable:
  - Use 'const' wherever possible, this allows a significant level of optimization on the compilation side
  - Change compile flags from -Os (which optimized for size) to -O3 which optimizes exclusively for performance and is best suited for a production version.  This also required the explicit export of some additional functions like _malloc and _free.  The C++ code also used auto* where possible and for-each iterations constructs.  In sum this made the code more readable and because of known compiler level optimizations, more performant.  
  - Benchmark comparison between the version above from 06/09/17 and this one can be found on this sheet: https://docs.google.com/spreadsheets/d/1DO0SKsZceLjPer3ZL7-2BdMwVWS-M-vhKgoZI8l-RDQ/edit#gid=0 
