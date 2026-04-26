#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Task {
    string id;
    string title;
    string category;
    int difficulty;
    int cognitiveLoad;
    int duration;
    int priority;
    vector<string> dependencies;
};

struct Session {
    string name;
    int capacity;
    int minutes;
};

struct Metrics {
    int cognitiveLoad = 0;
    int duration = 0;
    int priority = 0;
    int switchingPenalty = 0;
    int taskCount = 0;
    int efficiency = 0;
};

struct GraphResult {
    bool isValid = false;
    bool hasCycle = false;
    vector<string> topologicalOrder;
    vector<pair<string, string>> missingDependencies;
};

struct ScheduleResult {
    string strategy;
    vector<Task> schedule;
    vector<Task> skipped;
    Metrics metrics;
    GraphResult graph;
};

vector<Task> getSampleTasks() {
    return {
        {"research", "Collect project requirements", "Analysis", 3, 5, 45, 8, {}},
        {"dag", "Build dependency graph", "Design", 4, 6, 55, 9, {"research"}},
        {"dataset", "Prepare task data set", "Implementation", 3, 4, 40, 7, {"research"}},
        {"greedy", "Implement greedy scheduler", "Algorithm", 4, 7, 65, 9, {"dag", "dataset"}},
        {"dp", "Implement DP optimizer", "Algorithm", 5, 9, 85, 10, {"dag", "dataset"}},
        {"sessions", "Pack tasks into focus sessions", "Planning", 4, 6, 50, 8, {"greedy", "dp"}},
        {"ui", "Create scheduler interface", "Implementation", 3, 5, 60, 7, {"greedy"}},
        {"metrics", "Calculate comparison metrics", "Evaluation", 4, 6, 45, 8, {"greedy", "dp"}},
        {"report", "Write DAA project report", "Documentation", 2, 3, 50, 6, {"metrics", "sessions"}},
        {"viva", "Prepare viva explanation", "Documentation", 2, 2, 30, 5, {"report"}},
    };
}

vector<Session> getSessions() {
    return {
        {"Morning Deep Work", 18, 150},
        {"Afternoon Build Slot", 14, 120},
        {"Evening Light Review", 10, 90},
    };
}

unordered_map<string, Task> createTaskMap(const vector<Task>& tasks) {
    unordered_map<string, Task> taskMap;
    for (const Task& task : tasks) {
        taskMap[task.id] = task;
    }
    return taskMap;
}

GraphResult validateDependencyGraph(const vector<Task>& tasks) {
    GraphResult result;
    unordered_map<string, Task> taskMap = createTaskMap(tasks);
    unordered_map<string, int> indegree;
    unordered_map<string, vector<string>> adjacency;

    for (const Task& task : tasks) {
        indegree[task.id] = 0;
        adjacency[task.id] = {};
    }

    for (const Task& task : tasks) {
        for (const string& dependencyId : task.dependencies) {
            if (!taskMap.count(dependencyId)) {
                result.missingDependencies.push_back({task.id, dependencyId});
                continue;
            }

            adjacency[dependencyId].push_back(task.id);
            indegree[task.id]++;
        }
    }

    queue<string> ready;
    for (const Task& task : tasks) {
        if (indegree[task.id] == 0) {
            ready.push(task.id);
        }
    }

    while (!ready.empty()) {
        string current = ready.front();
        ready.pop();
        result.topologicalOrder.push_back(current);

        for (const string& nextTaskId : adjacency[current]) {
            indegree[nextTaskId]--;
            if (indegree[nextTaskId] == 0) {
                ready.push(nextTaskId);
            }
        }
    }

    result.hasCycle = result.topologicalOrder.size() != tasks.size();
    result.isValid = result.missingDependencies.empty() && !result.hasCycle;
    return result;
}

int getSwitchingCost(const Task* previousTask, const Task& nextTask) {
    if (previousTask == nullptr) {
        return 0;
    }
    if (previousTask->category == nextTask.category) {
        return 1;
    }

    int difficultyJump = abs(previousTask->difficulty - nextTask.difficulty);
    return 2 + difficultyJump;
}

double taskScore(const Task& task, const Task* previousTask, int remainingCapacity) {
    int switchingCost = getSwitchingCost(previousTask, task);
    double capacityFitBonus = max(0, remainingCapacity - task.cognitiveLoad) * 0.12;

    return task.priority * 3.0
        + task.difficulty * 1.2
        - task.cognitiveLoad * 0.9
        - switchingCost * 1.4
        + capacityFitBonus;
}

Metrics calculateScheduleMetrics(const vector<Task>& schedule) {
    Metrics metrics;
    metrics.taskCount = static_cast<int>(schedule.size());

    const Task* previousTask = nullptr;
    for (const Task& task : schedule) {
        metrics.cognitiveLoad += task.cognitiveLoad;
        metrics.duration += task.duration;
        metrics.priority += task.priority;
        metrics.switchingPenalty += getSwitchingCost(previousTask, task);
        previousTask = &task;
    }

    metrics.efficiency = metrics.priority * 10 - metrics.switchingPenalty * 4 - metrics.cognitiveLoad;
    return metrics;
}

bool dependenciesCompleted(const Task& task, const set<string>& completedIds) {
    return all_of(task.dependencies.begin(), task.dependencies.end(), [&](const string& dependencyId) {
        return completedIds.count(dependencyId) > 0;
    });
}

