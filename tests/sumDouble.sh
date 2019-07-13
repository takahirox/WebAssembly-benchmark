emcc sumDouble.c \
  -O3 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s "EXPORTED_FUNCTIONS=['_sumDouble']" \
  -o sumDouble.wasm
