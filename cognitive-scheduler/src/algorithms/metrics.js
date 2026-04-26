export function getSwitchingCost(previousTask, nextTask) {
  if (!previousTask || !nextTask) return 0
  if (previousTask.category === nextTask.category) return 1

  const difficultyJump = Math.abs(previousTask.difficulty - nextTask.difficulty)
  return 2 + difficultyJump
}

export function taskScore(task, previousTask = null, remainingCapacity = Infinity) {
  const switchingCost = getSwitchingCost(previousTask, task)
  const capacityFitBonus = Math.max(0, remainingCapacity - task.cognitiveLoad) * 0.12

  return (
    task.priority * 3 +
    task.difficulty * 1.2 -
    task.cognitiveLoad * 0.9 -
    switchingCost * 1.4 +
    capacityFitBonus
  )
}

export function calculateScheduleMetrics(schedule) {
  const totals = schedule.reduce(
    (summary, task, index) => {
      const previousTask = schedule[index - 1]
      const switchingCost = getSwitchingCost(previousTask, task)

      return {
        cognitiveLoad: summary.cognitiveLoad + task.cognitiveLoad,
        duration: summary.duration + task.duration,
        priority: summary.priority + task.priority,
        switchingPenalty: summary.switchingPenalty + switchingCost,
      }
    },
    { cognitiveLoad: 0, duration: 0, priority: 0, switchingPenalty: 0 },
  )

  return {
    ...totals,
    taskCount: schedule.length,
    efficiency: totals.priority * 10 - totals.switchingPenalty * 4 - totals.cognitiveLoad,
  }
}

export function formatDependencyList(task) {
  return task.dependencies.length > 0 ? task.dependencies.join(', ') : 'None'
}
