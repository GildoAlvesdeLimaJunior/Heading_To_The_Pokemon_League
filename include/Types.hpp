#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <vector>

// limites e regras do jogo
const int MAX_PARTY = 6;            // pokemons ativos
const int MAX_UNIDADES = 7;         // party + ovos juntos
const int POKEBOLAS_INICIAIS = 7;
const int INCUBADORAS_INICIAIS = 1;
const int DIST_OVO_CHOQUE = 100;    // distancia para chocar ovo
const int XP_POR_VITORIA = 10;
const int XP_POR_DERROTA = 3;
const int DIST_XP = 100;            // +1 xp a cada 100 unidades viajadas
const int XP_EVOLUCAO = 1000;
const int BONUS_EVOLUCAO = 30;      // % de AP/DP por fase de evolucao
const int MIN_HP_CONSCIENTE = 20;   // hp >= 20 -> pode batalhar
const int MIN_HP_CRITICO = 5;       // hp < 5 -> PCM obrigatorio
const int CUSTO_BATALHA = 1;        // unidades de tempo por batalha

enum class PokemonStatus { Consciente, Inconsciente, Ovo, No_PMC };

enum class TipoNo { Regular, Laboratorio, PMC, Ginasio, Estadio };

struct Arestas {
  int para;
  int peso;
};

struct Item {
  int id = 0;
  std::string nome;
  int efeito = 0;  // 0: cura HP | 1: revive
  int valor = 0;
};

struct No {
  int id = -1;
  std::string nome;
  TipoNo tipo = TipoNo ::Regular;
  std::vector<Arestas> vizinhos;

  std::vector<int> pokemons_selvagens;
  std::vector<int> treinadores;
  std::vector<Item> itens;
};

struct Pokemon {
  int id = 0;
  std::string nome;
  std::vector<std::string> evolucoes;
  int num_evolucoes = 3;
  int tipo = 0;

  int hp = 100;
  int max_hp = 100;

  int ap_base = 10;
  int dp_base = 10;
  int ap_bonus = 0;
  int dp_bonus = 0;

  int xp = 0;
  int evo = 1;
  int dist_ovo = 0;
  int dist_ovo = 0;
  int dist_xp = 0;

  PokemonStatus status = PokemonStatus ::Consciente;
  int no_atual = -1;
  int tempo_recuperacao = 0;

  int get_ap() const { return ap_base + ap_bonus + (xp / 10); }
  int get_dp() const { return dp_base + dp_bonus + (xp / 10); }
};

struct Treinador {
  int id = 0;
  std::string nome;
  int no_atual = -1;
  int xp = 0;

  std::vector<Pokemon> party;
  std::vector<Pokemon> ovos;
  std::vector<Pokemon> pc_professor;

  int pokebolas = 7;
  int ervas = 0;
  int insignias = 0;
  bool equipe_rocket = false;
};

struct GameState {
  std::vector<No> nos;
  std::vector<Treinador> treinadores;

  std::vector<Pokemon> selvagens;

  std::vector<Pokemon> catalogo_especies;
  std::vector<int> starters_lab;

  std::vector<std::string> nomes_tipos;
  std::vector<std::vector<double>> matriz_efetividade;

  int laboratorio = -1;
  int estadio = -1;
  std::vector<int> ginasios;
  std::vector<int> pmc;

  // quem e quem na simulacao
  int jogador_id = -1;          // definido pelo StateEngine ao criar o player
  int rocket_id = -1;           // treinador da unidade movel da Equipe Rocket
  int rocket_invisivel_ate = 0; // controle do intervalo de invisibilidade

  int soma_pesos = 0;
  int fator_k = 0;      // K lido do arquivo
  int tempo_decorrido = 0;
  int tempo_minimo = 0;
  int tempo_limite = 0; // fator_k * soma_pesos
};
#endif
