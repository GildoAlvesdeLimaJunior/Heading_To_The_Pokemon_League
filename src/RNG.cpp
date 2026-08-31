#include <random>

#include "RNG.hpp"

namespace RNG {

// um gerador unico pra todo o jogo, com seed variada por execucao
static std::mt19937 gerador = []() {
  std::random_device rd;
  return std::mt19937(rd());
}();

int aleatorio(int min, int max) {
  if (min > max) return min;
  std::uniform_int_distribution<int> dist(min, max);
  return dist(gerador);
}

double chance_float() {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  return dist(gerador);
}

bool chance(double p) {
  if (p <= 0.0) return false;
  if (p >= 1.0) return true;
  return chance_float() < p;
}

}
