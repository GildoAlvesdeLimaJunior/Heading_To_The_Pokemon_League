#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "BattleEngine.hpp"
#include "GUI.hpp"
#include "GraphEngine.hpp"
#include "MapParser.hpp"
#include "RNG.hpp"
#include "StateEngine.hpp"

using namespace std;

// ---------------------------------------------------------------------
// funcoes auxiliares do jogo
// ---------------------------------------------------------------------

// escolhe o pokemon inicial: um dos 3 starters fixos do mapa ou 1 aleatorio
// (o aleatorio sorteia de todo o catalogo, excluindo os starters fixos)
vector<int> escolherStarter(const GameState& jogo) {
  cout << "\nProfessor Carvalho: escolha seu pokemon inicial!\n";
  for (int i = 0; i < (int)jogo.starters_lab.size(); i++)
    cout << "  " << i << ") "
         << jogo.catalogo_especies[jogo.starters_lab[i]].nome << "\n";
  cout << "  " << jogo.starters_lab.size() << ") Pokemon aleatorio\n";
  int op;
  do {
    op = GUI::lerInt("Escolha: ");
    if (op < 0 || op > (int)jogo.starters_lab.size())
      cout << "Opcao invalida.\n";
  } while (op < 0 || op > (int)jogo.starters_lab.size());

  int sid = -1;
  if (op >= 0 && op < (int)jogo.starters_lab.size()) {
    sid = jogo.starters_lab[op];
  } else if (op == (int)jogo.starters_lab.size()) {
    // aleatorio que nao seja um dos starters fixos
    vector<int> opcoes;
    for (int i = 0; i < (int)jogo.catalogo_especies.size(); i++) {
      bool ehStarter = false;
      for (int s : jogo.starters_lab)
        if (s == i) ehStarter = true;
      if (!ehStarter) opcoes.push_back(i);
    }
    if (!opcoes.empty())
      sid = opcoes[RNG::aleatorio(0, (int)opcoes.size() - 1)];
    else
      sid = jogo.starters_lab[0];
  } else {
    sid = jogo.starters_lab[0];  // default: primeiro starter
  }

  vector<int> escolhidos;
  escolhidos.push_back(sid);
  cout << "Voce recebeu " << jogo.catalogo_especies[sid].nome << "!\n";
  return escolhidos;
}

// encontra o id de um treinador que nao seja o jogador presente no no
vector<int> treinadoresNoNo(const GameState& jogo, int no, int jogadorId) {
  vector<int> ids;
  for (const Treinador& t : jogo.treinadores) {
    if (t.id == jogadorId) continue;
    if (t.equipe_rocket) continue;
    if (t.no_atual == no) ids.push_back(t.id);
  }
  return ids;
}

// movimenta a unidade da equipe rocket por um vizinho aleatorio
void moverRocket(GameState& jogo) {
  if (jogo.rocket_id == -1) return;
  Treinador* rocket = StateEngine::encontrarTreinadorPorId(jogo, jogo.rocket_id);
  if (!rocket) return;

  // se esta invisivel, nao se move
  if (jogo.tempo_decorrido < jogo.rocket_invisivel_ate) return;

  // move o rocket sem consumir o tempo do jogador (nao usa AvancarTempo)
  const No& atual = jogo.nos[rocket->no_atual];
  if (atual.vizinhos.empty()) return;
  int escolha = RNG::aleatorio(0, (int)atual.vizinhos.size() - 1);
  rocket->no_atual = atual.vizinhos[escolha].para;
}

// teleporta o rocket para um no distante (aleatorio, diferente do atual)
void teleportarRocket(GameState& jogo) {
  if (jogo.rocket_id == -1) return;
  Treinador* rocket = StateEngine::encontrarTreinadorPorId(jogo, jogo.rocket_id);
  if (!rocket) return;
  int n = (int)jogo.nos.size();
  int novo;
  do {
    novo = RNG::aleatorio(0, n - 1);
  } while (novo == rocket->no_atual && n > 1);
  rocket->no_atual = novo;
}

