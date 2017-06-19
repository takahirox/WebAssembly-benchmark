// based on cv.js and aruco.js of https://github.com/jcmellado/js-aruco
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <stack>

namespace CV {
  class Image {
    public:
      int width;
      int height;
      int length;
      unsigned char *data;
      Image(int _width, int _height, unsigned char *_data)
        : width(_width), height(_height), length(_width * _height),
          data(_data) {}
  };

  class Point {
    public:
      int x;
      int y;
      Point(int _x, int _y): x(_x), y(_y) {}
  };

  class Square {
    public:
      int x;
      int y;
      int width;
      int height;
      Square(int _x, int _y, int _width, int _height)
        : x(_x), y(_y), width(_width), height(_height) {}
  };

  class Slice {
    public:
      int start_index;
      int end_index;
      Slice(int _start_index, int _end_index)
        : start_index(_start_index), end_index(_end_index) {}
  };

  #define NEIGHBORHOOD_NUM 8
  char neighborhood[NEIGHBORHOOD_NUM][2] = {
    {1, 0},
    {1, -1},
    {0, -1},
    {-1, -1},
    {-1, 0},
    {-1, 1},
    {0, 1},
    {1, 1}
  };

  Image* grayscale(Image *imageSrc, Image *imageDst);
  Image* threshold(Image *imageSrc, Image *imageDst, unsigned char threshold);
  Image* adaptiveThreshold(Image *imageSrc, Image *imageDst, unsigned char kernelSize, short threshold);
  int otsu(Image *imageSrc);
  Image* stackBoxBlur(Image *imageSrc, Image *imageDst, unsigned char kernelSize);
  Image* gaussianBlur(Image *imageSrc, Image *imageDst, Image *imageMean, unsigned char kernelSize);
  Image* gaussianBlurFilter(Image *imageSrc, Image *imageDst, double *kernel, unsigned char kernelSize, bool horizontal);
  double* gaussianKernel(unsigned char kernelSize);
  std::vector<std::vector<Point*>* >* findContours(Image *imageSrc, short *binary);
  std::vector<Point*>* borderFollowing(short *src, int pos, int nbd, int pointX, int pointY, bool hole, int *deltas);
  int* neighborhoodDeltas(int width);
  std::vector<Point*>* approxPolyDP(std::vector<Point*> *contour, double epsilon);
  Image* warp(Image *imageSrc, Image *imageDst, std::vector<Point*> *contour, int warpSize);
  double* getPerspectiveTransform(std::vector<Point *> *src, int size);
  double* square2quad(std::vector<Point*> *src);
  bool isContourConvex(std::vector<Point*> *contour);
  double perimeter(std::vector<Point*> *poly);
  double minEdgeLength(std::vector<Point*> *poly);
  int countNonZero(Image *imageSrc, Square *square);
  short* binaryBorder(Image *imageSrc, short *dst);

  Image* grayscale(Image *imageSrc, Image *imageDst){
    unsigned char *src = imageSrc->data;
    unsigned char *dst = imageDst->data;
    int len = imageSrc->length * 4, i = 0, j = 0;

    for (; i < len; i += 4){
      dst[j ++] = (unsigned char)
        (src[i] * 0.299 + src[i + 1] * 0.587 + src[i + 2] * 0.114 + 0.5);
    }
  
    imageDst->width = imageSrc->width;
    imageDst->height = imageSrc->height;
  
    return imageDst;
  }

  Image* threshold(Image *imageSrc, Image *imageDst, unsigned char threshold){
    unsigned char *src = imageSrc->data;
    unsigned char *dst = imageDst->data;
    int len = imageSrc->length, i;
    unsigned char tab[256];

    for (i = 0; i < 256; ++ i){
      tab[i] = i <= threshold? 0: 255;
    }

    for (i = 0; i < len; ++ i){
      dst[i] = tab[ src[i] ];
    }

    imageDst->width = imageSrc->width;
    imageDst->height = imageSrc->height;

    return imageDst;
  }

  Image* adaptiveThreshold(Image *imageSrc, Image *imageDst,
                           unsigned char kernelSize,
                           short threshold){
    unsigned char *src = imageSrc->data;
    unsigned char *dst = imageDst->data;
    int len = imageSrc->length, i;
    unsigned char tab[768];

    stackBoxBlur(imageSrc, imageDst, kernelSize);

    for (i = 0; i < 768; ++ i){
      tab[i] = (i - 255 <= -threshold)? 255: 0;
    }

    for (i = 0; i < len; ++ i){
      dst[i] = tab[ src[i] - dst[i] + 255 ];
    }

    imageDst->width = imageSrc->width;
    imageDst->height = imageSrc->height;

    return imageDst;  
  }

