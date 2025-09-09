#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  long long decimal;
  decimal = strtoll("2147483647", NULL, 10);
  unsigned long mask = 0xFFFFFFFF;
  int finalOutput = (decimal & mask);
  printf("%d", finalOutput);
}