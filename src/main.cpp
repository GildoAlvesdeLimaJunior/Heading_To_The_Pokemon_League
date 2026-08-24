#include <iostream>
#include <string>

#include "MapParser.hpp"

int main(int argc, char* argv[]) {
  std::string arquivo = argc > 1 ? argv[1] : "data/graph.txt";

  GameState jogo;
  if (!carregarMapa(arquivo, jogo)) {
    std::cout << "Falha ao carregar o mapa, encerrando." << std::endl;
    return 1;
  }

  int arestas = 0;
  for (const No& no : jogo.nos) arestas += (int)no.vizinhos.size();
  arestas /= 2;

  int qtItens = 0;
  for (const No& no : jogo.nos) qtItens += (int)no.itens.size();

  std::cout << "== Resumo do mundo carregado ==\n";
  std::cout << "Nos: " << jogo.nos.size() << " | Arestas: " << arestas
            << " | Soma dos pesos: " << jogo.soma_pesos << "\n";
  std::cout << "Tempo limite para se qualificar: " << jogo.tempo_limite
            << " (K=" << jogo.fator_k << ", soma=" << jogo.soma_pesos
            << ")\n\n";

  std::cout << "Laboratorio: no " << jogo.laboratorio << " ("
            << jogo.nos[jogo.laboratorio].nome << ")\n";
  std::cout << "Estadio: no " << jogo.estadio << " ("
            << jogo.nos[jogo.estadio].nome << ")\n";
  std::cout << "Ginasios: " << jogo.ginasios.size()
            << " | Centros Pokemon: " << jogo.pmc.size() << "\n\n";

  std::cout << "Tipos elementais:";
  for (const std::string& t : jogo.nomes_tipos) std::cout << " " << t;
  std::cout << "\nMatriz de efetividade:\n";
  for (const auto& linhaMatriz : jogo.matriz_efetividade) {
    for (double v : linhaMatriz) std::cout << v << " ";
    std::cout << "\n";
  }

  std::cout << "\nEspecies catalogadas: " << jogo.catalogo_especies.size()
            << "\n";
  std::cout << "Starters do laboratorio:";
  for (int s : jogo.starters_lab)
    std::cout << " " << jogo.catalogo_especies[s].nome;
  std::cout << "\n";

  std::cout << "Pokemons selvagens: " << jogo.selvagens.size()
            << " | Itens pelo mapa: " << qtItens << "\n";
  if (jogo.rocket_id != -1) {
    const Treinador& r = jogo.treinadores[jogo.rocket_id - 1];
    std::cout << "Equipe Rocket: #" << r.id << " " << r.nome << " @ "
              << jogo.nos[r.no_atual].nome << "\n";
  }

  std::cout << "\nTreinadores:\n";
  for (const Treinador& t : jogo.treinadores) {
    std::cout << "  #" << t.id << " " << t.nome << " @ "
              << jogo.nos[t.no_atual].nome << " | party:";
    for (const Pokemon& p : t.party)
      std::cout << " " << p.nome << " (ap " << p.get_ap() << ", dp "
                << p.get_dp() << ")";
    std::cout << "\n";
  }

  return 0;
}
