# WebAssembly-banchmark

Easy JavaScript vs WebAssembly performance comparison test-set.

## Demo

[Online Demo](https://takahirox.github.io/WebAssembly-benchmark)

## Develop locally

```sh
$ git clone https://github.com/takahirox/WebAssembly-benchmark.git
$ cd WebAssembly-benchmark
$ ./scripts/run_server.sh
# access http://localhost:8080
```

## Note

- In WebAssembly performance measurement, I include data copying between WASM and JS so far because I first wanted to know if we can simply replace JS functions with WASM functions. And even if I allocate WASM memory beforehand and remove copying, WASM is still slower in most of the micro tests. (06/09/2017)