  int otsu(Image *imageSrc){
    unsigned char *src = imageSrc->data;
    int len = imageSrc->length;
    int threshold = 0, sum = 0, sumB = 0, wB = 0, wF = 0, i;
    double mu, max, between;

    int hist[256];
    max = 0.0;

    for (i = 0; i < 256; ++ i){
      hist[i] = 0;
    }
  
    for (i = 0; i < len; ++ i){
      hist[ src[i] ] ++;
    }

    for (i = 0; i < 256; ++ i){
      sum += hist[i] * i;
    }

    for (i = 0; i < 256; ++ i){
      wB += hist[i];
      if (0 != wB){
    
        wF = len - wB;
        if (0 == wF){
          break;
        }

        sumB += hist[i] * i;
      
        mu = ((double)sumB / wB) - ( (double)(sum - sumB) / wF );

        between = wB * wF * mu * mu;
      
        if (between > max){
          max = between;
          threshold = i;
        }
      }
    }

    return threshold;
  }

  int stackBoxBlurMult[] =
    {1, 171, 205, 293, 57, 373, 79, 137, 241, 27, 391, 357, 41, 19, 283, 265};

  int stackBoxBlurShift[] =
    {0, 9, 10, 11, 9, 12, 10, 11, 12, 9, 13, 13, 10, 9, 13, 13};

  class BlurStack {
    public:
      unsigned char color;
      BlurStack *next;
      BlurStack(): color(0), next(NULL) {}
  };

  Image* stackBoxBlur(Image *imageSrc, Image *imageDst,
                      unsigned char kernelSize){
    unsigned char *src = imageSrc->data;
    unsigned char *dst = imageDst->data;
    int height = imageSrc->height, width = imageSrc->width,
        heightMinus1 = height - 1, widthMinus1 = width - 1,
        size = kernelSize + kernelSize + 1, radius = kernelSize + 1,
        mult = stackBoxBlurMult[kernelSize],
        shift = stackBoxBlurShift[kernelSize],
        sum, pos, start, p, x, y, i;
    unsigned char color;
    BlurStack *stack, *stackStart;

    stack = stackStart = new BlurStack();
    for (i = 1; i < size; ++ i){
      stack = stack->next = new BlurStack();
    }
    stack->next = stackStart;

    pos = 0;

    for (y = 0; y < height; ++ y){
      start = pos;
    
      color = src[pos];
      sum = radius * color;
    
      stack = stackStart;
      for (i = 0; i < radius; ++ i){
        stack->color = color;
        stack = stack->next;
      }
      for (i = 1; i < radius; ++ i){
        stack->color = src[pos + i];
        sum += stack->color;
        stack = stack->next;
      }
  
      stack = stackStart;
      for (x = 0; x < width; ++ x){
        dst[pos ++] = (sum * mult) >> shift;
      
        p = x + radius;
        p = start + (p < widthMinus1? p: widthMinus1);
        sum -= stack->color - src[p];
      
        stack->color = src[p];
        stack = stack->next;
      }
    }

    for (x = 0; x < width; ++ x){
      pos = x;
      start = pos + width;
    
      color = dst[pos];
      sum = radius * color;
    
      stack = stackStart;
      for (i = 0; i < radius; ++ i){
        stack->color = color;
        stack = stack->next;
      }
      for (i = 1; i < radius; ++ i){
        stack->color = dst[start];
        sum += stack->color;
        stack = stack->next;
      
        start += width;
      }
    
      stack = stackStart;
      for (y = 0; y < height; ++ y){
        dst[pos] = (sum * mult) >> shift;
      
        p = y + radius;
        p = x + ( (p < heightMinus1? p: heightMinus1) * width );
        sum -= stack->color - dst[p];
      
        stack->color = dst[p];
        stack = stack->next;
      
        pos += width;
      }
    }

    stack = stackStart;
    for (i = 0; i < size; ++ i){
      BlurStack *next = stack->next;
      delete stack;
      stack = next;
    }

    return imageDst;
  };

  Image* gaussianBlur(Image *imageSrc, Image *imageDst, Image *imageMean,
                      unsigned char kernelSize){
    double *kernel = gaussianKernel(kernelSize);

    imageDst->width = imageSrc->width;
    imageDst->height = imageSrc->height;
  
    imageMean->width = imageSrc->width;
    imageMean->height = imageSrc->height;

    gaussianBlurFilter(imageSrc, imageMean, kernel, kernelSize, true);
    gaussianBlurFilter(imageMean, imageDst, kernel, kernelSize, false);

    delete[] kernel;

    return imageDst;
  }

