emcc fib.c \
  -O3 \
  -ffast-math \
  -s "EXPORTED_FUNCTIONS=['_fib']" \
  -o fib.wasm