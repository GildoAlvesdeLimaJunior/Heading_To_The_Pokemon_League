#ifndef GRAPH_ENGINE_HPP
#define GRAPH_ENGINE_HPP

#include <vector>

#include "Types.hpp"

// algoritmos puros de teoria dos grafos, implementados manualmente sem libs
namespace GraphEngine {

// Dijkstra com min-heap manual. Retorna a menor distancia de 'origem' a todos.
std::vector<int> dijkstra(const GameState& estado, int origem);

// Dijkstra completo: retorna o caminho (lista de nos) de origem ate destino.
std::vector<int> dijkstraCaminho(const GameState& estado, int origem,
                                 int destino);

// BFS: numero de arestas de 'origem' ate cada no (-1 se nao alcanca).
std::vector<int> bfs(const GameState& estado, int origem);

// DFS que marca (visit = true) todos os alcancaveis a partir de 'origem'.
void dfs(const GameState& estado, int origem, std::vector<bool>& visit);

// MST de Kruskal (union-find manual). Retorna a soma dos pesos da AGM.
int mstKruskal(const GameState& estado);

// MST de Prim (min-heap manual). Retorna a soma dos pesos da AGM.
int mstPrim(const GameState& estado);

}

#endif