  Image* gaussianBlurFilter(Image *imageSrc, Image *imageDst, double *kernel,
                            unsigned char kernelSize, bool horizontal){
    unsigned char *src = imageSrc->data;
    unsigned char *dst = imageDst->data;
    int height = imageSrc->height, width = imageSrc->width,
        pos = 0, limit = kernelSize >> 1,
        cur, i, j, k;
    double value;
      
    for (i = 0; i < height; ++ i){
    
      for (j = 0; j < width; ++ j){
        value = 0.0;
    
        for (k = -limit; k <= limit; ++ k){

          if (horizontal){
            cur = pos + k;
            if (j + k < 0){
              cur = pos;
            }
            else if (j + k >= width){
              cur = pos;
            }
          }else{
            cur = pos + (k * width);
            if (i + k < 0){
              cur = pos;
            }
            else if (i + k >= height){
              cur = pos;
            }
          }

          value += kernel[limit + k] * src[cur];
        }
    
        dst[pos ++] = (unsigned char)(horizontal? value: (value + 0.5));
      }
    }

    return imageDst;
  }

  double* gaussianKernel(unsigned char kernelSize){
    double sigma, scale2X, sum;
    int center, i, x;
    double *kernel = new double[kernelSize];

    if ( (kernelSize <= 7) && (kernelSize % 2 == 1) ){
      if (kernelSize == 1) {
        kernel[0] = 1.0;
      } else if (kernelSize == 3) {
        kernel[0] = 0.25;
        kernel[1] = 0.5;
        kernel[2] = 0.25;
      } else if (kernelSize == 5) {
        kernel[0] = 0.0625;
        kernel[1] = 0.25;
        kernel[2] = 0.375;
        kernel[3] = 0.25;
        kernel[4] = 0.0625;
      } else {
        kernel[0] = 0.03125;
        kernel[1] = 0.109375;
        kernel[2] = 0.21875;
        kernel[3] = 0.28125;
        kernel[4] = 0.21875;
        kernel[5] = 0.109375;
        kernel[6] = 0.03125;
      }
    }else{
      center = (int)((kernelSize - 1.0) * 0.5);
      sigma = 0.8 + (0.3 * ((double)center - 1.0) );
      scale2X = -0.5 / (sigma * sigma);
      sum = 0.0;
      for (i = 0; i < kernelSize; ++ i){
        x = i - center;
        sum += kernel[i] = exp(scale2X * x * x);
      }
      sum = 1 / sum;
      for (i = 0; i < kernelSize; ++ i){
        kernel[i] *= sum;
      }  
    }

    return kernel;
  }

  std::vector<std::vector<Point*>* >*
  findContours(Image *imageSrc, short *binary){
    int width = imageSrc->width;
    int height = imageSrc->height;
    std::vector<std::vector<Point*>* > *contours =
      new std::vector<std::vector<Point*>* >;
    short *src;
    int *deltas, pos, pix, nbd, i, j;
    bool outer, hole;

    src = binaryBorder(imageSrc, binary);

    deltas = neighborhoodDeltas(width + 2);

    pos = width + 3;
    nbd = 1;

    for (i = 0; i < height; ++ i, pos += 2){
  
      for (j = 0; j < width; ++ j, ++ pos){
        pix = src[pos];

        if (0 != pix){
          outer = hole = false;

          if (1 == pix && 0 == src[pos - 1]){
            outer = true;
          }
          else if (pix >= 1 && 0 == src[pos + 1]){
            hole = true;
          }

          if (outer || hole){
            ++ nbd;
            contours->push_back( borderFollowing(src, pos, nbd, j, i, hole, deltas) );
          }
        }
      }
    }  

    delete[] deltas;

    return contours;
  }

  std::vector<Point*>* borderFollowing(short *src, int pos, int nbd,
                                       int pointX, int pointY, bool hole, int *deltas){
    std::vector<Point*> *contour = new std::vector<Point*>;
    int pos1, pos3, pos4;
    unsigned int s, s_end, s_prev;
    Point point(pointX, pointY);

    s = s_end = hole? 0: 4;
    do{
      s = (s - 1) & 7;
      pos1 = pos + deltas[s];
      if (src[pos1] != 0){
        break;
      }
    }while(s != s_end);

    if (s == s_end){
      src[pos] = -nbd;
      contour->push_back( new Point(point.x, point.y) );

    }else{
      pos3 = pos;
      s_prev = s ^ 4;

      while(true){
        s_end = s;
    
        do{
          pos4 = pos3 + deltas[++ s];
        }while(src[pos4] == 0);
      
        s &= 7;

        if ( s - 1 < s_end ){
          src[pos3] = -nbd;
        }
        else if (src[pos3] == 1){
          src[pos3] = nbd;
        }

        contour->push_back( new Point(point.x, point.y) );

        s_prev = s;

        point.x += neighborhood[s][0];
        point.y += neighborhood[s][1];

        if ( (pos4 == pos) && (pos3 == pos1) ){
          break;
        }
      
        pos3 = pos4;
        s = (s + 4) & 7;
      }
    }

    return contour;
  }

