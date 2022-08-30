#include "format.h"

#include <string>

using std::string;

string toString(int a) {
  if (a < 10) {
    return std::to_string(0) + std::to_string(a);
  } else {
    return std::to_string(a);
  }
}

string Format::ElapsedTime(long seconds) {
  int H = seconds / 3600;
  int M = (seconds - 3600 * H) / 60;
  int S = (seconds - 3600 * H - 60 * M);
  string formatted_time = toString(H) + ":" + toString(M) + ":" + toString(S);
  return formatted_time;
}
