double multiplyDouble(const double a, const double b, const int n) {
  double c = 1.0;
  for (int i = 0; i < n; i++) {
    c = c * a * b;
  }
  return c;
}
