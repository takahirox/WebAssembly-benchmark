#include <math.h>
#include <stdbool.h>

double wasm_cos(const double d) {
  return cos(d);
}

double wasm_tan(const double d) {
  return tan(d);
}

double wasm_atanh(const double d) {
  return atanh(d);
}

double wasm_tgamma(const double d) {
  return tgamma(d);
}

double wasm_fisher(const double d) {
  return log((1.0 + d) / (1.0 - d)) / 2.0;
}

double wasm_fisherinv(const double d) {
  const double val = exp(2.0 * value);
  return (val - 1.0) / (val + 1.0);
}

double wasm_normdist_calculate(const double x, const double mean, const double stdDev, const bool isCumulative) {
  const int MAX_TERMS = 5000;
  const double EPSILON = 1e-15;
  const double INV_SQRT_PI = 1.0 / sqrt(M_PI);
  const double INV_SQRT_2PI = 1.0 / sqrt(M_PI * 2.0);
  const double xMinusMean = x - mean;
  if (isCumulative) {
    double xNorm = xMinusMean / stdDev;
    bool isNeg = false;
    if (xNorm < 0) {
      xNorm = -xNorm;
      isNeg = true;
    }

    if (xNorm > 8) {
      return isNeg ? 0.0 : 1.0;
    }

    double sum = 0;
    double twoNPlusOne = 1.0;
    const double zSquare = (xNorm * xNorm) / 2;
    double nextValue = sqrt(zSquare);
    double last;

    int i = 0;
    do {
      last = sum;
      sum += nextValue;
      twoNPlusOne += 2.0;
      nextValue *= (2 * zSquare / twoNPlusOne);
    } while ((nextValue != 0) && (i++ <= MAX_TERMS) && (fabs((last - sum) / sum) > EPISOLON));

    sum *= exp(-1.0 * zSquare);
    if (isNeg) {
      return 1.0 - (0.5 + INV_SQRT_PI * sum);
    } else {
      return 0.5 + INV_SQRT_PI * sum;
    }
  } else {
    return INV_SQRT_2PI / stdDev * exp(-(xMinusMean * xMinusMean) / (2 * stdDev * stdDev));
  }
}

double wasm_gauss(double z) {
  return wasm_normdist_calculate(z, 0, 1, true) - 0.5;
}


