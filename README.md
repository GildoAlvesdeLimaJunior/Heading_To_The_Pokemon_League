# 🎮 Road to the Pokémon League: Graph-Based Text RPG Engine

A graph-based text RPG engine developed as a project for the **Graph Algorithms** course at **Universidade Federal do Cariri (UFCA)**. The engine simulates a Pokémon journey across a weighted graph world, handling pathfinding, discrete event simulation, turn-based combat, entity progression, dynamic world updates, and roaming NPCs — without relying on external graph libraries.

## Features

| Feature | Description |
|---|---|
| Graph-Based World | World modeled as a weighted graph $G = (V, E, w)$: vertices are locations (Cities, Gyms, Pokémon Medical Centers, Laboratory, Stadium), edges are paths with weight = travel time |
| Global Deadline $T$ | To qualify, register at the Stadium within a strict time limit bounded by $10 \cdot \sum w(e) \le T \le 15 \cdot \sum w(e)$; the factor $K$ is validated at load time (error if outside $[10,15]$) |
| One-Step Kinematics | Entities move one vertex per action; each edge traversal advances global time by $w(e)$ units |
| Starter Selection | At Professor Oak's Laboratory, choose 1 starter (Water / Fire / Grass) *or* 1 random Pokémon from the lab pool (excluding the fixed starters); invalid options are re-prompted |
| Initial Inventory | Starts with 1 Egg Incubator and 7 Pokéballs (6 for the active party + 1 for wild captures) |
| Party Management | Max **6 active Pokémon**; extras are automatically offloaded to the Laboratory for research (`enviarParaCarvalho`); full management of Professor Oak's PC (store/retrieve Pokémon and eggs) available anywhere |
| Egg Incubation | Wild eggs hatch after **100 distance units** in the incubator; active Pokémon + eggs combined cannot exceed 7 units; eggs cannot be abandoned |
| Health Dynamics | Three states — **Conscious** ($HP \ge 20$, battle-eligible), **Unconscious** ($5 \le HP < 20$, disabled for $t \in [10,50]$ units), **Critically Hurt** ($HP < 5$, mandatory PCM admission restoring HP to 100). Conscious Pokémon do **not** recover HP passively while walking — recovery happens only at the PCM, via items, or herbs. Critically hurt (`No_PMC`) Pokémon do **not** recover by walking either; they require PCM treatment |
| Wild Respawn | Wild Pokémon that fled uncaptured come back after their recovery timer expires — they return to **Conscious** with full HP on the same node and can be encountered again (`andandoReporSelvagens`) |
| Herbal Medicine | Foraged herbs brew remedies restoring $+10\,HP$ to all *conscious* party members; the herb is only consumed if at least one conscious Pokémon can receive it |
| XP & Attribute Scaling | $+10$ XP per victory, $+3$ XP per defeat, $+1$ XP per 100 units traveled; AP and DP scale with base + bonus + $10\%$ of XP; initial AP/DP are randomized at character creation |
| Evolutionary Metamorphosis | Reaching **1,000 XP** triggers evolution; each phase raises base AP/DP by **+30%**; species support up to 3 phases; consuming 1000 XP keeps the remainder |
| Turn-Based Battle Engine | Challenging requires ≥ 3 conscious Pokémon (wild battles need ≥ 1); each battle costs 1 time unit; damage $= \max(0, AP_{att} - DP_{def})$ with a **minimum damage of 1** and type multiplier; evasion and critical hits proportional to XP difference (crits deal $2\times$); defending halves incoming damage |
| Faint Substitution | Unconscious Pokémon are immediately replaced by a conscious ally during battle |
| Wild Capture | Capture wild Pokémon only after defeating them to an unconscious state; you then choose **Capture** or **Let go** (anything other than Capture lets it flee and respawn later); awards $+3$ XP to trainer and Pokémon; requires Pokéballs |
| Items & Revive | Cure-only items do **not** revive; revive items force $HP \ge MIN\_HP\_CONSCIENTE$ (battle-eligible); invalid targets (eggs, full HP, unconscious for cure items) are rejected |
| Type Effectiveness | Damage scaled by elemental type multiplier matrix (Water, Fire, Grass, Electric, Ice, Fighting, Psychic, Ghost) |
| Gym Leaders Roaming | Gym Leaders roam the region and periodically return to their home Gym; defeating a leader grants a unique permanent badge (won badges are tracked, and a badge stolen by Team Rocket can be re-conquered) |
| Gym Badges & League Qualification | Defeat 8 Gym Leaders for unique permanent badges; qualify by reaching the Stadium with all badges before deadline $T$ (`podeInscrever` checks the deadline and badge count) |
| Team Rocket Roaming | A roaming unit challenges trainers to steal badges/Pokémon; defeat teleports it away, victory grants stolen assets and makes it invisible for a set interval; losing to Rocket steals a badge (decrements `insignias` and removes it from `insignias_ganhas`) or a Pokémon |
| PCM Healing | Pokémon Medical Centers heal **all** Pokémon (party and Professor's PC); **eggs are never healed** (healing would prematurely turn them into battle-ready Pokémon) |
| Data-Driven Input | World topology, species, trainers, items, and multipliers loaded from a single ASCII config file (`data/graph.txt`) |
| Connectivity Validation | At boot the map is validated with **DFS** (every node reachable from the Laboratory) and **BFS** (strongly connected); a disconnected graph aborts loading with an error |

## Algorithmic Core

| Algorithm | Purpose |
|---|---|
| **Dijkstra (manual Min-Heap)** | Optimal paths (own responsibility: Gildo), deadline feasibility, NPC navigation |
| **BFS / DFS** | Connectivity validation at startup; reachable subgraphs for roaming entities (own responsibility: João) |
| **MST — Prim / Kruskal** | Edge-density analysis to validate the global time limit $T$ (own responsibility: Almir) |

> Each team member owns **one graph operation**, as required by the assignment (item ix): **Gildo → Dijkstra**, **João → BFS/DFS**, **Almir → MST (Prim/Kruskal)**.

## Module Overview

```
Types.hpp ──────────► MapParser.hpp
                           │
                           ▼
                      GraphEngine.hpp
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
        StateEngine.hpp  RNG.hpp   BattleEngine.hpp
              └────────────┴────────────┘
                           │
                           ▼
                       GUI.hpp
```

- `Types.hpp` — core domain types: `No`, `Pokemon`, `Treinador`, `GameState`, and enums (`TipoNo`, `PokemonStatus`); Gym-leader roaming fields (`eh_lider`, `no_base`, timers), won badges (`insignias_ganhas`), and player inventory
- `MapParser.hpp` — parses `data/graph.txt` into a `GameState`, randomizes stats, seeds Gym leaders, validates $K \in [10,15]$ and graph connectivity (`carregarMapa`)
- `GraphEngine.hpp` — Dijkstra, BFS/DFS, MST (Prim/Kruskal), all implemented manually without external libraries
- `BattleEngine.hpp` — turn-based combat, damage/type multipliers, evasion & criticals, XP & evolution, faint substitution, trainer battles, wild capture (`batalharTreinador`, `batalharSelvagem`, `aplicarDano`, `deixarInconsciente`, `tentarEvoluir`, `multiplicadorTipo`)
- `StateEngine.hpp` — discrete event simulation, movement and time passage, egg hatching, recovery timers, wild respawn, safe zones, NPC/Team Rocket movement (`moverTreinador`, `AvancarTempo`, `andandoChocarOvos`, `andandoReporSelvagens`, `verificar_zonasegura`, `podeBatalhar`, `podeInscrever`, `prazoExpirado`, `podePegarOvo`, `usarErva`, `enviarParaCarvalho`, `criarJogador`)
- `RNG.hpp` — randomization abstraction for stochastic mechanics (`aleatorio`, `chance`, `chance_float`)
- `GUI.hpp` — terminal interface, menus, badge display, HP bars, battle/capture menus (`menuPrincipal`, `escolherAcaoBatalha`, `escolherCaptura`, `barraHP`, `mostrarStatus`)

## Main Game Loop (main.cpp)

On startup the game prints a **graph analysis report** (Dijkstra distances from the Laboratory, MST weight via Prim and Kruskal, DFS connectivity check). Then the player picks a starter and enters the main loop with these options:

| # | Action |
|---|---|
| 1 | Move to an adjacent node (consumes $w(e)$ time, triggers node events) |
| 2 | Use a herb (+10 HP to all conscious party members) |
| 3 | Search for a wild Pokémon (blocked in safe zones) |
| 4 | Challenge a trainer / Gym Leader in the node |
| 5 | Heal at a Pokémon Medical Center (heals all, never eggs) |
| 6 | Register at the Stadium (win the game with 8 badges before the deadline) |
| 7 | Use an item from the inventory |
| 8 | Manage Professor Oak's PC (store/retrieve Pokémon and eggs) |
| 0 | Quit |

## Technologies

- **C++17** (`g++` / `clang++`)
- **Paradigm:** Data-Oriented Design — plain structs + free functions in namespaces
- **Dependencies:** none (graph algorithms implemented manually)
- **Build:** Makefile (compiles and runs the app)
- **Testing:** dedicated regression tests (`test_wild`, `test_evolve`, `test_integration`) compiled against the current object files to verify wild capture, evolution, and end-to-end battles

## Project Structure

```
Heading_To_The_Pokemon_League/
├── Makefile
├── include/
│   ├── BattleEngine.hpp
│   ├── GraphEngine.hpp
│   ├── GUI.hpp
│   ├── MapParser.hpp
│   ├── RNG.hpp
│   ├── StateEngine.hpp
│   └── Types.hpp
├── src/
│   ├── BattleEngine.cpp
│   ├── GraphEngine.cpp
│   ├── GUI.cpp
│   ├── main.cpp
│   ├── MapParser.cpp
│   ├── RNG.cpp
│   └── StateEngine.cpp
├── data/
│   └── graph.txt
└── README.md
```

## How to Run

```bash
make        # compile
make run    # run the game (data/graph.txt)
```

Or, directly:

```bash
./pokemon_rpg data/graph.txt
```

To clean the build artifacts:

```bash
make clean
```

### `data/graph.txt` Structure

The whole world is described by a single ASCII config file. Lines starting with `#` are comments. The file is read in nine sections:

| # | Section | Line format |
|---|---------|-------------|
| 1 | **Graph header** | `[nodes] [edges] [factor_K]` — total nodes, undirected edges and the deadline factor $K \in [10,15]$ |
| 2 | **Edges** | `[from] [to] [weight]` — $N$ undirected edges; `weight` is the travel time in time units |
| 3 | **Nodes** | `[id] [name] [type]` — `type` ∈ `REGULAR`, `LABORATORIO`, `PMC`, `GINASIO`, `ESTADIO` |
| 4 | **Elemental types** | `[qty_types]` followed by the type names (one line) and the `[attacker][defender]` multiplier matrix |
| 5 | **Species catalog** | `[qty_species]` then `[id] [num_phases] [phase1] [phase2] ... [type_id]` per species |
| 6 | **Lab starters** | `[species1] [species2] [species3]` — the three offered starters (Water / Fire / Grass) |
| 7 | **Wild Pokémon** | `[qty_wild]` then `[instance_id] [species] [node] [ap_base] [dp_base]` per wild Pokémon |
| 8 | **Items** | `[qty_items]` then `[item_id] [node] [name] [effect] [value]` — `effect`: `0` = heal HP, `1` = revive |
| 9 | **Trainers & Gym Leaders** | `[qty_trainers]` then `[id] [name] [node] [is_leader] [qty_pokemon] [species] [level]...` per trainer |

The state of each section is inferred in order, so keep the sections in the sequence above. Gym Leaders are marked with `is_leader = 1`; the Team Rocket roaming unit is identified by its name (e.g. `Rocket_Grunt`).

## Authors

- **Gildo Alves de Lima Junior**
- Video : https://youtu.be/LDUr2t2w12E
- **João Landin da Cruz Neto**
- Video : https://youtu.be/cEEJ3ymrExU
- **Francisco Almir Bezerra Leite**
- Video : https://www.youtube.com/watch?v=cyNn_Zl_Jjw

## Course

- **Course:** Graph Algorithms (Algoritmos em Grafos — 2026.1)
- **University:** Universidade Federal do Cariri (UFCA)
- **Professor:** Carlos Vinicius G. C. Lima
- **Location:** Juazeiro do Norte – CE, 2026
