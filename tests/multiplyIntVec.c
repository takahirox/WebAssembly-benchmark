void multiplyIntVec(const int* src1, const int* src2, int* res, const int n) {
  for (int i = 0; i < n; i++) {
    res[i] = src1[i] * src2[i];
  }
}