vector<Task> getSkippedTasks(const vector<Task>& tasks, const set<string>& selectedIds) {
    vector<Task> skipped;
    for (const Task& task : tasks) {
        if (!selectedIds.count(task.id)) {
            skipped.push_back(task);
        }
    }
    return skipped;
}

ScheduleResult runGreedyScheduler(const vector<Task>& tasks, int totalCapacity) {
    ScheduleResult result;
    result.strategy = "Greedy Priority Queue";
    result.graph = validateDependencyGraph(tasks);

    if (!result.graph.isValid) {
        result.skipped = tasks;
        result.metrics = calculateScheduleMetrics(result.schedule);
        return result;
    }

    set<string> completedIds;
    int remainingCapacity = totalCapacity;
    const Task* previousTask = nullptr;

    while (completedIds.size() < tasks.size()) {
        vector<pair<double, Task>> feasibleTasks;

        for (const Task& task : tasks) {
            if (completedIds.count(task.id)) {
                continue;
            }
            if (!dependenciesCompleted(task, completedIds)) {
                continue;
            }
            if (task.cognitiveLoad > remainingCapacity) {
                continue;
            }

            feasibleTasks.push_back({taskScore(task, previousTask, remainingCapacity), task});
        }

        if (feasibleTasks.empty()) {
            break;
        }

        sort(feasibleTasks.begin(), feasibleTasks.end(), [](const auto& left, const auto& right) {
            if (fabs(left.first - right.first) > 0.0001) {
                return left.first > right.first;
            }
            return left.second.priority > right.second.priority;
        });

        Task selected = feasibleTasks.front().second;
        result.schedule.push_back(selected);
        completedIds.insert(selected.id);
        remainingCapacity -= selected.cognitiveLoad;
        previousTask = &result.schedule.back();
    }

    result.skipped = getSkippedTasks(tasks, completedIds);
    result.metrics = calculateScheduleMetrics(result.schedule);
    return result;
}

bool respectsDependencies(const set<string>& candidateIds, const Task& task) {
    return all_of(task.dependencies.begin(), task.dependencies.end(), [&](const string& dependencyId) {
        return candidateIds.count(dependencyId) > 0;
    });
}

ScheduleResult runDpScheduler(const vector<Task>& tasks, int totalCapacity) {
    ScheduleResult result;
    result.strategy = "Dynamic Programming Optimizer";
    result.graph = validateDependencyGraph(tasks);

    if (!result.graph.isValid) {
        result.skipped = tasks;
        result.metrics = calculateScheduleMetrics(result.schedule);
        return result;
    }

    unordered_map<string, Task> taskMap = createTaskMap(tasks);
    vector<Task> orderedTasks;
    for (const string& taskId : result.graph.topologicalOrder) {
        orderedTasks.push_back(taskMap[taskId]);
    }

    int n = static_cast<int>(orderedTasks.size());
    vector<vector<int>> value(n + 1, vector<int>(totalCapacity + 1, 0));
    vector<vector<set<string>>> selected(n + 1, vector<set<string>>(totalCapacity + 1));

    for (int i = 1; i <= n; i++) {
        const Task& task = orderedTasks[i - 1];

        for (int capacity = 0; capacity <= totalCapacity; capacity++) {
            value[i][capacity] = value[i - 1][capacity];
            selected[i][capacity] = selected[i - 1][capacity];

            if (task.cognitiveLoad <= capacity) {
                int previousCapacity = capacity - task.cognitiveLoad;
                if (respectsDependencies(selected[i - 1][previousCapacity], task)) {
                    int candidateValue = value[i - 1][previousCapacity]
                        + task.priority * 10
                        + task.difficulty * 2
                        - task.cognitiveLoad;

                    if (candidateValue > value[i][capacity]) {
                        value[i][capacity] = candidateValue;
                        selected[i][capacity] = selected[i - 1][previousCapacity];
                        selected[i][capacity].insert(task.id);
                    }
                }
            }
        }
    }

    set<string> selectedIds = selected[n][totalCapacity];
    const Task* previousTask = nullptr;

    while (result.schedule.size() < selectedIds.size()) {
        vector<pair<double, Task>> candidates;
        set<string> scheduledIds;
        for (const Task& task : result.schedule) {
            scheduledIds.insert(task.id);
        }

        for (const Task& task : orderedTasks) {
            if (!selectedIds.count(task.id) || scheduledIds.count(task.id)) {
                continue;
            }
            if (dependenciesCompleted(task, scheduledIds)) {
                candidates.push_back({taskScore(task, previousTask, totalCapacity), task});
            }
        }

        if (candidates.empty()) {
            break;
        }

        sort(candidates.begin(), candidates.end(), [](const auto& left, const auto& right) {
            return left.first > right.first;
        });

        result.schedule.push_back(candidates.front().second);
        previousTask = &result.schedule.back();
    }

    result.skipped = getSkippedTasks(tasks, selectedIds);
    result.metrics = calculateScheduleMetrics(result.schedule);
    return result;
}

void printMetrics(const Metrics& metrics) {
    cout << "Tasks: " << metrics.taskCount
         << ", Load: " << metrics.cognitiveLoad
         << ", Duration: " << metrics.duration << " min"
         << ", Priority: " << metrics.priority
         << ", Switching penalty: " << metrics.switchingPenalty
         << ", Efficiency: " << metrics.efficiency << '\n';
}

