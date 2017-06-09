#include <math.h>

struct position {
  double x;
  double y;
  double z;
};

int collisionDetection(struct position *positions,
                       double *radiuses,
                       unsigned char *res, int n) {
  int count = 0;
  for (int i = 0; i < n; i++) {
    struct position p = positions[i];
    double r = radiuses[i];
    unsigned char collision = 0;
    for (int j = i+1; j < n; j++) {
      struct position  p2 = positions[j];
      double r2 = radiuses[j];
      double dx = p.x - p2.x;
      double dy = p.y - p2.y;
      double dz = p.z - p2.z;
      double d = sqrt(dx*dx + dy*dy + dz*dz);
      if (r > d) {
        collision = 1;
        count++;
        break;
      }
    }
    int index = (i / 8) | 0;
    unsigned char pos = 7 - (i % 8);
    if (collision == 0) {
      res[index] &= ~(1 << pos);
    } else {
      res[index] |= (1 << pos);
    }
  }
  return count;
}