  int* neighborhoodDeltas(int width){
    int len = NEIGHBORHOOD_NUM, i = 0;
    int *deltas = new int[NEIGHBORHOOD_NUM*2];

    for (; i < len; ++ i){
      deltas[i] = neighborhood[i][0] + (neighborhood[i][1] * width);
    }

    for (i = 0; i < len; ++ i){
      deltas[i + len] = deltas[i];
    }
  
    return deltas;
  }

  std::vector<Point*>* approxPolyDP(std::vector<Point*> *contour,
                                    double epsilon){
    Slice slice(0, 0);
    Slice right_slice(0, 0);
    std::stack<Slice> stack;
    std::vector<Point*> *poly = new std::vector<Point*>;
    int len = contour->size(), dist, max_dist,
        dx, dy, i, j, k;
    Point *pt, *start_pt, *end_pt;
    bool le_eps;

    epsilon *= epsilon;
  
    k = 0;
  
    for (i = 0; i < 3; ++ i){
      max_dist = 0;
    
      k = (k + right_slice.start_index) % len;
      start_pt = (*contour)[k];
      if (++ k == len) {k = 0;}
  
      for (j = 1; j < len; ++ j){
        pt = (*contour)[k];
        if (++ k == len) {k = 0;}
    
        dx = pt->x - start_pt->x;
        dy = pt->y - start_pt->y;
        dist = dx * dx + dy * dy;

        if (dist > max_dist){
          max_dist = dist;
          right_slice.start_index = j;
        }
      }
    }

    if ((double)max_dist <= epsilon){
      poly->push_back( new Point(start_pt->x, start_pt->y) );

    }else{
      slice.start_index = k;
      slice.end_index = (right_slice.start_index += slice.start_index);
  
      right_slice.start_index -= right_slice.start_index >= len? len: 0;
      right_slice.end_index = slice.start_index;
      if (right_slice.end_index < right_slice.start_index){
        right_slice.end_index += len;
      }
    
      Slice s1(right_slice.start_index, right_slice.end_index);
      Slice s2(slice.start_index, slice.end_index);
      stack.push(s1);
      stack.push(s2);
    }

    while(! stack.empty()){
      slice = stack.top();
      stack.pop();

      end_pt = (*contour)[slice.end_index % len];
      start_pt = (*contour)[k = slice.start_index % len];
      if (++ k == len) {k = 0;}
    
      if (slice.end_index <= slice.start_index + 1){
        le_eps = true;
    
      }else{
        max_dist = 0;

        dx = end_pt->x - start_pt->x;
        dy = end_pt->y - start_pt->y;
      
        for (i = slice.start_index + 1; i < slice.end_index; ++ i){
          pt = (*contour)[k];
          if (++ k == len) {k = 0;}
        
          dist = abs( (pt->y - start_pt->y) * dx - (pt->x - start_pt->x) * dy);

          if (dist > max_dist){
            max_dist = dist;
            right_slice.start_index = i;
          }
        }
      
        le_eps = (double)max_dist * max_dist <= epsilon * (dx * dx + dy * dy);
      }
    
      if (le_eps){
        poly->push_back( new Point(start_pt->x, start_pt->y) );

      }else{
        right_slice.end_index = slice.end_index;
        slice.end_index = right_slice.start_index;

        Slice s1(right_slice.start_index, right_slice.end_index);
        Slice s2(slice.start_index, slice.end_index);
        stack.push(s1);
        stack.push(s2);
      }
    }

    std::stack<Slice>().swap(stack);
    return poly;
  }

