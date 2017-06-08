void quicksortInt(int *array, int start, int end) {
  if (start >= end) return;
  int pivot = array[end];
  int left = 0;
  int right = 0;
  while (left + right < end - start) {
    int num = array[start+left];
    if (num < pivot) {
      left++;
    } else {
      array[start+left] = array[end-right-1];
      array[end-right-1] = pivot;
      array[end-right] = num;
      right++;
    }
  }
  quicksortInt(array, start, start+left-1);
  quicksortInt(array, start+left+1, end);
}
