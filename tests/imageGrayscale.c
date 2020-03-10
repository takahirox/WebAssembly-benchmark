void imageGrayscale(unsigned char *data, const int width, const int height) {
  const int il = width * height;
  for (int i = 0; i < il; i++) {
    const unsigned char r = data[i * 4 + 0];
    const unsigned char g = data[i * 4 + 1];
    const unsigned char b = data[i * 4 + 2];
    data[i * 4 + 0] = data[i * 4 + 1] = data[i * 4 + 2] =
      0.2126 * r + 0.7152 * g + 0.0722 * b;
  }
}
