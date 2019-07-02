void imageConvolute(const unsigned char *data,
                    unsigned char *data2,
                    const int width, const int height,
                    const double* weights,
                    const int wwidth, const int wheight) {
  const int halfWWidth = wwidth / 2;
  const int halfWHeight = wheight / 2;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      double r = 0.0;
      double g = 0.0;
      double b = 0.0;
      double a = 0.0;
      for (int wy = 0; wy < wheight; wy++) {
        const int sy = y + wy - halfWHeight;
        if (sy < 0 || sy >= height)
          continue;
        for (int wx = 0; wx < wwidth; wx++) {
          const int sx = x + wx - halfWWidth;
          if (sx < 0 || sx >= width)
            continue;
            const int index = sy * width + sx;
            const double weight = weights[wy * wwidth + wx];
            r += data[index * 4 + 0] * weight;
            g += data[index * 4 + 1] * weight;
            b += data[index * 4 + 2] * weight;
            a += data[index * 4 + 3] * weight;
        }
      }
      const int index = y * width + x;
      data2[index * 4 + 0] = r;
      data2[index * 4 + 1] = g;
      data2[index * 4 + 2] = b;
      data2[index * 4 + 3] = a;
    }
  }
}
