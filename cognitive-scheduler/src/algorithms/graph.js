export function createTaskMap(tasks) {
  return new Map(tasks.map((task) => [task.id, task]))
}

export function validateDependencyGraph(tasks) {
  const taskMap = createTaskMap(tasks)
  const missingDependencies = []

  tasks.forEach((task) => {
    task.dependencies.forEach((dependencyId) => {
      if (!taskMap.has(dependencyId)) {
        missingDependencies.push({ taskId: task.id, dependencyId })
      }
    })
  })

  const indegree = new Map(tasks.map((task) => [task.id, 0]))
  const adjacency = new Map(tasks.map((task) => [task.id, []]))

  tasks.forEach((task) => {
    task.dependencies.forEach((dependencyId) => {
      if (!taskMap.has(dependencyId)) return
      adjacency.get(dependencyId).push(task.id)
      indegree.set(task.id, indegree.get(task.id) + 1)
    })
  })

  const queue = tasks
    .filter((task) => indegree.get(task.id) === 0)
    .map((task) => task.id)
  const order = []

  while (queue.length > 0) {
    const taskId = queue.shift()
    order.push(taskId)

    adjacency.get(taskId).forEach((nextTaskId) => {
      indegree.set(nextTaskId, indegree.get(nextTaskId) - 1)
      if (indegree.get(nextTaskId) === 0) {
        queue.push(nextTaskId)
      }
    })
  }

  return {
    isValid: missingDependencies.length === 0 && order.length === tasks.length,
    hasCycle: order.length !== tasks.length,
    missingDependencies,
    topologicalOrder: order,
  }
}

export function getAvailableTasks(tasks, completedIds) {
  return tasks.filter(
    (task) =>
      !completedIds.has(task.id) &&
      task.dependencies.every((dependencyId) => completedIds.has(dependencyId)),
  )
}
