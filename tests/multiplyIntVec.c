void multiplyIntVec(const int* src1, const int* src2, int* res, const int n) {
  int i = 0;
  for (; i < n; i++) {
    res[i] = src1[i] * src2[i];
  }
}
