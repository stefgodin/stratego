#include "math.h"

int Math_Clampi(int min, int max, int value) {
  if (value > max) {
    return max;
  }
  if (value < min) {
    return min;
  }
  return value;
}

int Math_Maxi(int v1, int v2) {
  if (v1 > v2) {
    return v1;
  }
  return v2;
}

int Math_Mini(int v1, int v2) {
  if (v1 < v2) {
    return v1;
  }
  return v2;
}
