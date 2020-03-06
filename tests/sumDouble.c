double sumDouble(const double* array, const int n) {
  double s = 0.0;
  int i = 0;
  for (; i < n; i++) {
    s += array[i];
  }
  return s;
}
