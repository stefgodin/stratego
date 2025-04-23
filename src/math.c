#include "math.h"

int clampi(int min, int max, int value) {
  if (value > max) {
    return max;
  }
  if (value < min) {
    return min;
  }
  return value;
}

int maxi(int v1, int v2) {
  if (v1 > v2) {
    return v1;
  }
  return v2;
}

int mini(int v1, int v2) {
  if (v1 < v2) {
    return v1;
  }
  return v2;
}
