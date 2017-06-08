void multiplyIntVec(int *src1, int *src2, int *res, int n) {
  for (int i = 0; i < n; i++) {
    res[i] = src1[i] * src2[i];
  }
}
