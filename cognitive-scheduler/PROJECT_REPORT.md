# Cognitive Load-Aware Task Scheduling System Using Algorithmic Optimization

## 1. Description of the Project

Traditional task schedulers usually assume that if time is available, productive work can be completed. In real life, productivity also depends on mental energy, fatigue, task difficulty, and the cost of switching between different kinds of work.

This project implements a cognitive load-aware scheduler that treats mental capacity as a limited resource. Each task has a priority, duration, difficulty, cognitive load, category, and dependency list. The scheduler creates an explainable task order while respecting dependencies and avoiding overload.

The system uses classical Data Structures and Algorithms only. It does not use Artificial Intelligence or Machine Learning.

## 2. Data Structures Used

| Data Structure | Explanation |
| --- | --- |
| Graph (DAG) | Represents task dependencies. An edge means one task must be completed before another. |
| Queue | Used during topological validation of dependency order. |
| Priority Queue Concept | The greedy algorithm ranks feasible tasks by a score and selects the best available task. |
| Dynamic Programming Table | Stores the best achievable value for task prefixes under different cognitive capacities. |
| Arrays and Lists | Store task records, completed tasks, skipped tasks, and session assignments. |
| Sets and Maps | Provide fast lookup for completed tasks, selected tasks, and task metadata. |

## 3. Algorithms Implemented

### Graph Validation and Topological Feasibility

The project first checks that every dependency exists and that the dependency graph has no cycle. A topological ordering is produced using indegree counts. If all tasks cannot be processed, the graph contains a cycle and scheduling is invalid.

### Greedy Priority Scheduling

The greedy scheduler repeatedly chooses the most feasible available task. A task is available only when all dependencies are completed. The score considers priority, difficulty, cognitive load, remaining capacity, and context-switching penalty.

### Dynamic Programming Optimization

The DP scheduler treats cognitive capacity as a constraint. It evaluates task prefixes and capacity states to choose a high-value subset of tasks. Dependencies are checked before a task is included, so the final output remains valid.

### Session Packing

After a schedule is generated, tasks are packed into work sessions such as morning, afternoon, and evening. Each session has a cognitive capacity and time limit. This models mental energy distribution across the day.

### Context-Switching Penalty

Switching between different task categories adds a penalty. Tasks from the same category have lower switching cost, while category changes and difficulty jumps increase the penalty.

## 4. Mapping of Concepts

| DAA Concept | Real-Life Analogy |
| --- | --- |
| Graph | Project workflow with prerequisites. |
| Priority Queue | Choosing the most mentally feasible task at a given moment. |
| Dynamic Programming | Planning work to maximize output under limited energy. |
| Bin Packing | Distributing mental effort across daily work sessions. |
| Arrays / Lists | Task lists and daily planners. |

## 5. Key Features

- Cognitive-aware task prioritization instead of only time-based ordering.
- Dependency handling through DAG validation.
- Context-switching penalty calculation.
- Session-wise grouping based on cognitive capacity.
- Greedy and optimized strategies for comparison.
- Deterministic and explainable scheduling output.
- Browser-based interactive demonstration.

## 6. Conclusion

The Cognitive Load-Aware Task Scheduling System demonstrates how classical DAA concepts can solve a human-centered productivity problem. By modeling mental energy as a constrained resource and incorporating dependencies and switching costs, the system creates realistic and explainable schedules without relying on machine learning.