  Image* warp(Image *imageSrc, Image *imageDst, std::vector<Point*> *contour,
              int warpSize){
    unsigned char *src = imageSrc->data;
    unsigned char *dst = imageDst->data;
    int width = imageSrc->width, height = imageSrc->height,
        pos = 0,
        sx1, sx2, dx1, dx2, sy1, sy2, dy1, dy2, p1, p2, p3, p4, i, j;
    double r, s, t, u, v, w, x, y;
  
    double *m = getPerspectiveTransform(contour, warpSize - 1);

    r = m[8];
    s = m[2];
    t = m[5];
  
    for (i = 0; i < warpSize; ++ i){
      r += m[7];
      s += m[1];
      t += m[4];

      u = r;
      v = s;
      w = t;
    
      for (j = 0; j < warpSize; ++ j){
        u += m[6];
        v += m[0];
        w += m[3];

        x = v / u;
        y = w / u;

        sx1 = (int)x;
        sx2 = (sx1 == width - 1)? sx1: sx1 + 1;
        dx1 = x - sx1;
        dx2 = 1 - dx1;

        sy1 = (int)y;
        sy2 = (sy1 == height - 1)? sy1: sy1 + 1;
        dy1 = y - sy1;
        dy2 = 1 - dy1;

        p1 = p2 = sy1 * width;
        p3 = p4 = sy2 * width;

        dst[pos ++] = (unsigned char)
          (dy2 * (dx2 * src[p1 + sx1] + dx1 * src[p2 + sx2]) +
           dy1 * (dx2 * src[p3 + sx1] + dx1 * src[p4 + sx2]) );

      }
    }

    imageDst->width = warpSize;
    imageDst->height = warpSize;

    delete[] m;

    return imageDst;
  }

  double* getPerspectiveTransform(std::vector<Point *> *src, int size){
    double *rq = square2quad(src);
  
    rq[0] /= size;
    rq[1] /= size;
    rq[3] /= size;
    rq[4] /= size;
    rq[6] /= size;
    rq[7] /= size;
  
    return rq;
  }

  double* square2quad(std::vector<Point*> *src){
    double *sq = new double[9];
    int px, py, dx1, dx2, dy1, dy2, den;
  
    px = (*src)[0]->x - (*src)[1]->x + (*src)[2]->x - (*src)[3]->x;
    py = (*src)[0]->y - (*src)[1]->y + (*src)[2]->y - (*src)[3]->y;
  
    if (0 == px && 0 == py){
      sq[0] = (*src)[1]->x - (*src)[0]->x;
      sq[1] = (*src)[2]->x - (*src)[1]->x;
      sq[2] = (*src)[0]->x;
      sq[3] = (*src)[1]->y - (*src)[0]->y;
      sq[4] = (*src)[2]->y - (*src)[1]->y;
      sq[5] = (*src)[0]->y;
      sq[6] = 0;
      sq[7] = 0;
      sq[8] = 1;

    }else{
      dx1 = (*src)[1]->x - (*src)[2]->x;
      dx2 = (*src)[3]->x - (*src)[2]->x;
      dy1 = (*src)[1]->y - (*src)[2]->y;
      dy2 = (*src)[3]->y - (*src)[2]->y;
      den = dx1 * dy2 - dx2 * dy1;
  
      sq[6] = (double)(px * dy2 - dx2 * py) / den;
      sq[7] = (double)(dx1 * py - px * dy1) / den;
      sq[8] = 1;
      sq[0] = (*src)[1]->x - (*src)[0]->x + sq[6] * (*src)[1]->x;
      sq[1] = (*src)[3]->x - (*src)[0]->x + sq[7] * (*src)[3]->x;
      sq[2] = (*src)[0]->x;
      sq[3] = (*src)[1]->y - (*src)[0]->y + sq[6] * (*src)[1]->y;
      sq[4] = (*src)[3]->y - (*src)[0]->y + sq[7] * (*src)[3]->y;
      sq[5] = (*src)[0]->y;
    }

    return sq;
  }

  bool isContourConvex(std::vector<Point *> *contour){
    int orientation = 0,
        len = contour->size(), i = 0, j = 0,
        dxdy0, dydx0, dx0, dy0, dx, dy;
    Point *cur_pt, *prev_pt;
    bool convex = true;

    prev_pt = (*contour)[len - 1];
    cur_pt = (*contour)[0];

    dx0 = cur_pt->x - prev_pt->x;
    dy0 = cur_pt->y - prev_pt->y;

    for (; i < len; ++ i){
      if (++ j == len) {j = 0;}

      prev_pt = cur_pt;
      cur_pt = (*contour)[j];

      dx = cur_pt->x - prev_pt->x;
      dy = cur_pt->y - prev_pt->y;
      dxdy0 = dx * dy0;
      dydx0 = dy * dx0;

      orientation |= dydx0 > dxdy0? 1: (dydx0 < dxdy0? 2: 3);

      if (3 == orientation){
          convex = false;
          break;
      }

      dx0 = dx;
      dy0 = dy;
    }

    return convex;
  }

