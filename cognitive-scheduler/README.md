# Cognitive Load Aware Task Scheduler

DAA lab project for **Cognitive Load-Aware Task Scheduling System Using Algorithmic Optimization**.

The app demonstrates how classical data structures and algorithms can schedule tasks by mental energy instead of time alone. It uses dependency graphs, greedy priority selection, dynamic programming, and session-wise packing to create an explainable schedule.

## How to Run

```bash
npm install
npm run dev
```

Open the local Vite URL shown in the terminal.

To create a production build:

```bash
npm run build
```

## C++ Implementation

This project also includes a standalone C++ version of the scheduling logic:

```bash
cd cpp
g++ scheduler.cpp -o scheduler
./scheduler
```

On Windows PowerShell, run:

```powershell
cd cpp
g++ scheduler.cpp -o scheduler.exe
.\scheduler.exe
```

The C++ program demonstrates graph validation, topological ordering, greedy scheduling, dynamic programming optimization, metrics calculation, and session packing.

## Features

- Shows sample project tasks with cognitive load, priority, duration, category, and dependencies.
- Validates the task dependency graph as a DAG.
- Generates a greedy schedule using priority-queue style scoring.
- Generates an optimized schedule using dynamic programming under a cognitive budget.
- Packs selected tasks into morning, afternoon, and evening focus sessions.
- Compares scheduled tasks, cognitive load, switching penalty, and efficiency score.

## Algorithms Used

| Concept | Use in Project |
| --- | --- |
| Graph / DAG | Represents task prerequisites and prevents invalid execution order. |
| Topological Sorting | Checks whether tasks can be scheduled without dependency cycles. |
| Priority Queue / Greedy | Selects the best currently feasible task using priority, load, and switching penalty. |
| Dynamic Programming | Selects a high-value dependency-safe subset under a limited cognitive budget. |
| Bin Packing | Groups selected tasks into focus sessions with capacity and time limits. |
| Arrays / Lists | Store active, completed, pending, and session task groups. |

## Demo Flow

1. Review the input task table.
2. Select **Compare both**, **Greedy priority queue**, or **Dynamic programming**.
3. Adjust the cognitive budget slider.
4. Read the generated task order and session allocation.
5. Compare metrics to explain why one strategy performs better for a given budget.

## Project Structure

```text
src/
  algorithms/
    dpScheduler.js
    graph.js
    greedyScheduler.js
    metrics.js
    sessionPacking.js
  data/
    tasks.js
  main.js
  style.css
```

## Notes

This project intentionally does not use AI or machine learning. All scheduling decisions are deterministic and explainable through DAA concepts.
