#include <bits/stdc++.h>

using namespace std;

#include "GraphEngine.hpp"
#include "RNG.hpp"
#include "Types.hpp"

bool carregarMapa(string caminho, GameState& jogo) {
  ifstream arquivo(caminho);
  if (!arquivo.is_open()) {
    cout << "Erro: nao foi possivel abrir '" << caminho << "'" << endl;
    return false;
  }

  // joga os comentarios (# ...) e linhas vazias do arquivo fora antes de ler
  stringstream entrada;
  string linha;
  while (getline(arquivo, linha)) {
    size_t comentario = linha.find('#');
    if (comentario != string::npos) linha = linha.substr(0, comentario);
    entrada << linha << '\n';
  }

  int n, m, k;
  if (!(entrada >> n >> m >> k) || n <= 0 || k <= 0) {
    cout << "Erro: cabecalho do mapa invalido" << endl;
    return false;
  }

  jogo.nos.resize(n);
  jogo.fator_k = k;

  for (int i = 0; i < m; i++) {
    int origem, destino, peso;
    if (!(entrada >> origem >> destino >> peso) || origem < 0 ||
        origem >= n || destino < 0 || destino >= n || peso <= 0) {
      cout << "Erro: aresta invalida no mapa" << endl;
      return false;
    }
    jogo.nos[origem].vizinhos.push_back({destino, peso});
    jogo.nos[destino].vizinhos.push_back({origem, peso});
    jogo.soma_pesos += peso;
  }

  for (int i = 0; i < n; i++) {
    int id;
    string nome, textoTipo;
    if (!(entrada >> id >> nome >> textoTipo) || id < 0 || id >= n) {
      cout << "Erro: no invalido no mapa" << endl;
      return false;
    }

    TipoNo tipo;
    if (textoTipo == "REGULAR")
      tipo = TipoNo ::Regular;
    else if (textoTipo == "LABORATORIO")
      tipo = TipoNo ::Laboratorio;
    else if (textoTipo == "PMC")
      tipo = TipoNo ::PMC;
    else if (textoTipo == "GINASIO")
      tipo = TipoNo ::Ginasio;
    else if (textoTipo == "ESTADIO")
      tipo = TipoNo ::Estadio;
    else {
      cout << "Erro: tipo de no desconhecido '" << textoTipo << "'" << endl;
      return false;
    }

    No& no = jogo.nos[id];
    no.id = id;
    no.nome = nome;
    no.tipo = tipo;

    if (tipo == TipoNo ::Laboratorio)
      jogo.laboratorio = id;
    else if (tipo == TipoNo ::Estadio)
      jogo.estadio = id;
    else if (tipo == TipoNo ::Ginasio)
      jogo.ginasios.push_back(id);
    else if (tipo == TipoNo ::PMC)
      jogo.pmc.push_back(id);
  }

  if (jogo.laboratorio == -1 || jogo.estadio == -1) {
    cout << "Erro: o mapa precisa de um laboratorio e um estadio" << endl;
    return false;
  }

  int qtTipos;
  entrada >> qtTipos;
  if (qtTipos <= 0) {
    cout << "Erro: quantidade de tipos invalida" << endl;
    return false;
  }

  jogo.matriz_efetividade.resize(qtTipos, vector<double>(qtTipos, 1.0));
  for (int i = 0; i < qtTipos; i++) {
    string nomeTipo;
    entrada >> nomeTipo;
    jogo.nomes_tipos.push_back(nomeTipo);
  }
  for (int i = 0; i < qtTipos; i++)
    for (int j = 0; j < qtTipos; j++) entrada >> jogo.matriz_efetividade[i][j];

  int qtEspecies;
  entrada >> qtEspecies;
  if (qtEspecies <= 0) {
    cout << "Erro: quantidade de especies invalida" << endl;
    return false;
  }
  jogo.catalogo_especies.resize(qtEspecies);

  for (int i = 0; i < qtEspecies; i++) {
    int id, fases;
    if (!(entrada >> id >> fases) || id < 0 || id >= qtEspecies || fases <= 0) {
      cout << "Erro: especie invalida no catalogo" << endl;
      return false;
    }

    Pokemon& especie = jogo.catalogo_especies[id];
    especie.id = id;
    especie.num_evolucoes = fases;
    for (int f = 0; f < fases; f++) {
      string fase;
      entrada >> fase;
      if (f == 0) especie.nome = fase;
      especie.evolucoes.push_back(fase);
    }
    entrada >> especie.tipo;
    if (especie.tipo < 0 || especie.tipo >= qtTipos) {
      cout << "Erro: tipo da especie fora do intervalo" << endl;
      return false;
    }
  }

  for (int i = 0; i < 3; i++) {
    int sid;
    if (!(entrada >> sid) || sid < 0 || sid >= qtEspecies) {
      cout << "Erro: starter com especie desconhecida" << endl;
      return false;
    }
    jogo.starters_lab.push_back(sid);
  }

  int qtSelvagens;
  entrada >> qtSelvagens;
  for (int i = 0; i < qtSelvagens; i++) {
    int instId, espId, noId, ap, dp;
    if (!(entrada >> instId >> espId >> noId >> ap >> dp) || espId < 0 ||
        espId >= qtEspecies || noId < 0 || noId >= n || ap < 0 || dp < 0) {
      cout << "Erro: pokemon selvagem invalido" << endl;
      return false;
    }
    Pokemon sel = jogo.catalogo_especies[espId];
    sel.id = instId;
    sel.ap_base = ap;
    sel.dp_base = dp;
    sel.no_atual = noId;
    jogo.selvagens.push_back(sel);
    jogo.nos[noId].pokemons_selvagens.push_back(instId);
  }

  int qtItens;
  entrada >> qtItens;
  for (int i = 0; i < qtItens; i++) {
    int itemId, noId, efeito, valor;
    string nomeItem;
    if (!(entrada >> itemId >> noId >> nomeItem >> efeito >> valor) ||
        noId < 0 || noId >= n || (efeito != 0 && efeito != 1)) {
      cout << "Erro: item invalido no mapa" << endl;
      return false;
    }
    Item item{itemId, nomeItem, efeito, valor};
    jogo.nos[noId].itens.push_back(item);
  }

  int qtTreinadores;
  entrada >> qtTreinadores;
  int idParty = 200;
  for (int i = 0; i < qtTreinadores; i++) {
    Treinador t;
    int lider, qtParty;
    if (!(entrada >> t.id >> t.nome >> t.no_atual >> lider >> qtParty) ||
        t.no_atual < 0 || t.no_atual >= n || qtParty <= 0) {
      cout << "Erro: treinador invalido" << endl;
      return false;
    }

    // a unidade movel da equipe rocket e marcada pelo nome no arquivo
    if (t.nome.find("Rocket") != string::npos) {
      t.equipe_rocket = true;
      jogo.rocket_id = t.id;
    }

    // lideres de ginasio: guardam o ginasio de origem e se movem pela regiao
    t.eh_lider = (lider == 1);
    t.no_base = t.no_atual;
    if (t.eh_lider) {
      t.timer_casa = RNG::aleatorio(10, 40);  // descansa um pouco no inicio
    }

    // o arquivo nao traz stats base por especie, entao o nivel vira AP e DP
    // inicial dos pokemons da party
    for (int p = 0; p < qtParty; p++) {
      int espId, nivel;
      if (!(entrada >> espId >> nivel) || espId < 0 ||
          espId >= qtEspecies || nivel <= 0) {
        cout << "Erro: pokemon da party com especie ou nivel invalido" << endl;
        return false;
      }
      Pokemon pk = jogo.catalogo_especies[espId];
      pk.id = idParty++;
      pk.ap_base = nivel;
      pk.dp_base = nivel;
      pk.no_atual = t.no_atual;
      t.party.push_back(pk);
    }

    jogo.treinadores.push_back(t);
    jogo.nos[t.no_atual].treinadores.push_back(t.id);
  }

  // tempo limite = K * soma_pesos, com K dentro de [10, 15]
  if (k < 10 || k > 15) {
    cout << "Erro: fator K fora do intervalo exigido [10,15] (K=" << k << ")"
         << endl;
    return false;
  }
  jogo.tempo_limite = k * jogo.soma_pesos;

  // valida que o grafo eh conexo / fortemente conexo. O grafo eh nao
  // direcionado, logo conexo == fortemente conexo. Todo no deve ser alcancavel
  // a partir do laboratorio (DFS) e deve conseguir voltar (BFS no reverso).
  {
    vector<bool> alcancaveis(n, false);
    GraphEngine::dfs(jogo, jogo.laboratorio, alcancaveis);
    for (bool v : alcancaveis) {
      if (!v) {
        cout << "Erro: grafo desconexo (nem todo no eh alcancavel a partir do "
                "laboratorio)"
             << endl;
        return false;
      }
    }

    // BFS a partir do laboratorio revisitando a lista de vizinhos confirma a
    // conectividade (mesma validade para grafos nao direcionados)
    vector<bool> visitados(n, false);
    queue<int> fila;
    fila.push(jogo.laboratorio);
    visitados[jogo.laboratorio] = true;
    while (!fila.empty()) {
      int atual = fila.front();
      fila.pop();
      for (const auto& v : jogo.nos[atual].vizinhos) {
        if (!visitados[v.para]) {
          visitados[v.para] = true;
          fila.push(v.para);
        }
      }
    }
    for (bool v : visitados) {
      if (!v) {
        cout << "Erro: grafo nao eh fortemente conexo (no inalcancavel)"
             << endl;
        return false;
      }
    }
  }

  return true;
}