void printSchedule(const ScheduleResult& result) {
    cout << "\n=== " << result.strategy << " ===\n";

    if (!result.graph.isValid) {
        cout << "Invalid dependency graph.\n";
        return;
    }

    cout << "Schedule order:\n";
    for (int i = 0; i < static_cast<int>(result.schedule.size()); i++) {
        const Task& task = result.schedule[i];
        cout << setw(2) << i + 1 << ". " << task.title
             << " [" << task.category << "]"
             << " | load=" << task.cognitiveLoad
             << " | priority=" << task.priority << '\n';
    }

    cout << "Skipped tasks: ";
    if (result.skipped.empty()) {
        cout << "None";
    } else {
        for (int i = 0; i < static_cast<int>(result.skipped.size()); i++) {
            cout << result.skipped[i].id << (i + 1 == static_cast<int>(result.skipped.size()) ? "" : ", ");
        }
    }
    cout << '\n';
    printMetrics(result.metrics);
}

void packIntoSessions(const vector<Task>& schedule, const vector<Session>& sessions) {
    cout << "\n=== Session Packing ===\n";

    vector<vector<Task>> packed(sessions.size());
    vector<int> usedCapacity(sessions.size(), 0);
    vector<int> usedMinutes(sessions.size(), 0);
    vector<Task> overflow;

    for (const Task& task : schedule) {
        bool placed = false;

        for (int i = 0; i < static_cast<int>(sessions.size()); i++) {
            if (usedCapacity[i] + task.cognitiveLoad <= sessions[i].capacity
                && usedMinutes[i] + task.duration <= sessions[i].minutes) {
                packed[i].push_back(task);
                usedCapacity[i] += task.cognitiveLoad;
                usedMinutes[i] += task.duration;
                placed = true;
                break;
            }
        }

        if (!placed) {
            overflow.push_back(task);
        }
    }

    for (int i = 0; i < static_cast<int>(sessions.size()); i++) {
        cout << sessions[i].name << " ("
             << usedCapacity[i] << "/" << sessions[i].capacity << " load, "
             << usedMinutes[i] << "/" << sessions[i].minutes << " min)\n";

        if (packed[i].empty()) {
            cout << "  No tasks assigned\n";
        } else {
            for (const Task& task : packed[i]) {
                cout << "  - " << task.title << '\n';
            }
        }
    }

    if (!overflow.empty()) {
        cout << "Overflow:\n";
        for (const Task& task : overflow) {
            cout << "  - " << task.title << '\n';
        }
    }
}

