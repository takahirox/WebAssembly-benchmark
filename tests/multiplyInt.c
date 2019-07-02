int multiplyInt(const int a, const int b, const int n) {
  int c = 1.0;
  for (int i = 0; i < n; i++) {
    c = c * a * b;
  }
  return c;
}
