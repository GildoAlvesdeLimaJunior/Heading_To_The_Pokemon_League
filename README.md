# 🎮 Road to the Pokémon League: Graph-Based Text RPG Engine

A graph-based text RPG engine developed as a project for the **Graph Algorithms** course at **Universidade Federal do Cariri (UFCA)**. The engine simulates a Pokémon journey across a weighted graph world, handling pathfinding, discrete event simulation, turn-based combat, entity progression, and dynamic world updates — without relying on external graph libraries.

## Features

| Feature | Description |
|---|---|
| Graph-Based World | World modeled as a weighted graph $G = (V, E, w)$: vertices are locations (Cities, Gyms, Pokémon Medical Centers, Laboratory, Stadium), edges are paths with weight = travel time |
| Global Deadline $T$ | To qualify, register at the Stadium within a strict time limit bounded by $10 \cdot \sum w(e) \le T \le 15 \cdot \sum w(e)$ |
| One-Step Kinematics | Entities move one vertex per action; each edge traversal advances global time by $w(e)$ units |
| Starter Selection | At Professor Oak's Laboratory, choose 1 starter (Water / Fire / Grass) *or* 1 random Pokémon from the lab pool |
| Initial Inventory | Starts with 1 Egg Incubator and 7 Pokéballs (6 for the active party + 1 for wild captures) |
| Party Management | Max **6 active Pokémon**; extras are automatically offloaded to the Laboratory for research |
| Egg Incubation | Wild eggs hatch after **100 distance units** in the incubator; active Pokémon + eggs combined cannot exceed 7 units; eggs cannot be abandoned |
| Health Dynamics | Three states — **Conscious** ($HP \ge 20$, battle-eligible, $+1\,HP$/10 units), **Unconscious** ($5 \le HP < 20$, disabled for $t \in [10,50]$ units), **Critically Hurt** ($HP < 5$, mandatory PCM admission restoring HP to 100) |
| Herbal Medicine | Foraged herbs brew remedies restoring $+10\,HP$ to all *conscious* party members |
| XP & Attribute Scaling | $+10$ XP per victory, $+3$ XP per defeat, $+1$ XP per 100 units traveled; AP and DP scale with base + bonus + $10\%$ of XP |
| Evolutionary Metamorphosis | Reaching **1,000 XP** triggers evolution; each phase raises base AP/DP by **+30%**; species support up to 3 phases |
| Turn-Based Battle Engine | Challenging requires ≥ 3 conscious Pokémon; each battle costs 1 time unit; damage $= \max(0, AP_{att} - DP_{def})$; evasion and critical hits proportional to XP difference (crits deal $2\times$) |
| Faint Substitution | Unconscious Pokémon are immediately replaced by a conscious ally during battle |
| Wild Capture | Capture wild Pokémon by defeating them to an unconscious state; awards $+3$ XP to trainer and Pokémon |
| Type Effectiveness | Damage scaled by elemental type multiplier matrix (Water, Fire, Grass, Electric, Ice, Fighting, Psychic, Ghost) |
| Gym Badges & League Qualification | Defeat 8 Gym Leaders for unique permanent badges; qualify by reaching the Stadium with all badges before deadline $T$ |
| Team Rocket Roaming | A roaming unit challenges trainers to steal badges/Pokémon; defeat teleports it away, victory grants stolen assets and makes it invisible for a set interval |
| Data-Driven Input | World topology, species, trainers, items, and multipliers loaded from a single ASCII config file (`data/graph.txt`) |

## Algorithmic Core

| Algorithm | Purpose |
|---|---|
| **Dijkstra (manual Min-Heap)** | Optimal paths, deadline feasibility, NPC navigation |
| **BFS / DFS** | Connectivity validation at startup; reachable subgraphs for roaming entities |
| **MST — Prim / Kruskal** | Edge-density analysis to validate the global time limit $T$ |

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

- `Types.hpp` — core domain types: `No`, `Pokemon`, `Treinador`, `GameState`, and enums (`TipoNo`, `PokemonStatus`)
- `MapParser.hpp` — parses `data/graph.txt` into a `GameState`
- `GraphEngine.hpp` — Dijkstra, BFS/DFS, MST (Prim/Kruskal)
- `BattleEngine.hpp` — turn-based combat, captures, type multipliers
- `StateEngine.hpp` — discrete event simulation, NPC/Team Rocket movement
- `RNG.hpp` — randomization abstraction for stochastic mechanics
- `GUI.hpp` — terminal interface, menus, badge display

## Technologies

- **C++17** (`g++` / `clang++`)
- **Paradigm:** Object-Oriented & Modular Design
- **Dependencies:** none (graph algorithms implemented manually)
- **Build:** Makefile (to be provided for compiling and running the app and tests)

## Project Structure

```
Heading_To_The_Pokemon_League/
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
├── docs/
│   └── Requests.pdf
└── README.md
```

## How to Run

A **Makefile** will be created to compile the tests and run the app. The expected usage is:

```bash
make        # build
make run    # run the game
```

The game loads its world from the configuration file:

```bash
./pokemon_rpg data/graph.txt
```

> **Note:** Build tooling is not yet committed — command above reflects the planned setup. Full requirements in [`docs/Requests.pdf`](docs/Requests.pdf).

## Authors

- **Gildo Alves de Lima Junior**
- **João Landin da Cruz Neto**
- **Francisco Almir Bezerra Leite**

## Course

- **Course:** Graph Algorithms (Algoritmos em Grafos — 2026.1)
- **University:** Universidade Federal do Cariri (UFCA)
- **Professor:** Carlos Vinicius G. C. Lima
- **Location:** Juazeiro do Norte – CE, 2026