// processa o encontro com a equipe rocket (roubo/teleporte)
void lidarRocket(GameState& jogo, Treinador& jogador) {
  Treinador* rocket = StateEngine::encontrarTreinadorPorId(jogo, jogo.rocket_id);
  if (!rocket) return;
  if (rocket->no_atual != jogador.no_atual) return;
  if (jogo.tempo_decorrido < jogo.rocket_invisivel_ate) return;
  if (StateEngine::verificar_zonasegura(jogo, jogador.no_atual)) return;

  cout << "\n!!! A EQUIPE ROCKET APARECEU !!!\n";
  if (!StateEngine::podeBatalhar(jogador)) {
    cout << "Voce nao tem pokemons conscientes suficientes e foge!\n";
    return;
  }

  vector<int> escolhidos = GUI::escolherPokemons(jogador, 3);

  // monta os escolhidos do rocket (3 primeiros conscientes da party dele)
  vector<int> rEsc;
  for (int i = 0; i < (int)rocket->party.size() && (int)rEsc.size() < 3; i++) {
    if (rocket->party[i].hp >= MIN_HP_CONSCIENTE) rEsc.push_back(i);
  }

  bool venceu = BattleEngine::batalharTreinador(jogo, jogador, escolhidos,
                                                *rocket, rEsc);
  if (venceu) {
    cout << "A Equipe Rocket fugiu e ficou invisivel por um tempo.\n";
    jogo.rocket_invisivel_ate = jogo.tempo_decorrido + RNG::aleatorio(50, 150);
  } else {
    // roubo: insignia ou pokemon
    if (jogador.insignias > 0) {
      jogador.insignias--;
      // mantem o conjunto de insignias coerente com o contador e permite
      // re-conquistar a insígnia ao vencer o lider de novo
      if (!jogador.insignias_ganhas.empty())
        jogador.insignias_ganhas.pop_back();
      cout << "A Equipe Rocket roubou uma das suas insignias!\n";
    } else if (jogador.party.size() > 1) {
      // nunca deixa o jogador sem pokemon; respeita o minimo de 1
      Pokemon roubado = jogador.party.back();
      jogador.party.pop_back();
      cout << "A Equipe Rocket roubou seu " << roubado.nome << "!\n";
    } else {
      cout << "A Equipe Rocket tentou roubar, mas nao conseguiu levar seu "
              "ultimo pokemon!\n";
    }
    cout << "A Equipe Rocket sumiu para um lugar distante.\n";
    teleportarRocket(jogo);
  }
}

// cura um pokemon (nao toca em ovos: curar mudaria o status de um ovo e o
// faria virar um pokemon batalhavel antes de chocar de verdade)
static void curarPokemon(Pokemon& p) {
  if (p.status == PokemonStatus::Ovo) return;
  p.status = PokemonStatus::Consciente;
  p.hp = 100;
  if (p.max_hp > 100) p.hp = p.max_hp;
  p.tempo_recuperacao = 0;
}

// trabalha os eventos ao chegar num no (itens, ovos, ginasios, estadio)
void eventosDoNo(GameState& jogo, Treinador& jogador) {
  No& no = jogo.nos[jogador.no_atual];

  // coleta itens do no (uma unica vez, depois o no fica sem itens)
  if (!no.itens.empty()) {
    cout << "\nVoce encontrou itens:\n";
    for (const Item& it : no.itens) {
      cout << "  - " << it.nome << "\n";
      jogador.inventario.push_back(it);
    }
    cout << "Itens guardados no seu inventario.\n";
    no.itens.clear();
  }

  // chance de achar uma erva pelo caminho
  if (no.tipo == TipoNo::Regular && RNG::chance(0.25)) {
    jogador.ervas++;
    cout << "\nVoce encontrou uma erva pelo caminho! (ervas: "
         << jogador.ervas << ")\n";
  }

  // chance de achar um ovo por aqui (5%)
  if (RNG::chance(0.05) && StateEngine::podePegarOvo(jogador)) {
    cout << "\nVoce encontrou um ovo de pokemon!\n";
    int op;
    do {
      op = GUI::lerInt("Quer pegar? (1=sim, 0=nao): ");
      if (op != 0 && op != 1) cout << "Opcao invalida.\n";
    } while (op != 0 && op != 1);
    if (op == 1) {
      Pokemon ovo;
      // sorteia a especie que o ovo vai virar ao chocar
      if (!jogo.catalogo_especies.empty()) {
        int esp = RNG::aleatorio(0, (int)jogo.catalogo_especies.size() - 1);
        ovo = jogo.catalogo_especies[esp];
        ovo.ap_base = RNG::aleatorio(12, 18);
        ovo.dp_base = RNG::aleatorio(12, 18);
      }
      ovo.status = PokemonStatus::Ovo;
      ovo.dist_ovo = 0;
      // um treinador pode manter mais de um ovo, desde que party + ovos <= 7
      if (StateEngine::podePegarOvo(jogador)) {
        jogador.ovos.push_back(ovo);
        cout << "Ovo colocado na encubadora.\n";
      } else {
        jogador.pc_professor.push_back(ovo);
        cout << "Sem espaco; o ovo foi guardado com o professor.\n";
      }
    }
  }

  // tratamento no PMC: cura TODOS os pokemons (party + professor),
  // independente do estado atual (ovos NAO sao curados: curar mudaria o
  // status deles e os faria virar pokemon batalhavel prematuramente)
  if (no.tipo == TipoNo::PMC) {
    for (Pokemon& p : jogador.party) curarPokemon(p);
    for (Pokemon& p : jogador.pc_professor) curarPokemon(p);
    cout << "Seus pokemons foram tratados no Centro Pokemon!\n";
  }
}

