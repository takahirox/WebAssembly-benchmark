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

## Notes on WebAssembly Memory Management
As of version 1.39.8 it seems that the requested memory via the Memory object is no longer being fully honored IF the requested size is more than 256 pages.  Keep
in mind that a page size is 64KB, which effectively means that when the WASM module starts up it doesn an initial allocation of ~16MB.  This is perfectly reasonable
for most applications and likely works just fine in most cases.  If you do need more memory then that, you will need to make an explicit request to "grow" the allocated memory.  The grow operation takes an incremental number of pages to grow the memory by, and does not support negative numbers, i.e., shrinking the memory.  Additionally, when the memory growing operation is performed, the underlying buffer is replaced and deallocated.  Thus, if for example you initialized the WebAssembly module initially with the default values, then created (as an example) an Int32Array backed by the buffer from the module, then upon growing, the Int32Array's backing buffer will go to 0.  Memory growth effectively deletes whatever was stored in the buffer previously - for the better or for the worse.  

There are also some additional changes as well - for example, previously the buffer that was going to be used came from the Memory object, that is no longer the case.  The memory object will still allocate that size buffer that you requested, but it is a disconnected buffer from the one that is returned and used by the loaded WASM module, therefore you need to grab the returned buffer, which is not shared with the Memory object's buffer, and use that one in order to have a shared memory view of the data.  It is accessible via instance.exports.memory <-- the two things in there of interest are buffer (which is an ArrayBuffer and has property bytelength so signal how many bytes are there), and the method .grow(...) which takes one parameter, and that is the number of pages to increase the memory by.

Finally, when setting up the WASM module, it was necessary to pass an object of "parameters" to the intialization code.  In previous versions, for the memory related items, the object had this form: {"end": {memory: Memory Object}}, this has now changed to: {js: {mem: Memory Object}}.  

## Note on Change in Exported Function Names
Although the .sh files and the EXPORTED_FUNCTIONS emcc parameter request that the function name start with an "\_", despite the fact that in the associated .c/cpp file the function does not have such a name, this use to map in prior versions of WASM to calls to instance.exports._{function name} on the JS side.  This has now changed and despite what the build rules say, the "\_" character is now removed on the JS side, thus providing a direct mapping from JS to the C side of things, i.e., in order to invoke the C function (WASM) from JS, the call is now instance.exports.{function name} without the "\_".  This is an improvement in usability, although it was a bit tricky to track down.  
