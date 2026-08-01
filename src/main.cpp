#include "../src/uci/uci.h"
#include <iostream>

int main() {
  std::cout.setf(std::ios::unitbuf);
  std::cin.setf(std::ios::unitbuf);

  UCI uci;
  uci.uciLoop();

  return 0;
}