// // movimento do jogador por um vizinho
void moverJogador(GameState& jogo, Treinador& jogador) {
  GUI::mostrarVizinhos(jogo, jogador.no_atual);
  int destino = GUI::lerInt("Para qual no ir? ");

  if (!StateEngine::moverTreinador(jogo, jogador, destino)) {
    cout << "Movimento invalido (no nao eh vizinho).\n";
    return;
  }

  // processa o novo local, a rocket e os lideres
  eventosDoNo(jogo, jogador);
  moverRocket(jogo);
  lidarRocket(jogo, jogador);
}

// procura e batalha contra um pokemon selvagem presente no no
void procurarSelvagem(GameState& jogo, Treinador& jogador) {
  if (StateEngine::verificar_zonasegura(jogo, jogador.no_atual)) {
    cout << "Batalhas sao proibidas em zonas seguras (Laboratorio/PMC).\n";
    return;
  }

  // selvagens do no
  vector<int> selvagensDoNo;
  for (int i = 0; i < (int)jogo.selvagens.size(); i++) {
    const Pokemon& s = jogo.selvagens[i];
    if (s.no_atual == jogador.no_atual && s.status != PokemonStatus::No_PMC) {
      selvagensDoNo.push_back(i);
    }
  }

  if (selvagensDoNo.empty()) {
    cout << "Nao ha pokemon selvagem aqui (ou todos fugiram).\n";
    return;
  }

  if (!StateEngine::podeBatalhar(jogador)) {
    cout << "Voce precisa de ao menos 1 pokemon consciente.\n";
    return;
  }

  vector<int> escolhidos = GUI::escolherPokemons(jogador, 3);
  BattleEngine::batalharSelvagem(jogo, jogador, escolhidos, selvagensDoNo[0]);
}

// inicia um combate contra um treinador/lider presente no no
void desafiarTreinador(GameState& jogo, Treinador& jogador) {
  if (StateEngine::verificar_zonasegura(jogo, jogador.no_atual)) {
    cout << "Batalhas sao proibidas em zonas seguras (Laboratorio/PMC).\n";
    return;
  }

  vector<int> ids = treinadoresNoNo(jogo, jogador.no_atual, jogador.id);
  if (ids.empty()) {
    cout << "Nao ha treinador para desafiar aqui.\n";
    return;
  }

  if (!StateEngine::podeBatalhar(jogador)) {
    cout << "Voce precisa de ao menos 1 pokemon consciente para batalhar.\n";
    return;
  }

  Treinador* alvo = StateEngine::encontrarTreinadorPorId(jogo, ids[0]);
  if (!alvo) return;

  cout << "\nDesafiando " << alvo->nome << "...\n";
  vector<int> escolhidos = GUI::escolherPokemons(jogador, 3);

  // escolhidos do oponente: ate 3 primeiros conscientes da party dele
  vector<int> tEsc;
  for (int i = 0; i < (int)alvo->party.size() && (int)tEsc.size() < 3; i++) {
    if (alvo->party[i].hp >= MIN_HP_CONSCIENTE) tEsc.push_back(i);
  }

  bool venceu = BattleEngine::batalharTreinador(jogo, jogador, escolhidos,
                                                *alvo, tEsc);
  if (venceu && alvo->eh_lider) {
    // derrotar um lider concede a insignia do ginasio dele
    bool jaTem = false;
    for (int id : jogador.insignias_ganhas)
      if (id == alvo->id) jaTem = true;
    if (!jaTem) {
      jogador.insignias_ganhas.push_back(alvo->id);
      jogador.insignias++;
      cout << "\nVoce venceu o lider " << alvo->nome
           << " e ganhou uma insignia! (totais: " << jogador.insignias << ")\n";
    }
  }
}

