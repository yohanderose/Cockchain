#include "../utils.h"
#include "bits/stdc++.h"
using namespace std;

int main() {
  map<double, int> data;
  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  data[3] = 0;
  data[4] = 0;

  double tmp;
  for (int i = 0; i < 1000; i++) {
    tmp = generate_random(0, 5);
    data[floor(tmp)]++;
  }
  graph_distrib(&data);
  return 0;
}
