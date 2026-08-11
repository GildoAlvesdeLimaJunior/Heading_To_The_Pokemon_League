# 🎮 Road to the Pokémon League: Graph-Based Text RPG Engine

## 1. Abstract

This repository contains the official C++ implementation of **Road to the Pokémon League**, a text-based RPG simulation engine developed for the *Graph Algorithms* course at the Federal University of Cariri (UFCA). The engine models a dynamic Pokémon journey across a weighted graph environment. It handles pathfinding, discrete event simulation, finite-state machines, turn-based combat, entity progression, and dynamic world state updates without relying on external graph libraries.

---

## 2. Domain Model & System Specifications

### 2.1 World Topology & Temporal Constraints
* **Graph Topology:** The region is modeled as a weighted graph $G = (V, E, w)$, where vertices $V$ represent locations (Cities, Gyms, Pokémon Medical Centers, Professor Oak's Laboratory, and the League Stadium), and edges $E$ represent paths with weights $w(e)$ corresponding to travel distance/time.
* **Entity Kinematics:** All entities navigate the graph strictly one vertex at a time. Traversing an edge advances the global simulation time by $w(e)$ units.
* **Global Deadline Constraint:** To qualify for the League, the player must register at the League Stadium within a strict time limit $T$, bounded by the total sum of edge weights:
  $$10 \cdot \sum_{e \in E} w(e) \le T \le 15 \cdot \sum_{e \in E} w(e)$$

### 2.2 Inventory & Starter Allocation
* **Initial Inventory:** The trainer receives 1 Egg Incubator and 7 Pokéballs (6 reserved for active party members and 1 designated for capturing wild specimens).
* **Starter Allocation:** Upon initialization at Professor Oak's Laboratory, the trainer may choose **1 starter** among three distinct elemental types (Water, Fire, Grass) or opt for **1 randomly selected Pokémon** from the laboratory pool.

### 2.3 Party Management & Egg Incubation
* **Party Capacity:** The trainer can carry a maximum of **6 active Pokémon** simultaneously.
* **Automated Offloading:** Any additional Pokémon acquired beyond the 6-member limit is automatically offloaded to Professor Oak's Laboratory for research.
* **Incubation Mechanics:**
  * Wild eggs require a cumulative travel distance of **100 distance units** in an incubator to hatch.
  * Newly hatched Pokémon initialize with **0 XP** and base Phase 1 attributes.
  * A trainer may carry multiple eggs concurrently, provided the total count of active Pokémon and unhatched eggs does **not exceed 7 units**. Eggs cannot be abandoned once acquired.

### 2.4 Health Dynamics, States, and Recovery Protocols
A Pokémon's state is defined by its Health Points ($HP \in [1, 100]$). State transitions are governed by the following rules:

| State | $HP$ Range | Operational Rules & Recovery Mechanics |
| :--- | :--- | :--- |
| **Conscious** | $HP \ge 20$ | Eligible for battle. Passive regeneration of $+1\,HP$ per 10 distance units traveled. |
| **Unconscious** | $5 \le HP < 20$ | Ineligible for battle. Remains indisposed for a randomized interval $t \in [10, 50]$ distance units. |
| **Critically Hurt** | $HP < 5$ | Requires immediate admission to a **PMC (Pokémon Medical Center)**. Entails a treatment stay of $t \in [10, 50]$ distance units. $HP$ is restored to $100$ upon discharge. |

* **Herbal Medicine:** Foraged wild herbs permit brewing natural remedies, restoring **$+10\,HP$** to all currently *conscious* party members (unconscious Pokémon cannot consume medicine).

### 2.5 Experience, Attribute Scaling, and Metamorphosis
* **XP Accumulation:**
  * **Combat Victory:** $+10\text{ XP}$.
  * **Combat Defeat:** $+3\text{ XP}$.
  * **Distance Traveled:** $+1\text{ XP}$ per 100 units moved.
* **Attribute Function:** Attack Points ($AP$) and Defense Points ($DP$) scale dynamically according to randomized base parameters, accumulated combat bonuses, and a $10\%$ linear scaling factor relative to total $XP$.
* **Evolutionary Metamorphosis:**
  * Reaching **$1,000\text{ XP}$** triggers evolutionary advancement.
  * Evolutionary progression increases base $AP$ and $DP$ by **$+30\%$** per phase.
  * Species support up to **3 distinct evolutionary phases**.

### 2.6 Deterministic/Stochastic Turn-Based Battle Engine
Challenging another trainer requires **at least 3 conscious Pokémon** in the active party. Each battle consumes $1\text{ time unit}$.

* **Combat Execution:** Turn-based protocol where the challenged party attacks first.
* **Base Damage Function:**
  $$\text{Damage} = \max\left(0, AP_{\text{attacker}} - DP_{\text{defender}}\right)$$
* **Evasion Probability:** Evasion chance is proportional to $|\text{XP}_{\text{defender}} - \text{XP}_{\text{attacker}}|$. Successful evasion completely nullifies incoming damage.
* **Critical Hit Probability:** Critical strike chance is proportional to $|\text{XP}_{\text{attacker}} - \text{XP}_{\text{defender}}|$, inflicting **$2\times$ damage** upon activation.
* **Faint Substitution:** Unconscious active Pokémon must be immediately substituted by a conscious party member.
* **Wild Captures:** Wild Pokémon are captured by defeating and reducing them to an unconscious state. Successful capture awards $+3\text{ XP}$ to both trainer and participating Pokémon.

### 2.7 Type Effectiveness Matrix (Extension)
Calculated damage incorporates elemental type multipliers (e.g., Water, Fire, Grass, Poison, Ghost) acting as scalar modifiers directly on computed attack damage.

### 2.8 League Qualification & Gym Dynamics
* **Gym Leaders:** Located at specified Gym vertices. Leaders may remain stationary or dynamically roam between vertices before returning periodically.
* **Badges:** Defeating a Gym Leader grants a unique, permanent Gym Badge.
* **Qualification Requirement:** Acquire **8 unique badges** and reach the League Stadium prior to time limit $T$.

### 2.9 Roaming Threat: Team Rocket (Extension)
* **Behavioral Protocol:** A roaming Team Rocket unit traverses the graph to challenge trainers, seeking to steal badges or Pokémon.
* **State Shifts:** Defeat teleports Team Rocket to a distant random vertex. Victory grants stolen assets and turns the unit invisible for a set interval before reappearing elsewhere.

---

## 3. Software Architecture & Algorithmic Design

The software adheres to modern C++ standards, emphasizing object-oriented modularization and computational efficiency.

```text
├── include/
│   ├── Graph.hpp
│   ├── Algorithms.hpp 
│   ├── Entities.hpp   
│   ├── BattleEngine.hpp 
│   └── Parser.hpp
└── src/
    └── main.cpp
```

### Algorithmic Implementation Specifications
1. **Dijkstra's Shortest Path Algorithm:**
   * Computes optimal paths, evaluates total distance bounds, and guides NPC navigation.
   * Implemented manually utilizing a custom Min-Heap data structure.
2. **Breadth-First Search (BFS) / Depth-First Search (DFS):**
   * Validates graph connectivity upon system startup and calculates reachable subgraphs for roaming entities.
3. **Minimum Spanning Tree (Prim's / Kruskal's MST):**
   * Analyzes map edge density to validate the global time limit $T$.

---

## 4. Data Specification & I/O Protocol

The application initializes its state space via an ASCII configuration file defining graph topology and initial entity distributions:

```text
# Graph Definition (Vertices and Weighted Edges)
VERTICES 15
EDGES 22
0 1 12
1 2 8

# Special Facilities
LAB 0
STADIUM 14
PMC 3 8 12
GYM 2 Fire_Badge
GYM 7 Water_Badge

# Initial Population
POKEMON_COUNT 25
TRAINER_COUNT 10
ITEMS_COUNT 15

# Time Limit Factor
TIME_LIMIT_FACTOR 12.5
```

---

## 5. Build & Execution Protocol

### Prerequisites
* C++17 compliant compiler (`g++` or `clang++`)
* `cmake` (version 3.14 or higher)

### Compilation
```bash
# Clone repository
git clone [https://github.com/your-repo/rumo-a-liga-pokemon.git](https://github.com/your-repo/rumo-a-liga-pokemon.git)
cd rumo-a-liga-pokemon

# Create build directory
mkdir build && cd build

# Configure and compile
cmake ..
make -j4
```

### Execution
```bash
./pokemon_rpg ../data/map_config.txt
```

---

## 6. Institutional Credits & Project Metadata

* **Institution:** Federal University of Cariri (UFCA) — CCT
* **Course:** Graph Algorithms (Algoritmos em Grafos — 2026.1)
* **Instructor:** Prof. Carlos Vinicius G. C. Lima
* **Project Title:** *Road to the Pokémon League*
* **Authors:**
  * Gildo Alves de Lima Junior
  * João Landin da Cruz Neto
  * Francisco Almir Bezerra Leite
