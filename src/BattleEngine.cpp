#include <iostream>
#include <sstream>

#include "BattleEngine.hpp"
#include "GUI.hpp"
#include "RNG.hpp"
#include "StateEngine.hpp"

namespace BattleEngine {

// aplica dano e pode derrubar o pokemon
void aplicarDano(Pokemon& p, int dano) {
  if (dano < 0) dano = 0;
  p.hp -= dano;
  if (p.hp <= 0) {
    // derrubado: o chamador usa deixarInconsciente (faint)
    p.hp = 0;
    return;
  }
  if (p.hp >= MIN_HP_CONSCIENTE) {
    p.status = PokemonStatus::Consciente;   // ainda consciente (HP >= 20)
  } else if (p.hp < MIN_HP_CRITICO) {
    // muito machucado (HP < 5): deve ser tratado no PMC
    p.status = PokemonStatus::No_PMC;
    if (p.tempo_recuperacao == 0)
      p.tempo_recuperacao = RNG::aleatorio(10, 50);
  } else {
    // inconsciente (5 <= HP < 20): indisponivel ate se recuperar
    p.status = PokemonStatus::Inconsciente;
    if (p.tempo_recuperacao == 0)
      p.tempo_recuperacao = RNG::aleatorio(10, 50);
  }
}

void deixarInconsciente(Pokemon& p) {
  // mantem o HP real (min. 0) e marca o pokemon como inconsciente;
  // nao infla o HP de volta pra cima
  if (p.hp > MIN_HP_CONSCIENTE - 1) p.hp = MIN_HP_CONSCIENTE - 1;
  if (p.hp < 0) p.hp = 0;
  p.status = PokemonStatus::Inconsciente;
  p.tempo_recuperacao = RNG::aleatorio(10, 50);
}

void ganharXP(Pokemon& p, int quantidade) {
  p.xp += quantidade;
}

// evolui se atingiu 1000 xp e ainda tem fase pra subir
void tentarEvoluir(Pokemon& p) {
  // so evolui com pokemon consciente e que ainda nao esta no topo
  if (p.xp < XP_EVOLUCAO) return;
  if (p.evo >= (int)p.evolucoes.size()) return;

  p.evo++;
  // nova fase ganha 30% de ap e dp
  p.ap_base = (int)(p.ap_base * 1.3);
  p.dp_base = (int)(p.dp_base * 1.3);
  // consome 1000 xp da "fatia" acumulada, mantendo o resto
  p.xp -= XP_EVOLUCAO;

  std::cout << "  >>> " << p.nome << " evoluiu para " << p.evolucoes[p.evo - 1]
            << "!\n";
  p.nome = p.evolucoes[p.evo - 1];
}

double multiplicadorTipo(const GameState& estado, int tipoAtk, int tipoDef) {
  if (tipoAtk < 0 || tipoAtk >= (int)estado.matriz_efetividade.size()) return 1.0;
  if (tipoDef < 0 || tipoDef >= (int)estado.matriz_efetividade[tipoAtk].size())
    return 1.0;
  return estado.matriz_efetividade[tipoAtk][tipoDef];
}

// AP efetivo do pokemon somando uma parte do xp do treinador
static int apEfetivo(const Pokemon& p, const Treinador& t) {
  return p.get_ap() + t.xp / 20;
}

static int dpEfetivo(const Pokemon& p, const Treinador& t) {
  return p.get_dp() + t.xp / 20;
}

// escreve uma linha: guarda no log (se houver) e sempre mostra na tela
static const int MAX_LOG = 40;
static void escreverSaida(std::vector<std::string>* log, std::string msg) {
  if (log) {
    log->push_back(msg);
    if ((int)log->size() > MAX_LOG) log->erase(log->begin());
  }
  std::cout << msg << "\n";
}

// procura o proximo pokemon consciente na party, -1 se nao tiver
static int proximoDisponivel(const Treinador& t,
                             const std::vector<int>& escolhidos) {
  for (int idx : escolhidos) {
    if (idx >= 0 && idx < (int)t.party.size()) {
      if (t.party[idx].hp >= MIN_HP_CONSCIENTE) return idx;
    }
  }
  return -1;
}

// calcula e aplica o dano de 'atk' em 'def'. Retorna o dano causado.
// Se 'defendendo' for true, o dano e reduzido pela metade.
static int calcularDano(const GameState& estado, Pokemon& atk, Treinador& tAtk,
                        Pokemon& def, Treinador& tDef, bool defendendo,
                        std::vector<std::string>* log = nullptr) {
  int dano = apEfetivo(atk, tAtk) - dpEfetivo(def, tDef);

  int delta = atk.xp - def.xp;
  if (delta < 0) delta = -delta;
  double chanceEsquiva = delta / 1000.0;
  if (chanceEsquiva > 0.5) chanceEsquiva = 0.5;
  if (RNG::chance(chanceEsquiva)) {
    escreverSaida(log, "  " + def.nome + " esquivou do ataque!");
    return 0;
  }

  double chanceCrit = delta / 1000.0;
  if (chanceCrit > 0.5) chanceCrit = 0.5;
  bool crit = RNG::chance(chanceCrit);

  double mult = multiplicadorTipo(estado, atk.tipo, def.tipo);
  int danoBase = dano < 0 ? 0 : dano;
  if (danoBase == 0) danoBase = 1;  // minimo base (evita loop de "defesa alta")
  int danoFinal = (int)(danoBase * mult);
  if (crit) danoFinal *= 2;
  if (danoFinal < 1) danoFinal = 1;  // minimo final (apos tipo/critico)
  if (defendendo) danoFinal = danoFinal / 2;
  if (danoFinal < 1) danoFinal = 1;

  std::string msg = "  " + atk.nome + " ataca " + def.nome + " causando " +
                    std::to_string(danoFinal) + " de dano";
  if (crit) msg += " (CRITICO!)";
  if (mult != 1.0) {
    std::ostringstream oss;
    oss << mult;
    msg += " [tipo x" + oss.str() + "]";
  }
  escreverSaida(log, msg);
  aplicarDano(def, danoFinal);
  // bonus de AP/DP ao derrotar um oponente com XP >= XP do atacante
  if (def.hp < MIN_HP_CONSCIENTE && def.xp >= atk.xp) {
    atk.ap_bonus++;
    atk.dp_bonus++;
  }
  return danoFinal;
}

// pede pra escolher um item e um pokemon alvo; consome o item se usar
static bool usarItemBatalha(Treinador& treinador) {
  if (treinador.inventario.empty()) {
    std::cout << "Seu inventario esta vazio.\n";
    return false;
  }
  std::cout << "Seu inventario:\n";
  for (size_t i = 0; i < treinador.inventario.size(); i++)
    std::cout << "  [" << i << "] " << treinador.inventario[i].nome << "\n";
  int idx = GUI::lerInt("Qual item (ou -1 p/ cancelar)? ");
  if (idx < 0 || idx >= (int)treinador.inventario.size()) {
    std::cout << "Cancelado.\n";
    return false;
  }
  Item it = treinador.inventario[idx];

  // escolhe o pokemon alvo (qualquer um da party que se encaixe no efeito)
  std::vector<int> alvos;
  if (it.efeito == 0) {
    // cura pokemon consciente ferido OU gravemente machucado (No_PMC)
    for (int i = 0; i < (int)treinador.party.size(); i++) {
      Pokemon& p = treinador.party[i];
      if (p.hp < p.max_hp &&
          (p.status == PokemonStatus::Consciente ||
           p.status == PokemonStatus::No_PMC))
        alvos.push_back(i);
    }
  } else if (it.efeito == 1) {
    // revive pokemon inconsciente OU gravemente machucado (No_PMC)
    for (int i = 0; i < (int)treinador.party.size(); i++) {
      Pokemon& p = treinador.party[i];
      if (p.status == PokemonStatus::Inconsciente ||
          p.status == PokemonStatus::No_PMC)
        alvos.push_back(i);
    }
  }

  if (alvos.empty()) {
    if (it.efeito == 0)
      std::cout << "Nenhum pokemon precisa de cura.\n";
    else
      std::cout << "Nenhum pokemon inconsciente/ferido para reviver.\n";
    return false;
  }

  int alvo = -1;
  if (alvos.size() > 1) {
    std::cout << "Em qual pokemon? (indice):\n";
    for (int i = 0; i < (int)treinador.party.size(); i++) {
      const Pokemon& p = treinador.party[i];
      if ((it.efeito == 0 && p.hp < p.max_hp &&
           (p.status == PokemonStatus::Consciente ||
            p.status == PokemonStatus::No_PMC)) ||
          (it.efeito == 1 &&
           (p.status == PokemonStatus::Inconsciente ||
            p.status == PokemonStatus::No_PMC)))
        std::cout << "  [" << i << "] " << p.nome << " (HP " << p.hp << "/"
                  << p.max_hp << ")\n";
    }
    alvo = GUI::lerInt("Indice (ou -1 p/ cancelar)? ");
    if (alvo < 0) { std::cout << "Cancelado.\n"; return false; }
  } else {
    alvo = alvos[0];
  }
  if (alvo < 0 || alvo >= (int)treinador.party.size()) {
    std::cout << "Indice invalido.\n";
    return false;
  }

  Pokemon& p = treinador.party[alvo];
  if (it.efeito == 0) {
    p.hp = std::min(p.max_hp, p.hp + it.valor);
    // se saiu do estado grave (hp >= consciente), volta a ser consciente
    if (p.status == PokemonStatus::No_PMC && p.hp >= MIN_HP_CONSCIENTE) {
      p.status = PokemonStatus::Consciente;
      p.tempo_recuperacao = 0;
    }
    std::cout << p.nome << " recuperou " << it.valor << " de HP ("
              << p.hp << "/" << p.max_hp << ").\n";
  } else if (it.efeito == 1) {
    p.status = PokemonStatus::Consciente;
    // revive deve deixar o pokemon apto a batalhar (HP >= MIN_HP_CONSCIENTE)
    int novoHp = std::min(p.max_hp, it.valor);
    if (novoHp < MIN_HP_CONSCIENTE) novoHp = MIN_HP_CONSCIENTE;
    if (novoHp > p.max_hp) novoHp = p.max_hp;
    p.hp = novoHp;
    p.tempo_recuperacao = 0;
    std::cout << p.nome << " foi revivido com " << p.hp << " de HP!\n";
  }
  treinador.inventario.erase(treinador.inventario.begin() + idx);
  return true;
}

// pede pra trocar o pokemon ativo por outro dos escolhidos. Retorna novo indice.
static int trocarPokemonBatalha(const Treinador& treinador,
                                const std::vector<int>& escolhidos,
                                int atual) {
  std::vector<int> opcoes;
  for (int i : escolhidos) {
    if (i >= 0 && i < (int)treinador.party.size() &&
        treinador.party[i].status == PokemonStatus::Consciente &&
        treinador.party[i].hp >= MIN_HP_CONSCIENTE && i != atual)
      opcoes.push_back(i);
  }
  if (opcoes.empty()) {
    std::cout << "Nao ha outro pokemon consciente para trocar.\n";
    return atual;
  }
  std::cout << "Escolha para quem trocar (indice):\n";
  for (int i : opcoes)
    std::cout << "  [" << i << "] " << treinador.party[i].nome << " (HP "
              << treinador.party[i].hp << ")\n";
  int alvo = GUI::lerInt("Indice (ou -1 p/ cancelar)? ");
  if (alvo < 0) return atual;
  for (int i : opcoes)
    if (i == alvo) return alvo;
  std::cout << "Indice invalido.\n";
  return atual;
}

bool batalharTreinador(GameState& estado, Treinador& desafiante,
                       const std::vector<int>& escolhidosA,
                       Treinador& desafiado,
                       const std::vector<int>& escolhidosB) {
  std::cout << "\n===== BATALHA: " << desafiante.nome << " vs "
            << desafiado.nome << " =====\n";

  // cada batalha custa uma unidade de tempo percorrido
  estado.tempo_decorrido += CUSTO_BATALHA;

  // ativo de cada lado (desafiado comeca atacando)
  int idxA = proximoDisponivel(desafiante, escolhidosA);
  int idxB = proximoDisponivel(desafiado, escolhidosB);

  bool defendendoJogador = false;
  bool defendendoOponente = false;

  // historico das acoes da batalha (mantido mesmo com a tela limpa)
  std::vector<std::string> log;

  int limiteTurnos = 120;

  while (limiteTurnos-- > 0) {
    if (idxA == -1) idxA = proximoDisponivel(desafiante, escolhidosA);
    if (idxB == -1) idxB = proximoDisponivel(desafiado, escolhidosB);
    if (idxA == -1 || idxB == -1) break;

    // limpa a tela mas mantem o historico das acoes (retornos de cada acao)
    GUI::limparTela();
    int inicio = log.size() > 8 ? (int)log.size() - 8 : 0;
    for (int i = inicio; i < (int)log.size(); i++)
      std::cout << log[i] << "\n";
    GUI::mostrarBatalla(desafiante.party[idxA], desafiado.party[idxB],
                        desafiado.nome);

    // --- turno do oponente (desafiado) ataca primeiro (spec) ---
    if (idxB != -1 && idxA != -1) {
      // 25% de chance de defender
      if (RNG::chance(0.25)) {
        defendendoOponente = true;
        escreverSaida(&log, desafiado.party[idxB].nome + " se defendeu!");
      } else {
        calcularDano(estado, desafiado.party[idxB], desafiado,
                     desafiante.party[idxA], desafiante, defendendoJogador,
                     &log);
        defendendoJogador = false;
        if (desafiante.party[idxA].hp < MIN_HP_CONSCIENTE) {
          deixarInconsciente(desafiante.party[idxA]);
          idxA = proximoDisponivel(desafiante, escolhidosA);
          if (idxA != -1)
            escreverSaida(&log, "Voce manda " + desafiante.party[idxA].nome +
                                    "!");
        }
      }
    }

    // fim da batalha? (oponente pode ter derrubado o ultimo pokemon)
    if (proximoDisponivel(desafiado, escolhidosB) == -1 ||
        proximoDisponivel(desafiante, escolhidosA) == -1)
      break;
    if (idxA == -1 || idxB == -1) break;

    // --- turno do jogador (desafiante) ---
    int acao = GUI::escolherAcaoBatalha();

    if (acao == 1) {  // atacar
      calcularDano(estado, desafiante.party[idxA], desafiante,
                   desafiado.party[idxB], desafiado, defendendoOponente, &log);
      if (desafiado.party[idxB].hp < MIN_HP_CONSCIENTE) {
        deixarInconsciente(desafiado.party[idxB]);
        idxB = proximoDisponivel(desafiado, escolhidosB);
        if (idxB != -1)
          escreverSaida(&log, desafiado.nome + " mandou " +
                                  desafiado.party[idxB].nome + "!");
      }
      defendendoOponente = false;
    } else if (acao == 2) {  // defender
      defendendoJogador = true;
      escreverSaida(&log, desafiante.party[idxA].nome + " se defendeu!");
    } else if (acao == 3) {  // usar item
      usarItemBatalha(desafiante);
    } else if (acao == 4) {  // trocar pokemon (entre os escolhidos)
      int novo = trocarPokemonBatalha(desafiante, escolhidosA, idxA);
      if (novo != idxA) {
        idxA = novo;
        escreverSaida(&log, "Voce mandou " + desafiante.party[idxA].nome + "!");
      }
    } else {
      escreverSaida(&log, "Acao invalida.");
    }

    // fim da batalha?
    bool acabouA = (proximoDisponivel(desafiante, escolhidosA) == -1);
    bool acabouB = (proximoDisponivel(desafiado, escolhidosB) == -1);
    if (acabouA || acabouB) {
      bool venceu = acabouB;  // desafiante vence se o desafiado nao tem mais
      std::cout << (venceu ? "\n>>> " + desafiante.nome + " venceu a batalha!\n"
                           : "\n>>> " + desafiado.nome + " venceu a batalha!\n");

      for (int i : escolhidosA) {
        if (i >= 0 && i < (int)desafiante.party.size()) {
          Pokemon& p = desafiante.party[i];
          if (venceu) { ganharXP(p, XP_POR_VITORIA); tentarEvoluir(p); }
          else ganharXP(p, XP_POR_DERROTA);
        }
      }
      for (int i : escolhidosB) {
        if (i >= 0 && i < (int)desafiado.party.size()) {
          Pokemon& p = desafiado.party[i];
          if (!venceu) { ganharXP(p, XP_POR_VITORIA); tentarEvoluir(p); }
          else ganharXP(p, XP_POR_DERROTA);
        }
      }

      if (venceu) {
        if (desafiante.xp >= desafiado.xp) desafiante.xp += 3;
        else desafiante.xp += 1;
      } else {
        if (desafiado.xp >= desafiante.xp) desafiado.xp += 3;
        else desafiado.xp += 1;
      }

      return venceu;
    }
  }

  // estourou o limite de turnos: termina pelo pokemon ainda consciente
  // (nunca por soma de HP restante; a batalha sempre termina no ponto de
  // inconsciente, hp < MIN_HP_CONSCIENTE)
  bool acabouA = (proximoDisponivel(desafiante, escolhidosA) == -1);
  bool acabouB = (proximoDisponivel(desafiado, escolhidosB) == -1);
  bool venceu = !acabouA && acabouB;
  std::cout << "\n>>> " << (venceu ? desafiante.nome : desafiado.nome)
            << " venceu a batalha!\n";

  for (int i : escolhidosA) {
    if (i >= 0 && i < (int)desafiante.party.size()) {
      Pokemon& p = desafiante.party[i];
      if (venceu) { ganharXP(p, XP_POR_VITORIA); tentarEvoluir(p); }
      else ganharXP(p, XP_POR_DERROTA);
    }
  }
  for (int i : escolhidosB) {
    if (i >= 0 && i < (int)desafiado.party.size()) {
      Pokemon& p = desafiado.party[i];
      if (!venceu) { ganharXP(p, XP_POR_VITORIA); tentarEvoluir(p); }
      else ganharXP(p, XP_POR_DERROTA);
    }
  }

  if (venceu) {
    if (desafiante.xp >= desafiado.xp) desafiante.xp += 3;
    else desafiante.xp += 1;
  } else {
    if (desafiado.xp >= desafiante.xp) desafiado.xp += 3;
    else desafiado.xp += 1;
  }

  return venceu;
}

bool batalharSelvagem(GameState& estado, Treinador& treinador,
                      const std::vector<int>& escolhidos, int idxSelvagem) {
  if (idxSelvagem < 0 || idxSelvagem >= (int)estado.selvagens.size())
    return false;

  Pokemon& sel = estado.selvagens[idxSelvagem];
  std::cout << "\n===== POKEMON SELVAGEM: " << sel.nome << " =====\n";

  // cada batalha custa uma unidade de tempo percorrido
  estado.tempo_decorrido += CUSTO_BATALHA;

  // treinador falso pra usar as mesmas formulas (xp 0)
  Treinador tSel;
  tSel.xp = 0;

  int idxA = proximoDisponivel(treinador, escolhidos);

  bool defendendoJogador = false;
  bool defendendoSelvagem = false;

  // historico das acoes da batalha
  std::vector<std::string> log;

  int limiteTurnos = 100;
  // o selvagem comeca atacando; a batalha termina quando o selvagem e
  // derrubado (hp < MIN_HP_CONSCIENTE) e entra na fase de captura
  while (limiteTurnos-- > 0) {
    if (idxA == -1) idxA = proximoDisponivel(treinador, escolhidos);
    if (idxA == -1) {
      std::cout << "Seus pokemons nao conseguiram vencer o selvagem.\n";
      return false;
    }

    // limpa a tela mas mantem o historico das acoes
    GUI::limparTela();
    int inicio = log.size() > 8 ? (int)log.size() - 8 : 0;
    for (int i = inicio; i < (int)log.size(); i++)
      std::cout << log[i] << "\n";
    GUI::mostrarBatalla(treinador.party[idxA], sel, sel.nome);

    // --- turno do selvagem (ataca primeiro) ---
    if (RNG::chance(0.25)) {
      defendendoSelvagem = true;
      escreverSaida(&log, sel.nome + " se defendeu!");
    } else {
      calcularDano(estado, sel, tSel, treinador.party[idxA], treinador,
                   defendendoJogador, &log);
      defendendoJogador = false;
      if (treinador.party[idxA].hp < MIN_HP_CONSCIENTE) {
        deixarInconsciente(treinador.party[idxA]);
        idxA = proximoDisponivel(treinador, escolhidos);
        if (idxA == -1) {
          std::cout << "Seus pokemons nao conseguiram vencer o selvagem.\n";
          return false;
        }
      }
    }

    // --- turno do jogador ---
    int acao = GUI::escolherAcaoBatalha();

    if (acao == 1) {  // atacar
      calcularDano(estado, treinador.party[idxA], treinador, sel, tSel,
                   defendendoSelvagem, &log);
      defendendoSelvagem = false;
      if (sel.hp < MIN_HP_CONSCIENTE) {
        deixarInconsciente(sel);
        break;  // selvagem derrubado -> fase de captura
      }
    } else if (acao == 2) {  // defender
      defendendoJogador = true;
      escreverSaida(&log, treinador.party[idxA].nome + " se defendeu!");
    } else if (acao == 3) {  // usar item
      usarItemBatalha(treinador);
    } else if (acao == 4) {  // trocar pokemon (entre os escolhidos)
      int novo = trocarPokemonBatalha(treinador, escolhidos, idxA);
      if (novo != idxA) {
        idxA = novo;
        escreverSaida(&log, "Voce mandou " + treinador.party[idxA].nome + "!");
      }
    } else {
      escreverSaida(&log, "Acao invalida.");
    }
  }

  // --- fase de captura: o selvagem esta derrubado/inconsciente ---
  if (sel.hp >= MIN_HP_CONSCIENTE) {
    // nao conseguiu derrubar a tempo: o selvagem foge de vez
    std::cout << "O pokemon selvagem fugiu!\n";
    sel.status = PokemonStatus::No_PMC;
    sel.tempo_recuperacao = RNG::aleatorio(10, 50);
    estado.nos[sel.no_atual].pokemons_selvagens.clear();
    return false;
  }

  // oferece apenas capturar ou deixar ir (a batalha parou)
  int escCaptura = GUI::escolherCaptura();

  // qualquer resposta que nao seja exatamente "1" (capturar) deixa ir:
  // o selvagem foge do mapa, mas volta depois
  if (escCaptura != 1) {
    std::cout << "Voce deixou " << sel.nome << " ir embora...\n";
    sel.status = PokemonStatus::No_PMC;
    sel.tempo_recuperacao = RNG::aleatorio(10, 50);
    estado.nos[sel.no_atual].pokemons_selvagens.clear();
    return false;
  }

  // capturar (escCaptura == 1)
  if (treinador.pokebolas <= 0) {
    std::cout << "Voce nao tem pokebolas para capturar!\n";
    std::cout << "Sem pokebola, o pokemon selvagem fugiu do mapa.\n"
                 "Ele reaparecera por aqui depois.\n";
    sel.status = PokemonStatus::No_PMC;
    sel.tempo_recuperacao = RNG::aleatorio(10, 50);
    estado.nos[sel.no_atual].pokemons_selvagens.clear();
    return false;
  }
  treinador.pokebolas--;

  std::cout << "Voce capturou " << sel.nome << "!\n";
  sel.hp = sel.max_hp;
  sel.status = PokemonStatus::Consciente;
  sel.xp = 0;
  sel.no_atual = treinador.no_atual;
  sel.tempo_recuperacao = 0;

  // entra na party se couber, senao vai pro PC do professor
  if (!StateEngine::equipeCheia(treinador))
    treinador.party.push_back(sel);
  else
    treinador.pc_professor.push_back(sel);

  // remove do mapa: capturado, nao volta ao encontro selvagem
  estado.selvagens[idxSelvagem].status = PokemonStatus::No_PMC;
  estado.nos[sel.no_atual].pokemons_selvagens.clear();

  // +3 xp pro treinador e pros pokemons envolvidos
  treinador.xp += 3;
  for (int i : escolhidos) {
    if (i >= 0 && i < (int)treinador.party.size()) {
      ganharXP(treinador.party[i], 3);
      tentarEvoluir(treinador.party[i]);
    }
  }

  return true;
}

}
