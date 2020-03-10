int sumInt(const int* array, const int n) {
  int s = 0;
  int i = 0;
  for (; i < n; i++) {
    s += array[i];
  }
  return s;
}
