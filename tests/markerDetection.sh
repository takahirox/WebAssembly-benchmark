emcc markerDetection.cpp \
  -Os \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s DEMANGLE_SUPPORT=1 \
  -s TOTAL_MEMORY=134217728 \
  -s "EXPORTED_FUNCTIONS=['_newARDetector', '_detect', '_freeResult']" \
  -o markerDetection.js