// tenta a inscricao no estadio; se ok, o jogador vence o jogo
void tentarInscricao(GameState& jogo, Treinador& jogador) {
  if (StateEngine::prazoExpirado(jogo)) {
    cout << "O prazo de inscricao ja expirou. Voce foi inapto.\n";
    return;
  }
  if (!StateEngine::podeInscrever(jogo, jogador)) {
    int necessarias = min(8, (int)jogo.ginasios.size());
    cout << "Voce precisa estar no estadio com " << necessarias
         << " insignias (tem " << jogador.insignias << ").\n";
    return;
  }
  cout << "\nPARABENS, " << jogador.nome
       << "! Voce se classificou para a Liga Pokemon!\n";
  cout << "VOCE VENCEU O JOGO!\n";
  exit(0);
}

// usa um item do inventario em um pokemon da party
void usarItem(GameState& jogo, Treinador& jogador) {
  (void)jogo;
  if (jogador.inventario.empty()) {
    cout << "Seu inventario esta vazio.\n";
    return;
  }
  cout << "\nSeu inventario:\n";
  for (size_t i = 0; i < jogador.inventario.size(); i++) {
    const Item& it = jogador.inventario[i];
    cout << "  [" << i << "] " << it.nome << " (efeito "
         << (it.efeito == 0 ? "cura" : "revive") << " " << it.valor << ")\n";
  }
  int idxItem = GUI::lerInt("Qual item usar? ");
  if (idxItem < 0 || idxItem >= (int)jogador.inventario.size()) {
    cout << "Indice invalido.\n";
    return;
  }

  // escolhe alvo entre os pokemons da party
  cout << "\nEscolha o pokemon alvo (indice):\n";
  for (size_t i = 0; i < jogador.party.size(); i++) {
    const Pokemon& p = jogador.party[i];
    cout << "  [" << i << "] " << p.nome << " (" << GUI::nomeStatus(p.status)
         << ", HP " << p.hp << "/" << p.max_hp << ")\n";
  }
  int alvo = GUI::lerInt("Indice: ");
  if (alvo < 0 || alvo >= (int)jogador.party.size()) {
    cout << "Indice invalido.\n";
    return;
  }

  Item it = jogador.inventario[idxItem];
  Pokemon& p = jogador.party[alvo];

  if (it.efeito == 0) {  // cura HP
    if (p.status == PokemonStatus::Ovo) {
      cout << "Ovos nao podem ser tratados assim.\n";
      return;
    }
    if (p.hp >= p.max_hp) {
      cout << p.nome << " ja esta com a saude maxima.\n";
      return;
    }
    if (p.status == PokemonStatus::Inconsciente || p.hp <= 0) {
      cout << p.nome << " esta inconsciente; item de cura nao revive.\n";
      return;
    }
    p.hp += it.valor;
    if (p.hp > p.max_hp) p.hp = p.max_hp;
    if (p.hp >= MIN_HP_CONSCIENTE) p.status = PokemonStatus::Consciente;
    cout << "Voce usou " << it.nome << " em " << p.nome
         << "! (HP agora " << p.hp << ")\n";
  } else {  // revive
    if (p.status == PokemonStatus::Ovo) {
      cout << "Ovos nao podem ser tratados assim.\n";
      return;
    }
    if (p.status == PokemonStatus::Consciente && p.hp >= MIN_HP_CONSCIENTE) {
      cout << p.nome << " ja esta consciente.\n";
      return;
    }
    p.status = PokemonStatus::Consciente;
    // revive deve deixar o pokemon apto a batalhar (HP >= MIN_HP_CONSCIENTE)
    int novoHp = it.valor;
    if (novoHp < MIN_HP_CONSCIENTE) novoHp = MIN_HP_CONSCIENTE;
    if (novoHp > p.max_hp) novoHp = p.max_hp;
    p.hp = novoHp;
    p.tempo_recuperacao = 0;
    cout << "Voce usou " << it.nome << " e reviveu " << p.nome
         << "! (HP agora " << p.hp << ")\n";
  }
  jogador.inventario.erase(jogador.inventario.begin() + idxItem);
}

