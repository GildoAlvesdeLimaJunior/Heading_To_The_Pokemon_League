#ifndef GUI_HPP
#define GUI_HPP

#include <string>
#include <vector>

#include "Types.hpp"

// interface de terminal simples para exibir e capturar entrada
namespace GUI {

// limpa a tela do terminal
void limparTela();

// pausa ate o usuario apertar Enter
void pausar();

// imprime a barra de vida em modo caractere
void barraHP(int hp, int maxHp);

// mostra o nome direito do estado por extenso
std::string nomeStatus(PokemonStatus s);

// mostra o nome do tipo a partir do id
std::string nomeTipo(const GameState& estado, int tipo);

// status geral do jogador: no atual, tempo, insignias, party
void mostrarStatus(const GameState& estado, const Treinador& t);

// exibe os vizinhos do no atual com seus pesos
void mostrarVizinhos(const GameState& estado, int no);

// le um inteiro com prompt. Retorna o inteiro lido.
int lerInt(const std::string& prompt);

// menu principal no topo de cada turno. Retorna a opcao escolhida.
int menuPrincipal();

// escolhe os pokemons pro combate (ate 'qtde' conscientes). Retorna indices.
std::vector<int> escolherPokemons(const Treinador& t, int qtde);

// exibe o estado de uma batalha (pokemon ativo de cada lado + oponente)
void mostrarBatalla(const Pokemon& ativo, const Pokemon& oponente,
                    const std::string& nomeOponente);

// menu de acao do jogador numa batalha. Retorna:
// 1=atacar, 2=defender, 3=usar item, 4=trocar pokemon
int escolherAcaoBatalha();

// menu da fase de captura (selvagem inconsciente). Retorna:
// 1=capturar, 2=deixar ir
int escolherCaptura();

}

#endif
