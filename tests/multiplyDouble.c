double multiplyDouble(double a, double b, int n) {
  double c = 1.0;
  for (int i = 0; i < n; i++) {
    c = c * a * b;
  }
  return c;
}
