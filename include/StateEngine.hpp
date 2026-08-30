#ifndef STATE_ENGINE_HPP
#define STATE_ENGINE_HPP

#include "Types.hpp"

namespace StateEngine {

/*move um treinador para o no adjacente 'destino'. Retorna false se "destino" nao for vizinho do no atual ou se as
posicoes forem invalidas. atualiza treinador.no_atual e chama função de avancar tempo com o peso da aresta usada */
bool moverTreinador(GameState& estado, Treinador& treinador, int destino);

/* aplica os efeitos de percorrer 'distancia'
choco de ovos, regeneracao de HP e contagem dos tempos de
indisponibilidade. Tambem avanca o relogio global */  
//função base!
void AvancarTempo(GameState& estado, Treinador& treinador, int distancia);


//true se ainda cabe mais um ovo
//chamar isso antas de adicionar um ovo encontrado ao vector treinador.ovos.
bool podePegarOvo(const Treinador& treinador);


/*usa uma erva do inventario para curar +10 em todos os pokemons conscientes da equipe
retorna false se nao houver erva disponivel. */
bool usarErva(Treinador& treinador);


/*Devolve a distância em número de arestas de raiz até cada nó, 
-1 para quem não é alcançável, mas o intuito é que seja todo mundo alcançavel, eu espero,
use sabendo qual o vértice ou quais vértices que o jogador quer saber a distancia*/
std::vector<int> acharQuantidadePontos(GameState &estado, int raiz);



/* cria o treinador jogador, posiciona no laboratorio, monta a party a
partir dos ids em "especies", que são indices em estado.catalogo_especies, 
registra em estado.treinadores e define estado.jogador_id, define a escolha de quais
os starters é feita por quem chama esta funcao, nessa função só faz a
criacao com base nisso e retorna o id do jogaodr criado. */
int criarJogador(GameState& estado, const std::string& nome, const std::vector<int> &especies);


// busca um treinador pelo id  Retorna nullptr se nao achar.
Treinador* encontrarTreinadorPorId(GameState& estado, int id);



//checar se o jogador tem as condições necessárias de batalhar que é ter 3 ou mais pokemon conscientes
bool podeBatalhar(Treinador &treinador);



//olha se passou do tempo limite para a inscrição no estádio
bool prazoExpirado(GameState &estado);




bool podeInscrever(GameState &estado, Treinador &treinador);



//------------------------ dentro de AvancarTempo

/* Incrementa a distancia acumulada de cada ovo na incubadora. Ovos que
atingem 100 unidades chocam e sao movidos para a equipe ativa
ou para o PC do professor se a equipe ja estiver com 6 pokemons */
void andandoChocarOvos(Treinador& treinador, int dist);

/* Cura passivamente 1 HP por pokemon consciente a cada 10 unidades de
distancia percorrida, nao afeta pokemons inconscientes */ 
void andandoRecuperarHP(Treinador& treinador, int dist);

/* decrementa tempo_recuperacao dos pokemons Inconsciente/No_PMC em
"distancia" unidades. Ao chegar a zero, o pokemon volta a
ficar Consciente com HP restaurado para max_hp */
void andandoIndisponivelReducao(Treinador& treinador, int dist);

// true se batalhas são proibidas no no "no_id", seja laboratorio ou pmc.
bool verificar_zonasegura(const GameState& estado, int no);

// true se a equipe ativa do treinador ja esta no limite de 6 pokemons.
bool equipeCheia(const Treinador& treinador);

/* move o pokemon de indice "indicepokemon" da equipe ativa para o PC do
professor. escolha é  feita por quem chama esta funcao  nomain */
bool enviarParaCarvalho(Treinador& treinador, int indicepokemon);


/* dá +1 de xp pra cada 100 de distância percorrida pra todos os pokemons
independente de status */
void andandoGanharXP(Treinador &treinador, int dist);

}

#endif