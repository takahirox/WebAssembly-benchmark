emcc multiplyDouble.c \
  -O3 \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s DEMANGLE_SUPPORT=1 \
  -s "EXPORTED_FUNCTIONS=['_multiplyDouble']" \
  -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
  -o multiplyDouble.js
