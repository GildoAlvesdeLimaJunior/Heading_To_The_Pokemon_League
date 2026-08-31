#include <algorithm>
#include <queue>

#include "GraphEngine.hpp"

namespace GraphEngine {

// ---------- min-heap manual usada pelo Dijkstra e pelo Prim ----------
struct Par {
  int dist;
  int no;
};

struct FilaMenor {
  std::vector<Par> vet;

  void push(int dist, int no) {
    vet.push_back({dist, no});
    // sobe o elemento ate a posicao certa
    int i = (int)vet.size() - 1;
    while (i > 0) {
      int pai = (i - 1) / 2;
      if (vet[pai].dist <= vet[i].dist) break;
      std::swap(vet[pai], vet[i]);
      i = pai;
    }
  }

  bool vazio() { return vet.empty(); }

  Par pop() {
    Par topo = vet[0];
    vet[0] = vet[(int)vet.size() - 1];
    vet.pop_back();
    // desce o elemento
    int i = 0;
    int n = (int)vet.size();
    while (true) {
      int esq = 2 * i + 1, dir = 2 * i + 2, menor = i;
      if (esq < n && vet[esq].dist < vet[menor].dist) menor = esq;
      if (dir < n && vet[dir].dist < vet[menor].dist) menor = dir;
      if (menor == i) break;
      std::swap(vet[i], vet[menor]);
      i = menor;
    }
    return topo;
  }
};

// ---------- Dijkstra ----------
std::vector<int> dijkstra(const GameState& estado, int origem) {
  int n = (int)estado.nos.size();
  std::vector<int> dist(n, -1);
  if (origem < 0 || origem >= n) return dist;

  FilaMenor h;
  dist[origem] = 0;
  h.push(0, origem);

  while (!h.vazio()) {
    Par p = h.pop();
    int u = p.no;
    if (p.dist > dist[u]) continue;  // entrada velha da heap
    for (const Arestas& a : estado.nos[u].vizinhos) {
      int nd = dist[u] + a.peso;
      if (dist[a.para] == -1 || nd < dist[a.para]) {
        dist[a.para] = nd;
        h.push(nd, a.para);
      }
    }
  }
  return dist;
}

std::vector<int> dijkstraCaminho(const GameState& estado, int origem,
                                 int destino) {
  int n = (int)estado.nos.size();
  std::vector<int> caminho;
  if (origem < 0 || origem >= n || destino < 0 || destino >= n ||
      origem == destino) {
    if (origem == destino && origem >= 0 && origem < n) {
      caminho.push_back(origem);
      return caminho;
    }
    return caminho;
  }

  std::vector<int> dist(n, -1), anterior(n, -1);
  FilaMenor h;
  dist[origem] = 0;
  h.push(0, origem);

  while (!h.vazio()) {
    Par p = h.pop();
    int u = p.no;
    if (p.dist > dist[u]) continue;
    for (const Arestas& a : estado.nos[u].vizinhos) {
      int nd = dist[u] + a.peso;
      if (dist[a.para] == -1 || nd < dist[a.para]) {
        dist[a.para] = nd;
        anterior[a.para] = u;
        h.push(nd, a.para);
      }
    }
  }

  if (dist[destino] == -1) return caminho;  // nao alcancavel

  // reconstrói do destino para a origem
  for (int v = destino; v != -1; v = anterior[v]) caminho.push_back(v);
  std::reverse(caminho.begin(), caminho.end());
  return caminho;
}

// ---------- BFS ----------
std::vector<int> bfs(const GameState& estado, int origem) {
  int n = (int)estado.nos.size();
  std::vector<int> dist(n, -1);
  if (origem < 0 || origem >= n) return dist;

  std::queue<int> fila;
  dist[origem] = 0;
  fila.push(origem);
  while (!fila.empty()) {
    int u = fila.front();
    fila.pop();
    for (const Arestas& a : estado.nos[u].vizinhos) {
      if (dist[a.para] == -1) {
        dist[a.para] = dist[u] + 1;
        fila.push(a.para);
      }
    }
  }
  return dist;
}

// ---------- DFS ----------
void dfs(const GameState& estado, int origem, std::vector<bool>& visit) {
  int n = (int)estado.nos.size();
  if (origem < 0 || origem >= n || visit[origem]) return;
  visit[origem] = true;
  for (const Arestas& a : estado.nos[origem].vizinhos) {
    if (!visit[a.para]) dfs(estado, a.para, visit);
  }
}

// ---------- union-find usada pelo Kruskal ----------
struct UnionFind {
  std::vector<int> pai, tam;
  UnionFind(int n) : pai(n), tam(n, 1) {
    for (int i = 0; i < n; i++) pai[i] = i;
  }
  int achar(int x) {
    if (pai[x] != x) pai[x] = achar(pai[x]);  // compressao de caminho
    return pai[x];
  }
  void unir(int a, int b) {
    a = achar(a);
    b = achar(b);
    if (a == b) return;
    if (tam[a] < tam[b]) std::swap(a, b);
    pai[b] = a;
    tam[a] += tam[b];
  }
};

// aresta com origem e destino explicitos, so pro Kruskal
struct ArestaTrio {
  int u, v, peso;
};

// monta lista unica de arestas (uma direcao so), pra nao contar 2x
std::vector<ArestaTrio> _montarArestas(const GameState& estado) {
  std::vector<ArestaTrio> arestas;
  int n = (int)estado.nos.size();
  for (int u = 0; u < n; u++) {
    for (const Arestas& a : estado.nos[u].vizinhos) {
      if (a.para > u) arestas.push_back({u, a.para, a.peso});
    }
  }
  return arestas;
}

// ---------- Kruskal ----------
int mstKruskal(const GameState& estado) {
  std::vector<ArestaTrio> arestas = _montarArestas(estado);
  // ordena pelo peso
  std::sort(arestas.begin(), arestas.end(),
            [](const ArestaTrio& x, const ArestaTrio& y) {
              return x.peso < y.peso;
            });

  int n = (int)estado.nos.size();
  UnionFind uf(n);
  int total = 0, usadas = 0;
  int m = (int)arestas.size();
  for (int i = 0; i < m && usadas < n - 1; i++) {
    if (uf.achar(arestas[i].u) != uf.achar(arestas[i].v)) {
      uf.unir(arestas[i].u, arestas[i].v);
      total += arestas[i].peso;
      usadas++;
    }
  }
  return total;
}

// ---------- Prim ----------
int mstPrim(const GameState& estado) {
  int n = (int)estado.nos.size();
  if (n == 0) return 0;

  const int INF = 1000000000;
  std::vector<int> custo(n, INF);
  std::vector<bool> dentro(n, false);
  FilaMenor h;
  custo[0] = 0;
  h.push(0, 0);

  int total = 0;
  while (!h.vazio()) {
    Par p = h.pop();
    int u = p.no;
    if (dentro[u]) continue;
    dentro[u] = true;
    total += p.dist;
    for (const Arestas& a : estado.nos[u].vizinhos) {
      if (!dentro[a.para] && a.peso < custo[a.para]) {
        custo[a.para] = a.peso;
        h.push(a.peso, a.para);
      }
    }
  }
  return total;
}

}
