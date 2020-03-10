void quicksortDouble(double *array, const int start, const int end) {
  if (start >= end) return;
  const double pivot = array[end];
  int left = 0;
  int right = 0;
  while (left + right < end - start) {
    const double num = array[start+left];
    if (num < pivot) {
      left++;
    } else {
      array[start + left] = array[end - right - 1];
      array[end - right - 1] = pivot;
      array[end - right] = num;
      right++;
    }
  }
  quicksortDouble(array, start, start + left - 1);
  quicksortDouble(array, start + left + 1, end);
}
