#ifndef BATTLE_ENGINE_HPP
#define BATTLE_ENGINE_HPP

#include <string>
#include <vector>

#include "Types.hpp"

// motor de combate por turnos, captura e progressao
namespace BattleEngine {

// aplica dano num pokemon e gerencia a troca de status (inconsciente etc)
void aplicarDano(Pokemon& p, int dano);

// derruba o pokemon (inconsciente) com tempo aleatorio em [10,50]
void deixarInconsciente(Pokemon& p);

// ganha XP e verifica evolucao (evolucao so sobe AP/DP se chamado por voce)
void ganharXP(Pokemon& p, int quantidade);

// evolui pra proxima fase se xp >= XP_EVOLUCAO e ainda tem fase
void tentarEvoluir(Pokemon& p);

// calcula o multiplicador de tipo entre atacante e defensor
double multiplicadorTipo(const GameState& estado, int tipoAtk, int tipoDef);

// batalha entre dois treinadores. Retorna true se "desafiante" venceu.
// escolhidosA e escolhidosB sao indices dentro da party de cada lado.
bool batalharTreinador(GameState& estado, Treinador& desafiante,
                       const std::vector<int>& escolhidosA,
                       Treinador& desafiado,
                       const std::vector<int>& escolhidosB);

// batalha contra um pokemon selvagem. Retorna true se foi capturado.
bool batalharSelvagem(GameState& estado, Treinador& treinador,
                      const std::vector<int>& escolhidos,
                      int idxSelvagem);

}

#endif
