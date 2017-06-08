void multiplyDoubleVec(double *src1, double *src2, double *res, int n) {
  for (int i = 0; i < n; i++) {
    res[i] = src1[i] * src2[i];
  }
}
