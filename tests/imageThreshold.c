// from https://pdfs.semanticscholar.org/8d74/418ec3c4e2ff45b72e723ac0fbe5fcd58620.pdf
void imageThreshold(unsigned char *data, const int width, const int height) {
  int array[width * height];
  const int s = 8;
  const int s2 = s / 2;
  const int t = 15;
  const double t2 = (double)(100 - t) / 100.0;
  for (int i = 0; i < width; i++) {
    int sum = 0;
    for (int j = 0; j < height; j++) {
      const int index = j * width + i;
      const unsigned char r = data[index * 4 + 0];
      const unsigned char g = data[index * 4 + 1];
      const unsigned char b = data[index * 4 + 2];
      data[index * 4] = ((unsigned char)(0.2126 * r + 0.7152 * g + 0.0722 * b));
      sum += data[index * 4];
      if (i == 0) {
        array[index] = sum;
      } else {
        array[index] = array[index - 1] + sum;
      }
    }
  }
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      int x1 = i - s2;
      int x2 = i + s2;
      int x1_1 = x1 - 1;
      int y1 = j - s2;
      int y2 = j + s2;
      int y1_1 = y1 - 1;
      if (x1 < 0) x1 = 0;
      if (x2 >= width) x2 = width - 1;
      if (x1_1 < 0) x1_1 = 0;
      if (y1 < 0) y1 = 0;
      if (y2 >= height) y2 = height - 1;
      if (y1_1 < 0) y1_1 = 0;
      const int count = (x2 - x1) * (y2 - y1);
      const int index = j * width + i;
      const int index1 = y2 * width + x2;
      const int index2 = y1_1 * width + x2;
      const int index3 = y2 * width + x1_1;
      const int index4 = y1_1 * width + x1_1;
      const int sum = array[index1] - array[index2] - array[index3] + array[index4];
      if (data[index * 4] * count <= sum * t2) {
        data[index * 4 + 0] = data[index * 4 + 1] = data[index * 4 + 2] = 0;
      } else {
        data[index * 4 + 0] = data[index * 4 + 1] = data[index * 4 + 2] = 255;
      }
    }
  }
}
