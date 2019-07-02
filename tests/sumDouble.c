double sumDouble(const double* array, const int n) {
  double s = 0.0;
  for (int i = 0; i < n; i++) {
    s += array[i];
  }
  return s;
}
