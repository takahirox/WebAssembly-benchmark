void multiplyDoubleVec(const double* src1, const double* src2, double *res, const int n) {
  int i = 0;
  for (; i < n; i++) {
    res[i] = src1[i] * src2[i];
  }
}