// gerencia o pc_professor: guardar/recolher pokemons e ovos em qualquer local
void gerenciarProfessor(GameState& jogo, Treinador& jogador) {
  (void)jogo;
  cout << "\n========== Inventario do Professor ==========\n";
  cout << "Sua party (" << jogador.party.size() << "/6):\n";
  for (size_t i = 0; i < jogador.party.size(); i++)
    cout << "  P[" << i << "] " << jogador.party[i].nome << "\n";
  if (jogador.ovos.empty())
    cout << "  [incubadora vazia]\n";
  else
    for (size_t i = 0; i < jogador.ovos.size(); i++)
      cout << "  O[" << i << "] Ovo (dist " << jogador.ovos[i].dist_ovo << "/"
           << DIST_OVO_CHOQUE << ")\n";

  cout << "Professor (" << jogador.pc_professor.size() << "):\n";
  if (jogador.pc_professor.empty())
    cout << "  (vazio)\n";
  else
    for (size_t i = 0; i < jogador.pc_professor.size(); i++) {
      const Pokemon& pp = jogador.pc_professor[i];
      if (pp.status == PokemonStatus::Ovo)
        cout << "  C[" << i << "] OVO\n";
      else
        cout << "  C[" << i << "] " << pp.nome << "\n";
    }

  cout << "\nO que quer fazer?\n";
  cout << "  1) Guardar pokemon da party no professor\n";
  cout << "  2) Guardar ovo da incubadora no professor\n";
  cout << "  3) Recolher do professor para a party/incubadora\n";
  cout << "  0) Voltar\n";
  int acao = GUI::lerInt("Acao: ");
  if (acao == 0) return;

  if (acao == 1) {  // guardar pokemon da party
    if (jogador.party.empty()) {
      cout << "Voce nao tem pokemons na party.\n";
      return;
    }
    if (jogador.party.size() == 1) {
      cout << "Voce nao pode ficar sem pokemon; deixe ao menos 1 na party.\n";
      return;
    }
    int idx = GUI::lerInt("Qual pokemon guardar? ");
    if (idx < 0 || idx >= (int)jogador.party.size()) {
      cout << "Indice invalido.\n";
      return;
    }
    if (StateEngine::enviarParaCarvalho(jogador, idx))
      cout << "Pokemon guardado com o professor.\n";
    else
      cout << "Nao foi possivel guardar.\n";

  } else if (acao == 2) {  // guardar ovo da incubadora
    if (jogador.ovos.empty()) {
      cout << "Voce nao tem ovos na incubadora.\n";
      return;
    }
    int idx = GUI::lerInt("Qual ovo guardar? ");
    if (idx < 0 || idx >= (int)jogador.ovos.size()) {
      cout << "Indice invalido.\n";
      return;
    }
    jogador.pc_professor.push_back(jogador.ovos[idx]);
    jogador.ovos.erase(jogador.ovos.begin() + idx);
    cout << "Ovo guardado com o professor.\n";

  } else if (acao == 3) {  // recolher do professor
    if (jogador.pc_professor.empty()) {
      cout << "O professor nao tem nada guardado.\n";
      return;
    }
    int idx = GUI::lerInt("Qual indice recolher? ");
    if (idx < 0 || idx >= (int)jogador.pc_professor.size()) {
      cout << "Indice invalido.\n";
      return;
    }
    Pokemon pp = jogador.pc_professor[idx];
    if (pp.status == PokemonStatus::Ovo) {
      // ovo: respeita o limite de party + ovos <= 7
      if (!StateEngine::podePegarOvo(jogador)) {
        cout << "Sem espaco (party + ovos ao limite de 7).\n";
        return;
      }
      jogador.ovos.push_back(pp);
    } else {
      // pokemon: so se o time nao estiver cheio (6)
      if (StateEngine::equipeCheia(jogador)) {
        cout << "Seu time esta cheio; nao da pra recolher.\n";
        return;
      }
      jogador.party.push_back(pp);
    }
    jogador.pc_professor.erase(jogador.pc_professor.begin() + idx);
    cout << "Recolhido com sucesso.\n";
  } else {
    cout << "Acao invalida.\n";
  }
}

