emcc collisionDetection.c \
  -O3 \
  -ffast-math \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s "EXPORTED_FUNCTIONS=['_collisionDetection']" \
  -o collisionDetection.wasm