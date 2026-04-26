import { getAvailableTasks, validateDependencyGraph } from './graph.js'
import { calculateScheduleMetrics, taskScore } from './metrics.js'

export function runGreedyScheduler(tasks, totalCapacity) {
  const graph = validateDependencyGraph(tasks)

  if (!graph.isValid) {
    return {
      strategy: 'Greedy Priority Queue',
      schedule: [],
      skipped: tasks,
      metrics: calculateScheduleMetrics([]),
      graph,
      explanation: ['Invalid dependency graph. Fix missing dependencies or cycles first.'],
    }
  }

  const completedIds = new Set()
  const schedule = []
  const explanation = []
  let remainingCapacity = totalCapacity
  let previousTask = null

  while (completedIds.size < tasks.length) {
    const feasibleTasks = getAvailableTasks(tasks, completedIds)
      .filter((task) => task.cognitiveLoad <= remainingCapacity)
      .map((task) => ({
        task,
        score: taskScore(task, previousTask, remainingCapacity),
      }))
      .sort((a, b) => b.score - a.score || b.task.priority - a.task.priority)

    if (feasibleTasks.length === 0) break

    const selected = feasibleTasks[0]
    schedule.push(selected.task)
    completedIds.add(selected.task.id)
    remainingCapacity -= selected.task.cognitiveLoad
    explanation.push(
      `${selected.task.title} selected with score ${selected.score.toFixed(2)} using priority, load, and switching cost.`,
    )
    previousTask = selected.task
  }

  return {
    strategy: 'Greedy Priority Queue',
    schedule,
    skipped: tasks.filter((task) => !completedIds.has(task.id)),
    metrics: calculateScheduleMetrics(schedule),
    graph,
    explanation,
  }
}
