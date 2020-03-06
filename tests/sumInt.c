int sumInt(const int* array, const int n) {
  int s = 0;
  for (int i = 0; i < n; i++) {
    s += *(array + i);
  }
  return s;
}
