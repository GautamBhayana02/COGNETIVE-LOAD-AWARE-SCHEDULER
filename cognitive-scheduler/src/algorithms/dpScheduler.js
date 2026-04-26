import { validateDependencyGraph } from './graph.js'
import { calculateScheduleMetrics, taskScore } from './metrics.js'

function respectsDependencies(candidateIds, task) {
  return task.dependencies.every((dependencyId) => candidateIds.has(dependencyId))
}

export function runDpScheduler(tasks, totalCapacity) {
  const graph = validateDependencyGraph(tasks)

  if (!graph.isValid) {
    return {
      strategy: 'Dynamic Programming Optimizer',
      schedule: [],
      skipped: tasks,
      metrics: calculateScheduleMetrics([]),
      graph,
      explanation: ['Invalid dependency graph. DP optimization needs a valid DAG.'],
    }
  }

  const orderedTasks = graph.topologicalOrder.map((taskId) =>
    tasks.find((task) => task.id === taskId),
  )
  const dp = Array.from({ length: orderedTasks.length + 1 }, () =>
    Array.from({ length: totalCapacity + 1 }, () => ({ value: 0, ids: new Set() })),
  )

  for (let i = 1; i <= orderedTasks.length; i += 1) {
    const task = orderedTasks[i - 1]

    for (let capacity = 0; capacity <= totalCapacity; capacity += 1) {
      const withoutTask = dp[i - 1][capacity]
      let best = withoutTask

      if (task.cognitiveLoad <= capacity) {
        const previous = dp[i - 1][capacity - task.cognitiveLoad]
        if (respectsDependencies(previous.ids, task)) {
          const value =
            previous.value + task.priority * 10 + task.difficulty * 2 - task.cognitiveLoad
          const ids = new Set(previous.ids)
          ids.add(task.id)

          if (value > best.value) {
            best = { value, ids }
          }
        }
      }

      dp[i][capacity] = best
    }
  }

  const selectedIds = dp[orderedTasks.length][totalCapacity].ids
  const schedule = []
  let previousTask = null

  while (schedule.length < selectedIds.size) {
    const candidates = orderedTasks
      .filter(
        (task) =>
          selectedIds.has(task.id) &&
          !schedule.some((scheduledTask) => scheduledTask.id === task.id) &&
          task.dependencies.every((dependencyId) =>
            schedule.some((scheduledTask) => scheduledTask.id === dependencyId),
          ),
      )
      .sort((a, b) => taskScore(b, previousTask) - taskScore(a, previousTask))

    if (candidates.length === 0) break
    schedule.push(candidates[0])
    previousTask = candidates[0]
  }

  return {
    strategy: 'Dynamic Programming Optimizer',
    schedule,
    skipped: tasks.filter((task) => !selectedIds.has(task.id)),
    metrics: calculateScheduleMetrics(schedule),
    graph,
    explanation: [
      `Evaluated ${orderedTasks.length} tasks across ${totalCapacity + 1} capacity states.`,
      'Selected the highest-value dependency-safe subset under the cognitive budget.',
    ],
  }
}