  double perimeter(std::vector<Point*> *poly){
    int len = poly->size(), i = 0, j = len - 1,
        dx, dy;
    double p = 0.0;

    for (; i < len; j = i ++){
      dx = (*poly)[i]->x - (*poly)[j]->x;
      dy = (*poly)[i]->y - (*poly)[j]->y;
    
      p += sqrt(dx * dx + dy * dy) ;
    }

    return p;
  }

  double minEdgeLength(std::vector<Point*> *poly){
    int len = poly->size(), i = 0, j = len - 1, 
        min = 0x7FFFFFFF, d, dx, dy;

    for (; i < len; j = i ++){
      dx = (*poly)[i]->x - (*poly)[j]->x;
      dy = (*poly)[i]->y - (*poly)[j]->y;

      d = dx * dx + dy * dy;

      if (d < min){
        min = d;
      }
    }
  
    return sqrt(min);
  }

  int countNonZero(Image *imageSrc, Square *square){
    unsigned char *src = imageSrc->data;
    int height = square->height, width = square->width,
        pos = square->x + (square->y * imageSrc->width),
        span = imageSrc->width - width,
        nz = 0, i, j;
  
    for (i = 0; i < height; ++ i){

      for (j = 0; j < width; ++ j){
    
        if ( 0 != src[pos ++] ){
          ++ nz;
        }
      }
    
      pos += span;
    }

    return nz;
  }

  short* binaryBorder(Image *imageSrc, short *dst){
    unsigned char *src = imageSrc->data;
    int height = imageSrc->height, width = imageSrc->width,
        posSrc = 0, posDst = 0, i, j;

    for (j = -2; j < width; ++ j){
      dst[posDst ++] = 0;
    }

    for (i = 0; i < height; ++ i){
      dst[posDst ++] = 0;
    
      for (j = 0; j < width; ++ j){
        dst[posDst ++] = (0 == src[posSrc ++]? 0: 1);
      }
    
      dst[posDst ++] = 0;
    }

    for (j = -2; j < width; ++ j){
      dst[posDst ++] = 0;
    }
  
    return dst;
  }
}

class ARMarker {
  public:
    int id;
    std::vector<CV::Point*> *corners;
    ARMarker(int _id, std::vector<CV::Point*> *_corners)
      : id(_id), corners(_corners){}
};

class ARDetector {
  public:
    CV::Image *grey;
    CV::Image *thres;
    CV::Image *homography;
    short *binary;

    ARDetector(int width, int height) {
      grey = new CV::Image(width, height, new unsigned char[width * height]);
      thres = new CV::Image(width, height, new unsigned char[width * height]);
      homography = new CV::Image(width, height, new unsigned char[width * height]);
      binary = new short[(width + 2) * (height + 2)];
    }

    std::vector<ARMarker*>* detect(CV::Image *image) {
      CV::grayscale(image, grey);
      CV::adaptiveThreshold(grey, thres, 2, 7);

      std::vector<std::vector<CV::Point*>* > *contours, *candidates, *candidates2;

      contours = CV::findContours(thres, binary);

      candidates = findCandidates(contours, image->width * 0.20, 0.05, 10.0);
      candidates = clockwiseCorners(candidates);
      candidates2 = getNotTooNear(candidates, 10.0);

      std::vector<ARMarker*> *markers = findMarkers(grey, candidates2, 49);

      freeResources(contours, candidates, candidates2);

      return markers;
    }

  private:
    class Pair {
      public:
        int first;
        int second;
        Pair(int _first, int _second): first(_first), second(_second) {}
    };

    void freeResources(
      std::vector<std::vector<CV::Point*>* > *contours,
      std::vector<std::vector<CV::Point*>* > *candidates,
      std::vector<std::vector<CV::Point*>* > *candidates2
    ) {
      for (int i = 0, il = contours->size(); i < il; i++) {
        std::vector<CV::Point*> *points = (*contours)[i];
        for (int j = 0, jl = points->size(); j < jl; j++) {
          delete (*points)[j];
        }
        points->clear();
        std::vector<CV::Point*>().swap(*points);
        delete points;
      }
      contours->clear();
      std::vector<std::vector<CV::Point*>* >().swap(*contours);
      delete contours;

      for (int i = 0, il = candidates->size(); i < il; i++) {
        std::vector<CV::Point*> *points = (*candidates)[i];
        for (int j = 0, jl = points->size(); j < jl; j++) {
          delete (*points)[j];
        }
        points->clear();
        std::vector<CV::Point*>().swap(*points);
        delete points;
      }
      candidates->clear();
      std::vector<std::vector<CV::Point*>* >().swap(*candidates);
      delete candidates;

      candidates2->clear();
      std::vector<std::vector<CV::Point*>* >().swap(*candidates2);
      delete candidates2;

    }

