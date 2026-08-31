#include <algorithm>
#include <iostream>

#include "GUI.hpp"
#include "RNG.hpp"

namespace GUI {

void limparTela() {
  std::cout << "\033[2J\033[H";  // apaga o terminal e volta pro inicio
}

// rotulo por extenso do tipo de no
static std::string nomeTipoNo(TipoNo tipo) {
  switch (tipo) {
    case TipoNo::Regular:     return "Cidade/Area";
    case TipoNo::Laboratorio: return "Laboratorio do Prof. Carvalho";
    case TipoNo::PMC:         return "Centro Pokemon (PMC)";
    case TipoNo::Ginasio:     return "Ginasio Pokemon";
    case TipoNo::Estadio:     return "Estadio da Liga Pokemon";
  }
  return "Desconhecido";
}

std::string nomeStatus(PokemonStatus s) {
  if (s == PokemonStatus::Consciente) return "Consciente";
  if (s == PokemonStatus::Inconsciente) return "Inconsciente";
  if (s == PokemonStatus::Ovo) return "Ovo";
  return "No_PMC";
}

std::string nomeTipo(const GameState& estado, int tipo) {
  if (tipo < 0 || tipo >= (int)estado.nomes_tipos.size()) return "?";
  return estado.nomes_tipos[tipo];
}

void barraHP(int hp, int maxHp) {
  if (maxHp <= 0) maxHp = 1;
  int cheio = (int)((double)hp / maxHp * 10);
  if (cheio < 0) cheio = 0;
  if (cheio > 10) cheio = 10;
  std::cout << "[";
  for (int i = 0; i < 10; i++) std::cout << (i < cheio ? '#' : '-');
  std::cout << "] " << hp << "/" << maxHp;
}

// imprime os identificadores do que ha num no (lideres, treinadores,
// selvagens, itens, pmc, estadio). Usa 'pre' como prefixo de cada linha.
// 'ignorarId' (se >= 0) e um treinador a nao listar (o proprio jogador).
static void imprimirPresentesNo(const GameState& estado, int no,
                                const std::string& pre, int ignorarId = -1) {
  const No& n = estado.nos[no];
  bool algoAqui = false;
  auto linha = [&](const std::string& txt) {
    if (!algoAqui) { std::cout << pre << "Presente aqui:\n"; algoAqui = true; }
    std::cout << pre << "  - " << txt << "\n";
  };
  for (const Treinador& tr : estado.treinadores) {
    if (tr.id == ignorarId) continue;
    if (tr.no_atual != no) continue;
    if (tr.equipe_rocket &&
        estado.tempo_decorrido >= estado.rocket_invisivel_ate)
      continue;
    if (tr.eh_lider) linha("Lider " + tr.nome);
    else linha("Treinador " + tr.nome);
  }
  for (const Pokemon& s : estado.selvagens) {
    if (s.no_atual != no || s.status == PokemonStatus::No_PMC) continue;
    linha("Selvagem " + s.nome);
  }
  for (const Item& it : n.itens) linha("Item " + it.nome);
  if (n.tipo == TipoNo::PMC) linha("Centro Pokemon (curar)");
  if (n.tipo == TipoNo::Estadio) linha("Inscricao da Liga");
}

void mostrarStatus(const GameState& estado, const Treinador& t) {
  std::cout << "\n================================\n";
  std::cout << "| " << t.nome << " em "
            << estado.nos[t.no_atual].nome << "\n";
  std::cout << "| Tipo: " << nomeTipoNo(estado.nos[t.no_atual].tipo) << "\n";

  // o que ha neste local no momento
  imprimirPresentesNo(estado, t.no_atual, "| ", t.id);

  std::cout << "| Tempo decorrido: " << estado.tempo_decorrido
            << " / " << estado.tempo_limite << "\n";
  std::cout << "| Insignias: " << t.insignias << "/"
            << std::min(8, (int)estado.ginasios.size()) << "\n";  std::cout << "| Pokebolas: " << t.pokebolas
            << " | Ervas: " << t.ervas
            << " | Itens: " << t.inventario.size() << "\n";
  std::cout << "================================\n";

  // party
  for (size_t i = 0; i < t.party.size(); i++) {
    const Pokemon& p = t.party[i];
    if (p.status == PokemonStatus::Ovo) {
      std::cout << "  [" << i << "] OVO (dist "
                << p.dist_ovo << "/" << DIST_OVO_CHOQUE << ")\n";
      continue;
    }
    std::cout << "  [" << i << "] " << p.nome << " ("
              << nomeTipo(estado, p.tipo) << ") "
              << nomeStatus(p.status) << " | ";
    barraHP(p.hp, p.max_hp);
    std::cout << " | AP " << p.get_ap() << " DP " << p.get_dp() << " XP "
              << p.xp << " evo " << p.evo << "\n";
  }

  // ovos nao chocados (fora da party)
  for (const Pokemon& o : t.ovos) {
    std::cout << "  [OVO incubando] dist " << o.dist_ovo << "/"
              << DIST_OVO_CHOQUE << "\n";
  }
}

void mostrarVizinhos(const GameState& estado, int no) {
  std::cout << "Vizinhos de " << estado.nos[no].nome << ":\n";
  for (const Arestas& a : estado.nos[no].vizinhos) {
    int dest = a.para;
    std::cout << "  -> [" << dest << "] " << estado.nos[dest].nome
              << " (" << nomeTipoNo(estado.nos[dest].tipo)
              << ", tempo " << a.peso << ")\n";
    imprimirPresentesNo(estado, dest, "      ");
  }
}

void pausar() {
  std::cout << "\nPressione Enter para continuar...";
  std::cin.ignore(10000, '\n');
  if (std::cin.peek() == '\n') std::cin.get();
}

int lerInt(const std::string& prompt) {
  int v;
  while (true) {
    std::cout << prompt;
    if (std::cin >> v) return v;
    // entrada invalida: limpa o erro e tenta de novo
    std::cin.clear();
    std::cin.ignore(10000, '\n');
    std::cout << "Entrada invalida. Digite um numero.\n";
  }
}

int menuPrincipal() {
  std::cout << "\n--- O que deseja fazer? ---\n";
  std::cout << "  1) Mover-se\n";
  std::cout << "  2) Usar erva (+10 HP p/ conscientes)\n";
  std::cout << "  3) Procurar pokemon selvagem\n";
  std::cout << "  4) Desafiar treinador/lider aqui\n";
  std::cout << "  5) Centro Pokemon (PMC)\n";
  std::cout << "  6) Tentar inscricao no estadio\n";
  std::cout << "  7) Usar item do inventario\n";
  std::cout << "  8) Inventario do Professor\n";
  std::cout << "  0) Sair\n";
  return lerInt("Escolha: ");
}

std::vector<int> escolherPokemons(const Treinador& t, int qtde) {
  std::vector<int> escolhidos;
  int disponiveis = 0;
  for (size_t i = 0; i < t.party.size(); i++)
    if (t.party[i].status == PokemonStatus::Consciente) disponiveis++;
  int qtdFinal = std::min(qtde, disponiveis);

  for (int n = 0; n < qtdFinal; n++) {
    std::cout << "\nEscolha o " << (n + 1) << "o de " << qtdFinal
              << " pokemon consciente (indice):\n";
    for (size_t i = 0; i < t.party.size(); i++) {
      const Pokemon& p = t.party[i];
      if (p.status == PokemonStatus::Consciente)
        std::cout << "  [" << i << "] " << p.nome << " (HP " << p.hp << ")\n";
    }

    while (true) {
      int idx = lerInt("Indice: ");
      if (idx < 0 || idx >= (int)t.party.size()) {
        std::cout << "Indice invalido.\n";
        continue;
      }
      if (t.party[idx].status != PokemonStatus::Consciente) {
        std::cout << "Esse pokemon nao esta consciente.\n";
        continue;
      }
      // evita escolher o mesmo duas vezes
      bool jaTem = false;
      for (int e : escolhidos)
        if (e == idx) jaTem = true;
      if (jaTem) {
        std::cout << "Ja escolhido, escolha outro.\n";
        continue;
      }
      escolhidos.push_back(idx);
      break;
    }
  }
  return escolhidos;
}

void mostrarBatalla(const Pokemon& ativo, const Pokemon& oponente,
                    const std::string& nomeOponente) {
  std::cout << "\n--------------------------------\n";
  std::cout << nomeOponente << ": ";
  barraHP(oponente.hp, oponente.max_hp);
  std::cout << "  " << oponente.nome << " (HP " << oponente.hp << "/"
            << oponente.max_hp << ")\n";
  std::cout << "Voce: ";
  barraHP(ativo.hp, ativo.max_hp);
  std::cout << "  " << ativo.nome << " (HP " << ativo.hp << "/"
            << ativo.max_hp << ")\n";
  std::cout << "--------------------------------\n";
}

int escolherAcaoBatalha() {
  std::cout << "\nO que fazer?\n";
  std::cout << "  1) Atacar\n";
  std::cout << "  2) Defender\n";
  std::cout << "  3) Usar item\n";
  std::cout << "  4) Trocar pokemon\n";
  return lerInt("Acao: ");
}

int escolherCaptura() {
  std::cout << "\nO pokemon selvagem esta inconsciente!\n";
  std::cout << "  1) Capturar\n";
  std::cout << "  2) Deixar ir\n";
  return lerInt("Escolha: ");
}

}
