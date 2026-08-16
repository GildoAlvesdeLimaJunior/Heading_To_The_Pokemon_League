#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <vector>

enum class PokemonStatus { Consciente, Inconsciente, Ovo, No_PMC };

enum class TipoNo { Regular, Laboratorio, PMC, Ginasio, Estadio };

struct Arestas {
  int para;
  int peso;
};

struct No {
  int id = -1;
  std::string nome;
  TipoNo tipo = TipoNo ::Regular;
  std::vector<Arestas> vizinhos;

  std::vector<int> pokemons_selvagens;
  std::vector<int> treinadores;
  std::vector<std::string> itens;
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

  int laboratorio = -1;
  int estadio = -1;
  std::vector<int> ginasios;
  std::vector<int> pmc;

  int soma_pesos = 0;
  int tempo_decorrido = 0;
  int tempo_minimo = 0;
  int tempo_limite = 0;
};
#endif