    std::vector<std::vector<CV::Point*>* >*
    findCandidates(std::vector<std::vector<CV::Point*>* > *contours,
                   double minSize, double epsilon, double minLength){
      std::vector<std::vector<CV::Point*>* > *candidates =
        new std::vector<std::vector<CV::Point*>* >;
      int len = contours->size(), i;
      std::vector<CV::Point*> *contour, *poly;

      for (i = 0; i < len; ++ i){
        contour = (*contours)[i];

        if ((double)contour->size() >= minSize){
          poly = CV::approxPolyDP(contour, contour->size() * epsilon);
          bool added = false;

          if ( (4 == poly->size()) && ( CV::isContourConvex(poly) ) ){

            if ( CV::minEdgeLength(poly) >= minLength){
              candidates->push_back(poly);
              added = true;
            }
          }

          if (! added) {
            for (int j = 0, jl = poly->size(); j < jl; j++) {
              delete (*poly)[j];
            }
            poly->clear();
            std::vector<CV::Point*>().swap(*poly);
            delete poly;
          }
        }
      }

      return candidates;
    }

    std::vector<std::vector<CV::Point*>* >*
    clockwiseCorners(std::vector<std::vector<CV::Point*>* > *candidates){
      int len = candidates->size(), dx1, dx2, dy1, dy2, i;
      CV::Point *swap;

      for (i = 0; i < len; ++ i){
        std::vector<CV::Point*> *candidate = (*candidates)[i];
        dx1 = (*candidate)[1]->x - (*candidate)[0]->x;
        dy1 = (*candidate)[1]->y - (*candidate)[0]->y;
        dx2 = (*candidate)[2]->x - (*candidate)[0]->x;
        dy2 = (*candidate)[2]->y - (*candidate)[0]->y;

        if ( (dx1 * dy2 - dy1 * dx2) < 0){
          swap = (*candidate)[1];
          (*candidate)[1] = (*candidate)[3];
          (*candidate)[3] = swap;

        }
      }

      return candidates;
    }

    std::vector<std::vector<CV::Point*>* >*
    getNotTooNear(std::vector<std::vector<CV::Point*>* > *candidates, double minDist){
      std::vector<std::vector<CV::Point*>* > *notTooNear =
        new std::vector<std::vector<CV::Point*>* >;
      bool tooNears[candidates->size()];
      int len = candidates->size(), dist, dx, dy, i, j, k;

      for (i = 0; i < len; ++ i){
        tooNears[i] = false;
      }

      for (i = 0; i < len; ++ i){
  
        for (j = i + 1; j < len; ++ j){
          dist = 0;
      
          std::vector<CV::Point*> *pointsI = (*candidates)[i];
          std::vector<CV::Point*> *pointsJ = (*candidates)[j];
          for (k = 0; k < 4; ++ k){
            dx = (*pointsI)[k]->x - (*pointsJ)[k]->x;
            dy = (*pointsI)[k]->y - (*pointsJ)[k]->y;
      
            dist += dx * dx + dy * dy;
          }
      
          if ( ((double)dist / 4) < (minDist * minDist) ){
      
            if ( CV::perimeter( (*candidates)[i] ) < CV::perimeter( (*candidates)[j] ) ){
              tooNears[i] = true;
            }else{
              tooNears[j] = true;
            }
          }
        }
      }

      for (i = 0; i < len; ++ i){
        if ( !tooNears[i] ){
          notTooNear->push_back( (*candidates)[i] );
        }
      }

      return notTooNear;
    }

    std::vector<ARMarker*>* findMarkers(
        CV::Image* imageSrc,
        std::vector<std::vector<CV::Point*>* > *candidates,
        int warpSize){
      int len = candidates->size(), i;
      std::vector<CV::Point*> *candidate;
      std::vector<ARMarker*> *markers = new std::vector<ARMarker*>;
      ARMarker *marker;

      for (i = 0; i < len; ++ i){
        candidate = (*candidates)[i];

        CV::warp(imageSrc, homography, candidate, warpSize);
  
        CV::threshold(homography, homography, CV::otsu(homography) );

        marker = getMarker(homography, candidate);
        if (marker){
          markers->push_back(marker);
        }
      }
  
      return markers;
    }