// ---------------------------------------------------------------------
// main
// ---------------------------------------------------------------------
int main(int argc, char* argv[]) {
  string arquivo = argc > 1 ? argv[1] : "data/graph.txt";

  GameState jogo;
  if (!carregarMapa(arquivo, jogo)) {
    cout << "Falha ao carregar o mapa, encerrando." << endl;
    return 1;
  }

  int arestas = 0;
  for (const No& no : jogo.nos) arestas += (int)no.vizinhos.size();
  arestas /= 2;

  cout << "== Resumo do mundo carregado ==\n";
  cout << "Nos: " << jogo.nos.size() << " | Arestas: " << arestas
       << " | Soma dos pesos: " << jogo.soma_pesos << "\n";
  cout << "Tempo limite: " << jogo.tempo_limite << " (K=" << jogo.fator_k
       << ")\n";

  // analise de grafos (projeto)
  cout << "\n== Analise de grafos ==\n";
  vector<int> distLab = GraphEngine::dijkstra(jogo, jogo.laboratorio);
  cout << "Distancias (Dijkstra) a partir do laboratorio:\n";
  for (int i = 0; i < (int)jogo.nos.size(); i++)
    cout << "  " << jogo.nos[i].nome << ": " << distLab[i] << "\n";
  cout << "MST (Prim): " << GraphEngine::mstPrim(jogo) << "\n";
  cout << "MST (Kruskal): " << GraphEngine::mstKruskal(jogo) << "\n";

  vector<bool> vis(jogo.nos.size(), false);
  GraphEngine::dfs(jogo, jogo.laboratorio, vis);
  bool conectado = true;
  for (bool v : vis)
    if (!v) conectado = false;
  cout << "Mapa conectado (DFS): " << (conectado ? "SIM" : "NAO") << "\n\n";

  // escolhe starter e cria jogador
  string nome;
  cout << "Digite seu nome de treinador: ";
  cin >> nome;

  vector<int> starters = escolherStarter(jogo);
  int jogadorId = StateEngine::criarJogador(jogo, nome, starters);
  Treinador* jogador = StateEngine::encontrarTreinadorPorId(jogo, jogadorId);
  if (!jogador) return 1;

  cout << "\nBem-vindo, " << jogador->nome << "! Boa jornada!\n";
  GUI::pausar();
  GUI::limparTela();

  // loop principal
  while (true) {
    GUI::mostrarStatus(jogo, *jogador);
    int op = GUI::menuPrincipal();
    bool sair = false;

    switch (op) {
      case 1:
        moverJogador(jogo, *jogador);
        break;
      case 2:
        if (StateEngine::usarErva(*jogador))
          cout << "Voce usou uma erva. +10 HP nos conscientes.\n";
        else
          cout << "Sem ervas disponiveis.\n";
        break;
      case 3:
        procurarSelvagem(jogo, *jogador);
        break;
      case 4:
        desafiarTreinador(jogo, *jogador);
        break;
      case 5:
        if (jogo.nos[jogador->no_atual].tipo == TipoNo::PMC) {
          for (Pokemon& p : jogador->party) curarPokemon(p);
          for (Pokemon& p : jogador->pc_professor) curarPokemon(p);
          cout << "Seus pokemons foram tratados no Centro Pokemon!\n";
        } else {
          cout << "Voce nao esta num Centro Pokemon.\n";
        }
        break;
      case 6:
        tentarInscricao(jogo, *jogador);
        break;
      case 7:
        usarItem(jogo, *jogador);
        break;
      case 8:
        gerenciarProfessor(jogo, *jogador);
        break;
      case 0:
        cout << "Encerrando...\n";
        sair = true;
        break;
      default:
        cout << "Opcao invalida.\n";
        break;
    }

    if (sair) break;

    // checa se o prazo expirou
    if (StateEngine::prazoExpirado(jogo)) {
      cout << "\nO prazo de inscricao expirou e voce nao se classificou.\n";
      cout << "Fim de jogo.\n";
      break;
    }

    // pausa para ler o feedback, depois limpa para o proximo turno
    GUI::pausar();
    GUI::limparTela();
  }

  return 0;
}
