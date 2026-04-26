import { calculateScheduleMetrics } from './metrics.js'

export function packIntoSessions(schedule, sessions) {
  const packedSessions = sessions.map((session) => ({
    ...session,
    tasks: [],
    usedCapacity: 0,
    usedMinutes: 0,
  }))
  const overflow = []

  schedule.forEach((task) => {
    const targetSession = packedSessions.find(
      (session) =>
        session.usedCapacity + task.cognitiveLoad <= session.capacity &&
        session.usedMinutes + task.duration <= session.minutes,
    )

    if (!targetSession) {
      overflow.push(task)
      return
    }

    targetSession.tasks.push(task)
    targetSession.usedCapacity += task.cognitiveLoad
    targetSession.usedMinutes += task.duration
  })

  return {
    sessions: packedSessions.map((session) => ({
      ...session,
      metrics: calculateScheduleMetrics(session.tasks),
    })),
    overflow,
  }
}