    ARMarker* getMarker(CV::Image* imageSrc,
                        std::vector<CV::Point*> *candidate){
      int width = (int)(imageSrc->width / 7),
          minZero = (width * width) >> 1, inc, i, j, k;

      for (i = 0; i < 7; ++ i){
        inc = (0 == i || 6 == i)? 1: 6;
    
        for (j = 0; j < 7; j += inc){
          CV::Square square(j * width, i * width, width, width);
          if ( CV::countNonZero(imageSrc, &square) > minZero){
            return NULL;
          }
        }
      }

      unsigned char **bits = new unsigned char*[5];
      for (i = 0; i < 5; i++) {
        bits[i] = new unsigned char[5];
      }

      unsigned char **rotations[4];
      int distances[4];

      for (i = 0; i < 5; ++ i){
        for (j = 0; j < 5; ++ j){
          CV::Square square((j + 1) * width, (i + 1) * width, width, width);
      
          bits[i][j] = CV::countNonZero(imageSrc, &square) > minZero? 1: 0;
        }
      }

      rotations[0] = bits;
      distances[0] = hammingDistance( rotations[0] );
  
      Pair pair(distances[0], 0);
  
      for (i = 1; i < 4; ++ i){
        rotations[i] = rotate( rotations[i - 1] );
        distances[i] = hammingDistance( rotations[i] );
    
        if (distances[i] < pair.first){
          pair.first = distances[i];
          pair.second = i;
        }
      }

      ARMarker *marker = NULL;

      if (0 == pair.first){
        marker = new ARMarker(
          mat2id( rotations[pair.second] ), 
          rotate2(candidate, 4 - pair.second) );
      }

      for (i = 0; i < 4; i++) {
        for (j = 0; j < 5; j++) {
          delete[] rotations[i][j];
        }
        delete[] rotations[i];
      }

      return marker;
    }

    int hammingDistance(unsigned char **bits){
      unsigned char ids[4][5] = {
        {1,0,0,0,0},
        {1,0,1,1,1},
        {0,1,0,0,1},
        {0,1,1,1,0}
      };
      int dist = 0, sum, minSum, i, j, k;

      for (i = 0; i < 5; ++ i){
        minSum = 0x7FFFFFFF;
    
        for (j = 0; j < 4; ++ j){
          sum = 0;

          for (k = 0; k < 5; ++ k){
              sum += bits[i][k] == ids[j][k]? 0: 1;
          }

          if (sum < minSum){
            minSum = sum;
          }
        }

        dist += minSum;
      }

      return dist;
    }

    unsigned int mat2id(unsigned char **bits){
      unsigned int id = 0;
      int i;

      for (i = 0; i < 5; ++ i){
        id <<= 1;
        id |= bits[i][1];
        id <<= 1;
        id |= bits[i][3];
      }

      return id;
    }

    unsigned char** rotate(unsigned char **src){
      int len = 5;
      unsigned char **dst = new unsigned char*[len];
      int i, j;
  
      for (i = 0; i < len; ++ i){
        dst[i] = new unsigned char[len];
        for (j = 0; j < len; ++ j){
          dst[i][j] = src[len - j - 1][i];
        }
      }

      return dst;
    }

    std::vector<CV::Point*>* rotate2(std::vector<CV::Point*> *src,
                                     int rotation){
      std::vector<CV::Point*> *dst = new std::vector<CV::Point*>;
      int len = src->size(), i;
      for (i = 0; i < len; ++ i){
        CV::Point *point = (*src)[ (rotation + i) % len ];
        dst->push_back(new CV::Point(point->x, point->y));
      }

      return dst;
    }
};

extern "C" {
  ARDetector* newARDetector(int width, int height) {
    return new ARDetector(width, height);
  }

  void freeMarkers(std::vector<ARMarker*> *markers) {
    for (int i = 0, il = markers->size(); i < il; i++) {
      ARMarker *marker = (*markers)[i];
      std::vector<CV::Point*> *points = marker->corners;
      for (int j = 0, jl = marker->corners->size(); j < jl; j++) {
        delete (*points)[j];
      }
      points->clear();
      std::vector<CV::Point*>().swap(*points);
      delete points;
      delete marker;
    }
    markers->clear();
    std::vector<ARMarker*>().swap(*markers);
    delete markers;
  }

  int* detect(ARDetector* detector, unsigned char *src, int width, int height) {
    CV::Image *image = new CV::Image(width, height, src);
    std::vector<ARMarker*> *markers = detector->detect(image);

    int *result = new int[1 + markers->size() * 9];
    int pos = 0;
    result[pos++] = markers->size();
    for (int i = 0, il = markers->size(); i < il; i++) {
      ARMarker *marker = (*markers)[i];
      result[pos++] = marker->id;

      std::vector<CV::Point *> *points = marker->corners;
      for (int j = 0; j < 4; j++) {
        result[pos++] = (*points)[j]->x;
        result[pos++] = (*points)[j]->y;
      }
    }

    delete image;
    freeMarkers(markers);

    return result;
  }

  void freeResult(int *result) {
    delete[] result;
  }
}
