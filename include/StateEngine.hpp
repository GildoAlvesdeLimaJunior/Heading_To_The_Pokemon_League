#ifndef STATE_ENGINE_HPP
#define STATE_ENGINE_HPP

#include "Types.hpp"

namespace StateEngine {

// Move um treinador para o no adjacente 'destino'. Retorna false se
// 'destino' nao for vizinho do no atual ou se as
// posicoes forem invalidas. atualiza
// treinador.no_atual e chama função de avancar tempo com o peso da aresta usada
bool moveTreinador(GameState& estado, Treinador& treinador, int destino);

// Aplica os efeitos de percorrer 'distancia'
// choco de ovos, regeneracao de HP e contagem dos tempos de
// indisponibilidade. Tambem avanca o relogio global
void AvancarTempo(GameState& estado, Treinador& treinador, int distancia);







//------------------------ dentro de AvancarTempo

// Incrementa a distancia acumulada de cada ovo na incubadora. Ovos que
// atingem 100 unidades chocam e sao movidos para a equipe ativa
// ou para o PC do professor se a equipe ja estiver com 6 pokemons
void andandoChocarOvos(Treinador& treinador, int dist);

// Cura passivamente 1 HP por pokemon Consciente a cada 10 unidades de
// distancia percorrida, respeitando o limite de max_hp. Nao afeta
// pokemons Inconsciente
void andandoRecuperarHP(Treinador& treinador, int dist);

// Decrementa tempo_recuperacao dos pokemons Inconsciente/No_PMC em
// 'distancia' unidades. Ao chegar a zero, o pokemon volta a
// ficar Consciente com HP restaurado para max_hp
void andandoIndisponivelReducao(Treinador& treinador, int dist);

// True se batalhas sao proibidas no no 'no_id' (Laboratorio ou PMC).
bool verificar_zonasegura(const GameState& estado, int no);

// True se a equipe ativa do treinador ja esta no limite de 6 pokemons.
bool equipeCheia(const Treinador& treinador);

// Move o pokemon de indice "indicepokemon" da equipe ativa para o PC do
// Professor Carvalho. A escolha qual pokemon enviar e feita por quem
// chama esta funcao no main
bool enviarParaCarvalho(Treinador& treinador, int indicepokemon);

}

#endif