namespace AcademicSupport {

string makeBar(int value, int scale) {
    string bar;
    int count = max(1, value / max(1, scale));

    for (int i = 0; i < count; i++) {
        bar += '#';
    }

    return bar;
}

void printDivider(char symbol = '=') {
    for (int i = 0; i < 72; i++) {
        cout << symbol;
    }
    cout << '\n';
}

void printHeading(const string& heading) {
    cout << '\n';
    printDivider('=');
    cout << heading << '\n';
    printDivider('=');
}

void printSubHeading(const string& heading) {
    cout << '\n';
    cout << heading << '\n';
    printDivider('-');
}

string dependencyText(const Task& task) {
    if (task.dependencies.empty()) {
        return "None";
    }

    string text;
    for (int i = 0; i < static_cast<int>(task.dependencies.size()); i++) {
        text += task.dependencies[i];
        if (i + 1 < static_cast<int>(task.dependencies.size())) {
            text += ", ";
        }
    }

    return text;
}

void printTaskTable(const vector<Task>& tasks) {
    printSubHeading("Input Task Table");

    cout << left
         << setw(12) << "ID"
         << setw(31) << "Title"
         << setw(17) << "Category"
         << setw(6) << "Diff"
         << setw(6) << "Load"
         << setw(7) << "Time"
         << setw(6) << "Prio"
         << "Dependencies"
         << '\n';

    printDivider('.');

    for (const Task& task : tasks) {
        cout << left
             << setw(12) << task.id
             << setw(31) << task.title.substr(0, 30)
             << setw(17) << task.category
             << setw(6) << task.difficulty
             << setw(6) << task.cognitiveLoad
             << setw(7) << task.duration
             << setw(6) << task.priority
             << dependencyText(task)
             << '\n';
    }
}

void printAdjacencyList(const vector<Task>& tasks) {
    printSubHeading("Graph Representation as Adjacency List");

    unordered_map<string, vector<string>> adjacency;
    for (const Task& task : tasks) {
        adjacency[task.id] = {};
    }

    for (const Task& task : tasks) {
        for (const string& dependencyId : task.dependencies) {
            adjacency[dependencyId].push_back(task.id);
        }
    }

    for (const Task& task : tasks) {
        cout << setw(12) << left << task.id << " -> ";
        if (adjacency[task.id].empty()) {
            cout << "No outgoing edges";
        } else {
            for (int i = 0; i < static_cast<int>(adjacency[task.id].size()); i++) {
                cout << adjacency[task.id][i];
                if (i + 1 < static_cast<int>(adjacency[task.id].size())) {
                    cout << ", ";
                }
            }
        }
        cout << '\n';
    }
}

void printIndegreeTable(const vector<Task>& tasks) {
    printSubHeading("Indegree Table Used by Topological Sort");

    unordered_map<string, int> indegree;
    for (const Task& task : tasks) {
        indegree[task.id] = 0;
    }

    for (const Task& task : tasks) {
        for (const string& dependencyId : task.dependencies) {
            if (indegree.count(task.id)) {
                indegree[task.id]++;
            }
        }
    }

    for (const Task& task : tasks) {
        cout << setw(12) << left << task.id
             << " indegree = "
             << indegree[task.id]
             << '\n';
    }
}

map<string, int> calculateDependencyLevels(const vector<Task>& tasks) {
    GraphResult graph = validateDependencyGraph(tasks);
    unordered_map<string, Task> taskMap = createTaskMap(tasks);
    map<string, int> levels;

    for (const string& taskId : graph.topologicalOrder) {
        const Task& task = taskMap[taskId];
        int bestLevel = 0;

        for (const string& dependencyId : task.dependencies) {
            bestLevel = max(bestLevel, levels[dependencyId] + 1);
        }

        levels[taskId] = bestLevel;
    }

    return levels;
}

void printDependencyLevels(const vector<Task>& tasks) {
    printSubHeading("Dependency Levels");

    map<string, int> levels = calculateDependencyLevels(tasks);

    for (const auto& entry : levels) {
        cout << setw(12) << left << entry.first
             << " level "
             << entry.second
             << " "
             << makeBar(entry.second + 1, 1)
             << '\n';
    }
}

int calculateCriticalPathLength(const vector<Task>& tasks) {
    GraphResult graph = validateDependencyGraph(tasks);
    unordered_map<string, Task> taskMap = createTaskMap(tasks);
    unordered_map<string, int> bestDuration;

    for (const string& taskId : graph.topologicalOrder) {
        const Task& task = taskMap[taskId];
        int bestBefore = 0;

        for (const string& dependencyId : task.dependencies) {
            bestBefore = max(bestBefore, bestDuration[dependencyId]);
        }

        bestDuration[taskId] = bestBefore + task.duration;
    }

    int answer = 0;
    for (const auto& entry : bestDuration) {
        answer = max(answer, entry.second);
    }

    return answer;
}

void printCriticalPathSummary(const vector<Task>& tasks) {
    printSubHeading("Critical Path Estimate");

    int criticalPath = calculateCriticalPathLength(tasks);
    int totalDuration = 0;

    for (const Task& task : tasks) {
        totalDuration += task.duration;
    }

    cout << "Total work duration if all tasks are done sequentially: "
         << totalDuration
         << " minutes\n";

    cout << "Longest dependency chain duration estimate: "
         << criticalPath
         << " minutes\n";
}

map<string, vector<Task>> groupByCategory(const vector<Task>& tasks) {
    map<string, vector<Task>> groups;

    for (const Task& task : tasks) {
        groups[task.category].push_back(task);
    }

    return groups;
}

void printCategorySummary(const vector<Task>& tasks) {
    printSubHeading("Category-Wise Workload Summary");

    map<string, vector<Task>> groups = groupByCategory(tasks);

    cout << left
         << setw(18) << "Category"
         << setw(8) << "Tasks"
         << setw(8) << "Load"
         << setw(10) << "Minutes"
         << setw(10) << "Priority"
         << '\n';

    printDivider('.');

    for (const auto& entry : groups) {
        int load = 0;
        int minutes = 0;
        int priority = 0;

        for (const Task& task : entry.second) {
            load += task.cognitiveLoad;
            minutes += task.duration;
            priority += task.priority;
        }

        cout << left
             << setw(18) << entry.first
             << setw(8) << entry.second.size()
             << setw(8) << load
             << setw(10) << minutes
             << setw(10) << priority
             << '\n';
    }
}

void printLoadHistogram(const vector<Task>& tasks) {
    printSubHeading("Cognitive Load Histogram");

    for (const Task& task : tasks) {
        cout << setw(12) << left << task.id
             << " "
             << makeBar(task.cognitiveLoad, 1)
             << " ("
             << task.cognitiveLoad
             << ")"
             << '\n';
    }
}

void printPriorityHistogram(const vector<Task>& tasks) {
    printSubHeading("Priority Histogram");

    for (const Task& task : tasks) {
        cout << setw(12) << left << task.id
             << " "
             << makeBar(task.priority, 1)
             << " ("
             << task.priority
             << ")"
             << '\n';
    }
}

vector<Task> sortTasksByPriority(const vector<Task>& tasks) {
    vector<Task> sorted = tasks;

    sort(sorted.begin(), sorted.end(), [](const Task& left, const Task& right) {
        if (left.priority != right.priority) {
            return left.priority > right.priority;
        }
        return left.cognitiveLoad < right.cognitiveLoad;
    });

    return sorted;
}

vector<Task> sortTasksByLoad(const vector<Task>& tasks) {
    vector<Task> sorted = tasks;

    sort(sorted.begin(), sorted.end(), [](const Task& left, const Task& right) {
        if (left.cognitiveLoad != right.cognitiveLoad) {
            return left.cognitiveLoad > right.cognitiveLoad;
        }
        return left.priority > right.priority;
    });

    return sorted;
}

void printTopTasksByPriority(const vector<Task>& tasks) {
    printSubHeading("Tasks Sorted by Priority");

    vector<Task> sorted = sortTasksByPriority(tasks);

    for (const Task& task : sorted) {
        cout << setw(12) << left << task.id
             << " priority="
             << task.priority
             << " load="
             << task.cognitiveLoad
             << " title="
             << task.title
             << '\n';
    }
}

void printTasksByLoad(const vector<Task>& tasks) {
    printSubHeading("Tasks Sorted by Cognitive Load");

    vector<Task> sorted = sortTasksByLoad(tasks);

    for (const Task& task : sorted) {
        cout << setw(12) << left << task.id
             << " load="
             << task.cognitiveLoad
             << " priority="
             << task.priority
             << " title="
             << task.title
             << '\n';
    }
}

vector<Task> getTasksAboveLoad(const vector<Task>& tasks, int threshold) {
    vector<Task> result;

    for (const Task& task : tasks) {
        if (task.cognitiveLoad >= threshold) {
            result.push_back(task);
        }
    }

    return result;
}

void printHighLoadTasks(const vector<Task>& tasks, int threshold) {
    printSubHeading("High Cognitive Load Tasks");

    vector<Task> highLoadTasks = getTasksAboveLoad(tasks, threshold);

    if (highLoadTasks.empty()) {
        cout << "No task has load >= " << threshold << '\n';
        return;
    }

    for (const Task& task : highLoadTasks) {
        cout << task.id
             << " needs load "
             << task.cognitiveLoad
             << " and should be placed in a strong focus slot.\n";
    }
}

vector<Task> getBottleneckTasks(const vector<Task>& tasks) {
    unordered_map<string, int> outgoingCount;

    for (const Task& task : tasks) {
        outgoingCount[task.id] = 0;
    }

    for (const Task& task : tasks) {
        for (const string& dependencyId : task.dependencies) {
            outgoingCount[dependencyId]++;
        }
    }

    vector<Task> bottlenecks;

    for (const Task& task : tasks) {
        if (outgoingCount[task.id] >= 2) {
            bottlenecks.push_back(task);
        }
    }

    sort(bottlenecks.begin(), bottlenecks.end(), [&](const Task& left, const Task& right) {
        return outgoingCount[left.id] > outgoingCount[right.id];
    });

    return bottlenecks;
}

void printBottleneckTasks(const vector<Task>& tasks) {
    printSubHeading("Bottleneck Tasks");

    vector<Task> bottlenecks = getBottleneckTasks(tasks);

    if (bottlenecks.empty()) {
        cout << "No major bottleneck tasks found.\n";
        return;
    }

    unordered_map<string, int> outgoingCount;
    for (const Task& task : tasks) {
        outgoingCount[task.id] = 0;
    }
    for (const Task& task : tasks) {
        for (const string& dependencyId : task.dependencies) {
            outgoingCount[dependencyId]++;
        }
    }

    for (const Task& task : bottlenecks) {
        cout << setw(12) << left << task.id
             << " unlocks "
             << outgoingCount[task.id]
             << " future tasks\n";
    }
}

void printCapacityExperiment(const vector<Task>& tasks, const vector<int>& capacities) {
    printSubHeading("Capacity Experiment");

    cout << left
         << setw(10) << "Budget"
         << setw(14) << "GreedyLoad"
         << setw(14) << "GreedyEff"
         << setw(10) << "DPload"
         << setw(10) << "DPeff"
         << setw(10) << "DPtasks"
         << '\n';

    printDivider('.');

    for (int capacity : capacities) {
        ScheduleResult greedy = runGreedyScheduler(tasks, capacity);
        ScheduleResult dp = runDpScheduler(tasks, capacity);

        cout << left
             << setw(10) << capacity
             << setw(14) << greedy.metrics.cognitiveLoad
             << setw(14) << greedy.metrics.efficiency
             << setw(10) << dp.metrics.cognitiveLoad
             << setw(10) << dp.metrics.efficiency
             << setw(10) << dp.metrics.taskCount
             << '\n';
    }
}

void printScheduleAsIds(const vector<Task>& schedule) {
    if (schedule.empty()) {
        cout << "No tasks selected";
        return;
    }

    for (int i = 0; i < static_cast<int>(schedule.size()); i++) {
        cout << schedule[i].id;
        if (i + 1 < static_cast<int>(schedule.size())) {
            cout << " -> ";
        }
    }
}

void printStrategyComparison(const ScheduleResult& greedy, const ScheduleResult& dp) {
    printSubHeading("Strategy Comparison");

    cout << "Greedy order: ";
    printScheduleAsIds(greedy.schedule);
    cout << '\n';

    cout << "DP order:     ";
    printScheduleAsIds(dp.schedule);
    cout << '\n';

    cout << "Greedy efficiency: "
         << greedy.metrics.efficiency
         << '\n';

    cout << "DP efficiency: "
         << dp.metrics.efficiency
         << '\n';

    if (dp.metrics.efficiency > greedy.metrics.efficiency) {
        cout << "DP performs better for this capacity because it explores combinations.\n";
    } else if (dp.metrics.efficiency < greedy.metrics.efficiency) {
        cout << "Greedy performs better for this capacity because local choices fit well.\n";
    } else {
        cout << "Both strategies produce the same efficiency for this capacity.\n";
    }
}

vector<string> buildVivaExplanationLines() {
    return {
        "This project models scheduling as an optimization problem.",
        "Every task has priority, duration, difficulty, cognitive load, and dependencies.",
        "Cognitive load is treated as a limited resource similar to knapsack capacity.",
        "Dependencies are represented through a directed graph.",
        "The graph should be a DAG because cyclic prerequisites cannot be completed.",
        "Topological sorting is used to check whether the dependency graph is feasible.",
        "Indegree stores how many prerequisites are still pending for each task.",
        "A queue stores all tasks whose indegree has become zero.",
        "If topological sorting visits all tasks, then the graph has no cycle.",
        "If some tasks are not visited, then a cycle exists.",
        "The greedy scheduler selects one feasible task at a time.",
        "A feasible task is one whose dependencies are already completed.",
        "The greedy score combines priority, difficulty, load, and switching penalty.",
        "Higher priority increases the chance of selection.",
        "Higher cognitive load reduces the score because it consumes more energy.",
        "The switching penalty discourages jumping between unrelated categories.",
        "The dynamic programming scheduler solves a capacity-constrained selection problem.",
        "The DP table has dimensions task index and remaining cognitive capacity.",
        "Each DP state stores the best value achievable for a prefix of tasks.",
        "A task is included only when all its dependencies are already selected.",
        "This keeps the DP result dependency-safe.",
        "Session packing distributes selected tasks into morning, afternoon, and evening.",
        "Each session has a cognitive capacity and a time duration limit.",
        "Packing is similar to bin packing with two constraints.",
        "Metrics help compare different schedules.",
        "Efficiency rewards priority and penalizes cognitive load and switching.",
        "The project is deterministic and does not use AI or machine learning.",
        "All results can be explained through classical DAA concepts.",
        "The main data structures are arrays, vectors, maps, sets, queue, and graph.",
        "The algorithmic ideas are DAG validation, greedy choice, DP, and packing.",
        "This makes the system suitable for a DAA lab project."
    };
}

void printVivaExplanation() {
    printSubHeading("Short Viva Explanation");

    vector<string> lines = buildVivaExplanationLines();

    for (int i = 0; i < static_cast<int>(lines.size()); i++) {
        cout << setw(2) << i + 1
             << ". "
             << lines[i]
             << '\n';
    }
}

void printComplexityAnalysis(int taskCount, int capacity) {
    printSubHeading("Complexity Analysis");

    cout << "Let n be the number of tasks and C be the cognitive capacity.\n";
    cout << "Here, n = " << taskCount << " and C = " << capacity << ".\n";
    cout << "Graph validation using topological sort: O(n + e).\n";
    cout << "Greedy scheduling: O(n^2 log n) in this simple implementation.\n";
    cout << "Dynamic programming: O(n * C * dependency-check-cost).\n";
    cout << "Session packing: O(n * number_of_sessions).\n";
    cout << "Space for DP table: O(n * C) plus selected task sets.\n";
}

vector<Task> makeMiniDataset() {
    return {
        {"a", "Read problem statement", "Analysis", 1, 2, 20, 5, {}},
        {"b", "Draw graph", "Design", 2, 3, 30, 7, {"a"}},
        {"c", "Code algorithm", "Implementation", 4, 6, 80, 9, {"b"}},
        {"d", "Test cases", "Evaluation", 3, 4, 40, 8, {"c"}},
        {"e", "Final explanation", "Documentation", 2, 2, 25, 6, {"d"}},
    };
}

vector<Task> makeCycleDataset() {
    return {
        {"x", "Task X", "Demo", 1, 2, 10, 5, {"z"}},
        {"y", "Task Y", "Demo", 1, 2, 10, 5, {"x"}},
        {"z", "Task Z", "Demo", 1, 2, 10, 5, {"y"}},
    };
}

void printValidationDemo() {
    printSubHeading("Validation Demo with a Cyclic Dataset");

    vector<Task> cycleTasks = makeCycleDataset();
    GraphResult result = validateDependencyGraph(cycleTasks);

    cout << "Cycle dataset valid: "
         << (result.isValid ? "Yes" : "No")
         << '\n';

    cout << "Cycle detected: "
         << (result.hasCycle ? "Yes" : "No")
         << '\n';
}

void printMiniDatasetDemo() {
    printSubHeading("Mini Dataset Demo");

    vector<Task> miniTasks = makeMiniDataset();
    ScheduleResult greedy = runGreedyScheduler(miniTasks, 12);
    ScheduleResult dp = runDpScheduler(miniTasks, 12);

    cout << "Mini greedy order: ";
    printScheduleAsIds(greedy.schedule);
    cout << '\n';

    cout << "Mini DP order:     ";
    printScheduleAsIds(dp.schedule);
    cout << '\n';
}

double calculateAverageLoad(const vector<Task>& tasks) {
    if (tasks.empty()) {
        return 0.0;
    }

    int total = 0;
    for (const Task& task : tasks) {
        total += task.cognitiveLoad;
    }

    return static_cast<double>(total) / tasks.size();
}

double calculateAveragePriority(const vector<Task>& tasks) {
    if (tasks.empty()) {
        return 0.0;
    }

    int total = 0;
    for (const Task& task : tasks) {
        total += task.priority;
    }

    return static_cast<double>(total) / tasks.size();
}

double calculateAverageDuration(const vector<Task>& tasks) {
    if (tasks.empty()) {
        return 0.0;
    }

    int total = 0;
    for (const Task& task : tasks) {
        total += task.duration;
    }

    return static_cast<double>(total) / tasks.size();
}

void printDatasetAverages(const vector<Task>& tasks) {
    printSubHeading("Dataset Averages");

    cout << fixed << setprecision(2);
    cout << "Average load: "
         << calculateAverageLoad(tasks)
         << '\n';
    cout << "Average priority: "
         << calculateAveragePriority(tasks)
         << '\n';
    cout << "Average duration: "
         << calculateAverageDuration(tasks)
         << " minutes\n";
    cout.unsetf(ios::floatfield);
}

int countDependencyEdges(const vector<Task>& tasks) {
    int edgeCount = 0;

    for (const Task& task : tasks) {
        edgeCount += static_cast<int>(task.dependencies.size());
    }

    return edgeCount;
}

void printGraphStats(const vector<Task>& tasks) {
    printSubHeading("Graph Statistics");

    int nodes = static_cast<int>(tasks.size());
    int edges = countDependencyEdges(tasks);

    cout << "Nodes/tasks: "
         << nodes
         << '\n';

    cout << "Edges/dependencies: "
         << edges
         << '\n';

    cout << "Average dependencies per task: ";
    if (nodes == 0) {
        cout << "0\n";
    } else {
        cout << fixed << setprecision(2) << static_cast<double>(edges) / nodes << '\n';
        cout.unsetf(ios::floatfield);
    }
}

vector<Task> getLeafTasks(const vector<Task>& tasks) {
    unordered_map<string, int> outgoingCount;

    for (const Task& task : tasks) {
        outgoingCount[task.id] = 0;
    }

    for (const Task& task : tasks) {
        for (const string& dependencyId : task.dependencies) {
            outgoingCount[dependencyId]++;
        }
    }

    vector<Task> leaves;
    for (const Task& task : tasks) {
        if (outgoingCount[task.id] == 0) {
            leaves.push_back(task);
        }
    }

    return leaves;
}

void printLeafTasks(const vector<Task>& tasks) {
    printSubHeading("Leaf Tasks");

    vector<Task> leaves = getLeafTasks(tasks);

    for (const Task& task : leaves) {
        cout << task.id
             << " has no dependent tasks after it.\n";
    }
}

vector<Task> getRootTasks(const vector<Task>& tasks) {
    vector<Task> roots;

    for (const Task& task : tasks) {
        if (task.dependencies.empty()) {
            roots.push_back(task);
        }
    }

    return roots;
}

void printRootTasks(const vector<Task>& tasks) {
    printSubHeading("Root Tasks");

    vector<Task> roots = getRootTasks(tasks);

    for (const Task& task : roots) {
        cout << task.id
             << " can start immediately because it has no prerequisites.\n";
    }
}

void printReadableSchedule(const vector<Task>& schedule) {
    printSubHeading("Readable Schedule");

    int currentMinute = 0;

    for (const Task& task : schedule) {
        cout << setw(4) << currentMinute
             << " - "
             << setw(4) << currentMinute + task.duration
             << " min : "
             << task.title
             << '\n';

        currentMinute += task.duration;
    }
}

void printEnergyTimeline(const vector<Task>& schedule) {
    printSubHeading("Energy Consumption Timeline");

    int cumulativeLoad = 0;

    for (const Task& task : schedule) {
        cumulativeLoad += task.cognitiveLoad;

        cout << setw(12) << left << task.id
             << " cumulative load "
             << setw(3) << cumulativeLoad
             << " "
             << makeBar(cumulativeLoad, 2)
             << '\n';
    }
}

void printDependencyMatrix(const vector<Task>& tasks) {
    printSubHeading("Dependency Matrix");

    cout << setw(10) << left << "";
    for (const Task& task : tasks) {
        cout << setw(4) << task.id.substr(0, 3);
    }
    cout << '\n';

    for (const Task& rowTask : tasks) {
        cout << setw(10) << left << rowTask.id.substr(0, 9);
        set<string> deps(rowTask.dependencies.begin(), rowTask.dependencies.end());

        for (const Task& columnTask : tasks) {
            cout << setw(4) << (deps.count(columnTask.id) ? 1 : 0);
        }
        cout << '\n';
    }
}

void printScoreBreakdown(const vector<Task>& tasks, int capacity) {
    printSubHeading("Initial Greedy Score Breakdown");

    const Task* previousTask = nullptr;

    cout << left
         << setw(12) << "Task"
         << setw(10) << "Priority"
         << setw(10) << "Diff"
         << setw(10) << "Load"
         << setw(10) << "Score"
         << '\n';

    printDivider('.');

    for (const Task& task : tasks) {
        cout << left
             << setw(12) << task.id
             << setw(10) << task.priority
             << setw(10) << task.difficulty
             << setw(10) << task.cognitiveLoad
             << setw(10) << fixed << setprecision(2) << taskScore(task, previousTask, capacity)
             << '\n';
    }

    cout.unsetf(ios::floatfield);
}

void printSessionCapacities(const vector<Session>& sessions) {
    printSubHeading("Session Capacities");

    for (const Session& session : sessions) {
        cout << setw(24) << left << session.name
             << " capacity="
             << setw(3) << session.capacity
             << " minutes="
             << session.minutes
             << '\n';
    }
}

void printProjectMappingTable() {
    printSubHeading("DAA Concept Mapping");

    cout << left
         << setw(24) << "DAA Concept"
         << "Project Use\n";

    printDivider('.');

    cout << setw(24) << "Graph"
         << "Task dependencies and prerequisites\n";
    cout << setw(24) << "Topological Sort"
         << "Checks whether all tasks can be ordered\n";
    cout << setw(24) << "Queue"
         << "Stores zero-indegree tasks during graph validation\n";
    cout << setw(24) << "Greedy"
         << "Selects the best currently feasible task\n";
    cout << setw(24) << "Dynamic Programming"
         << "Optimizes task value under cognitive budget\n";
    cout << setw(24) << "Bin Packing"
         << "Places tasks into limited work sessions\n";
    cout << setw(24) << "Set"
         << "Tracks completed and selected tasks\n";
    cout << setw(24) << "Map"
         << "Supports fast lookup by task id\n";
}

void printAlgorithmSteps() {
    printSubHeading("Algorithm Steps");

    vector<string> steps = {
        "Read all tasks and their dependency lists.",
        "Build graph using dependency edges.",
        "Compute indegree of every task.",
        "Run topological sort to validate the graph.",
        "Reject the schedule if a cycle or missing dependency exists.",
        "For greedy scheduling, repeatedly find feasible tasks.",
        "Score each feasible task using priority, load, difficulty, and switching cost.",
        "Pick the highest scoring task and reduce remaining capacity.",
        "For DP scheduling, process tasks in topological order.",
        "For every task and capacity, decide whether to include or exclude the task.",
        "Only include a task when all dependencies are already selected.",
        "Reconstruct a valid schedule from the selected DP task set.",
        "Calculate final metrics.",
        "Pack tasks into sessions with capacity and time limits.",
        "Print the final result for comparison and explanation."
    };

    for (int i = 0; i < static_cast<int>(steps.size()); i++) {
        cout << setw(2) << i + 1
             << ". "
             << steps[i]
             << '\n';
    }
}

void printSubmissionChecklist() {
    printSubHeading("Submission Checklist");

    vector<string> checklist = {
        "Problem statement is clear.",
        "Input task attributes are defined.",
        "Graph representation is explained.",
        "Topological sorting is implemented.",
        "Greedy scheduling is implemented.",
        "Dynamic programming optimization is implemented.",
        "Session packing is implemented.",
        "Metrics are printed.",
        "Complexity analysis is available.",
        "Sample output is generated."
    };

    for (const string& item : checklist) {
        cout << "[x] " << item << '\n';
    }
}

void printCompleteAcademicReport(
    const vector<Task>& tasks,
    const vector<Session>& sessions,
    const ScheduleResult& greedy,
    const ScheduleResult& dp,
    int capacity
) {
    printHeading("Academic Console Report");
    printTaskTable(tasks);
    printGraphStats(tasks);
    printAdjacencyList(tasks);
    printIndegreeTable(tasks);
    printDependencyLevels(tasks);
    printDependencyMatrix(tasks);
    printRootTasks(tasks);
    printLeafTasks(tasks);
    printBottleneckTasks(tasks);
    printCriticalPathSummary(tasks);
    printCategorySummary(tasks);
    printDatasetAverages(tasks);
    printLoadHistogram(tasks);
    printPriorityHistogram(tasks);
    printTopTasksByPriority(tasks);
    printTasksByLoad(tasks);
    printHighLoadTasks(tasks, 7);
    printScoreBreakdown(tasks, capacity);
    printSessionCapacities(sessions);
    printStrategyComparison(greedy, dp);
    printReadableSchedule(dp.schedule);
    printEnergyTimeline(dp.schedule);
    printCapacityExperiment(tasks, {15, 20, 25, 30, 35, 40, 45});
    printComplexityAnalysis(static_cast<int>(tasks.size()), capacity);
    printProjectMappingTable();
    printAlgorithmSteps();
    printValidationDemo();
    printMiniDatasetDemo();
    printVivaExplanation();
    printSubmissionChecklist();
}

}

int main() {
    vector<Task> tasks = getSampleTasks();
    vector<Session> sessions = getSessions();
    int totalCapacity = 35;

    GraphResult graph = validateDependencyGraph(tasks);
    cout << "Cognitive Load-Aware Task Scheduler\n";
    cout << "Dependency graph valid: " << (graph.isValid ? "Yes" : "No") << '\n';
    cout << "Topological order: ";
    for (int i = 0; i < static_cast<int>(graph.topologicalOrder.size()); i++) {
        cout << graph.topologicalOrder[i] << (i + 1 == static_cast<int>(graph.topologicalOrder.size()) ? "" : " -> ");
    }
    cout << "\nCognitive budget: " << totalCapacity << "\n";

    ScheduleResult greedy = runGreedyScheduler(tasks, totalCapacity);
    ScheduleResult dp = runDpScheduler(tasks, totalCapacity);

    printSchedule(greedy);
    printSchedule(dp);
    packIntoSessions(dp.schedule, sessions);
    AcademicSupport::printCompleteAcademicReport(tasks, sessions, greedy, dp, totalCapacity);

    return 0;
}
