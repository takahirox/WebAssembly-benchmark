emcc multiplyIntVec.c \
  -O3 \
  -ffast-math \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s "EXPORTED_FUNCTIONS=['_multiplyIntVec']" \
  -o multiplyIntVec.wasm
