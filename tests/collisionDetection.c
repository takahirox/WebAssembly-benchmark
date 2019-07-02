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
    const struct position* p = &positions[i];
    const double r = radiuses[i];
    unsigned char collision = 0;
    for (int j = i+1; j < n; j++) {
      const struct position*  p2 = &positions[j];
      const double dx = p->x - p2->x;
      const double dy = p->y - p2->y;
      const double dz = p->z - p2->z;
      const double d = sqrt(dx*dx + dy*dy + dz*dz);
      if (r > d) {
        collision = 1;
        count++;
        break;
      }
    }
    const int index = (i / 8) | 0;
    const unsigned char pos = 7 - (i % 8);
    if (collision == 0) {
      res[index] &= ~(1 << pos);
    } else {
      res[index] |= (1 << pos);
    }
  }
  return count;
}
