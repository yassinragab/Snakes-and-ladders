# Snakes & Ladders – ADS Project (Graph Representation, BFS Shortest Path, Qt GUI)

## 1. Project Overview

This project implements a complete Snakes & Ladders game in C++ with a graph-based backend and a fully interactive Qt GUI.  
The aim is to demonstrate how graph theory, adjacency lists, and Breadth-First Search (BFS) can be applied to solve real-world problems—in this case, finding the minimum number of dice rolls needed to reach square 100.

The project also focuses on software engineering principles such as modularity, event-driven GUI programming, and clean separation between logic, algorithms, and presentation.

Key capabilities:
- BFS shortest path computation with actual dice roll reconstruction.
- Random valid board generation.
- Custom board editing and validation.
- Real-time animations and multi-player turn handling.
- Player statistics and logging for transparency and testing.

---

## 2. Data Structures & Algorithms

### 2.1 Graph Model (`adj_list`)
The board (1–100) is transformed into a directed unweighted graph:
- Each square is a vertex.
- Edges represent valid dice rolls (1–6).
- Snakes and ladders are encoded through a `board[]` redirection array so edges point to the *resulting* square after sliding/climbing.

Each vertex has:
- Degree ≤ 6 (or fewer near square 100).
- Neighbors stored in:
  - `int** adj` → dynamic adjacency list.
  - `getNeighbors(v)` returns the neighbor array for a square.
  - `getDegree(v)` returns its degree.

This compact adjacency structure ensures **O(1)** access per vertex and **O(V + E)** graph-building time.

### 2.2 BFS Shortest Path (`BFS` + `Queue`)
The goal is to compute:
- Minimum number of dice rolls to reach 100.
- The exact sequence of squares taken.
- The corresponding dice rolls.

BFS is ideal since all edges have equal weight (each dice roll = 1 step).

Algorithm details:
- Arrays `visited[]` and `parent[]` track explored nodes and path reconstruction.
- A custom circular queue supports efficient BFS expansion.
- Once the target is reached, `parent[]` is traced backward to reconstruct the optimal path.

**Time Complexity:**  
`O(V + E)` ≈ `O(100 + ~600)` → effectively constant and extremely fast.

### 2.3 Board Generation and Editing
- **RandomBoard:** Creates valid, non-overlapping snakes and ladders with enforced constraints:
  - Snakes: `head > tail`
  - Ladders: `top > bottom`
- **BoardEditor:** A `QDialog` allowing user input in the form:
  - `S,head,tail`
  - `L,bottom,top`
  - Invalid entries are logged and rejected.

All edited boards trigger a graph rebuild for correctness.

---

## 3. Architecture & Software Design

### 3.1 Core Logic Layer (`BoardEngine`)
`BoardEngine` acts as the “game brain”:
- Stores and manages snakes, ladders, and the graph.
- Rebuilds the graph when the board changes.
- Provides high-level functions:
  - `applySnakesAndLadders()`
  - `shortestPathFrom()`
- Keeps the GUI independent from algorithmic complexity.

### 3.2 GUI Layer (Qt Widgets)

#### `BoardWidget`
Responsible for all board rendering:
- Ladders drawn as double rails + rungs.
- Snakes drawn as curved Bézier paths with stylized heads.
- Players drawn as colored tokens positioned intelligently within cells.
- A star animation appears on square 100 when a player wins.

Geometry functions ensure correct scaling and centering regardless of window size.

#### `GameWindow`
This is the main interactive UI:
- Contains settings panel, dice controls, statistics, and log messages.
- Handles turn order, dice rolling, animations, and win conditions.
- Updates the board at every state change, keeping GUI and logic synchronized.

#### `WelcomePage`
A polished introductory screen that enhances usability and user experience.

---

## 4. Building & Running the Application

### Requirements
- Qt 6 (Qt Widgets)
- C++17-compatible compiler
- Qt Creator or compatible build system

### Running
1. Open project in Qt Creator.
2. Build using a Desktop Qt 6 kit.
3. Run → Welcome Page → Start Game.

Note: CSV paths in the code may need adjusting based on your machine’s directory structure.

---

## 5. How to Use the Application

1. Start Game from the welcome screen.  
2. Configure players, snakes, ladders → *Create Game*.  
3. Use Roll Dice to play. Snakes/ladders are applied automatically.  
4. Shortest Path button shows BFS-based optimal moves from the current player.  
5. Edit Board opens manual configuration dialog for custom boards.  
6. Reset Game restores default configuration.  
7. Exit Game closes the application with confirmation.

Statistics automatically track:
- Turns taken
- Snakes hit
- Ladders climbed
- Player finishing order

---

## 6. Limitations and Future Enhancements

### Current Limitations
- Board fixed at 100 squares and 10×10 layout.
- CSV file path currently hard-coded.
- No game-state persistence between sessions.

### Potential Extensions
- Dynamic board sizes (e.g., 12×12, 15×15).
- AI opponent powered by BFS for optimal play.
- Online multiplayer using Qt networking.
- Theming system for skins (classic, futuristic, minimal, etc.).
- Exporting gameplay statistics to CSV or JSON for analysis.

---

## 7. Academic Objectives Achieved
- Application of graph theory to a real interactive system.  
- Implementation of BFS using a custom queue.  
- Use of dynamic memory, pointers, and object-oriented design.  
- Integration of algorithmic logic** with GUI programming.  
- Understanding of event-driven systems and state management.  

This project combines theoretical ADS concepts with practical software engineering and GUI development.

