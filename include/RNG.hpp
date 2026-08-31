#ifndef RNG_HPP
#define RNG_HPP

#include <random>

// centraliza a geracao de numeros aleatorios do jogo
namespace RNG {

// gera um inteiro em [min, max] inclusive
int aleatorio(int min, int max);

// gera um double em [0.0, 1.0)
double chance_float();

// retorna true com probabilidade 'p' (p entre 0 e 1)
bool chance(double p);

}

#endif
