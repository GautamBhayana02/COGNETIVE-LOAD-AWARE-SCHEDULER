import './style.css'
import { runDpScheduler } from './algorithms/dpScheduler.js'
import { runGreedyScheduler } from './algorithms/greedyScheduler.js'
import { formatDependencyList } from './algorithms/metrics.js'
import { packIntoSessions } from './algorithms/sessionPacking.js'
import { cognitiveSessions, sampleTasks } from './data/tasks.js'

const state = {
  strategy: 'compare',
  totalCapacity: 36,
}

const app = document.querySelector('#app')

function runSchedulers() {
  const greedy = runGreedyScheduler(sampleTasks, state.totalCapacity)
  const optimized = runDpScheduler(sampleTasks, state.totalCapacity)
  const selected = state.strategy === 'greedy' ? greedy : optimized

  return {
    greedy,
    optimized,
    selected: state.strategy === 'compare' ? optimized : selected,
  }
}

function renderMetric(label, value, helper = '') {
  return `
    <div class="metric">
      <span>${label}</span>
      <strong>${value}</strong>
      ${helper ? `<small>${helper}</small>` : ''}
    </div>
  `
}

function renderTaskTable(tasks) {
  return `
    <div class="table-wrap" aria-label="Task data table">
      <table>
        <thead>
          <tr>
            <th>Task</th>
            <th>Category</th>
            <th>Load</th>
            <th>Priority</th>
            <th>Duration</th>
            <th>Dependencies</th>
          </tr>
        </thead>
        <tbody>
          ${tasks
            .map(
              (task) => `
                <tr>
                  <td>
                    <strong>${task.title}</strong>
                    <span>${task.id}</span>
                  </td>
                  <td>${task.category}</td>
                  <td>${task.cognitiveLoad}</td>
                  <td>${task.priority}</td>
                  <td>${task.duration} min</td>
                  <td>${formatDependencyList(task)}</td>
                </tr>
              `,
            )
            .join('')}
        </tbody>
      </table>
    </div>
  `
}

function renderSchedule(result) {
  const packed = packIntoSessions(result.schedule, cognitiveSessions)

  return `
    <section class="panel output-panel">
      <div class="section-heading">
        <div>
          <p class="eyebrow">Selected schedule</p>
          <h2>${result.strategy}</h2>
        </div>
        <span class="status-pill">${result.graph.isValid ? 'Valid DAG' : 'Invalid Graph'}</span>
      </div>

      <div class="metrics-grid">
        ${renderMetric('Tasks', result.metrics.taskCount, 'scheduled')}
        ${renderMetric('Cognitive Load', result.metrics.cognitiveLoad, `of ${state.totalCapacity}`)}
        ${renderMetric('Switch Penalty', result.metrics.switchingPenalty, 'lower is better')}
        ${renderMetric('Efficiency', result.metrics.efficiency, 'priority adjusted')}
      </div>

      <ol class="timeline">
        ${
          result.schedule.length
            ? result.schedule
                .map(
                  (task, index) => `
                    <li>
                      <span>${index + 1}</span>
                      <div>
                        <strong>${task.title}</strong>
                        <p>${task.category} | load ${task.cognitiveLoad} | priority ${task.priority}</p>
                      </div>
                    </li>
                  `,
                )
                .join('')
            : '<li><span>!</span><div><strong>No feasible tasks</strong><p>Increase cognitive capacity or fix dependencies.</p></div></li>'
        }
      </ol>

      <div class="session-grid">
        ${packed.sessions
          .map(
            (session) => `
              <article class="session-card">
                <div>
                  <strong>${session.name}</strong>
                  <span>${session.usedCapacity}/${session.capacity} load, ${session.usedMinutes}/${session.minutes} min</span>
                </div>
                <ul>
                  ${
                    session.tasks.length
                      ? session.tasks.map((task) => `<li>${task.title}</li>`).join('')
                      : '<li>No task assigned</li>'
                  }
                </ul>
              </article>
            `,
          )
          .join('')}
      </div>

      ${
        result.skipped.length
          ? `<p class="note">Skipped because of capacity or dependency constraints: ${result.skipped
              .map((task) => task.title)
              .join(', ')}.</p>`
          : '<p class="note">All tasks fit inside the selected cognitive budget.</p>'
      }
      ${
        packed.overflow.length
          ? `<p class="note warning-note">Scheduled but not placed in a session because of session time limits: ${packed.overflow
              .map((task) => task.title)
              .join(', ')}.</p>`
          : ''
      }
    </section>
  `
}

function renderComparison(greedy, optimized) {
  return `
    <section class="panel">
      <div class="section-heading">
        <div>
          <p class="eyebrow">Algorithm comparison</p>
          <h2>Greedy vs Dynamic Programming</h2>
        </div>
      </div>

      <div class="comparison-grid">
        ${[greedy, optimized]
          .map(
            (result) => `
              <article class="comparison-card">
                <h3>${result.strategy}</h3>
                <div class="compact-metrics">
                  ${renderMetric('Tasks', result.metrics.taskCount)}
                  ${renderMetric('Load', result.metrics.cognitiveLoad)}
                  ${renderMetric('Penalty', result.metrics.switchingPenalty)}
                  ${renderMetric('Score', result.metrics.efficiency)}
                </div>
                <p>${result.explanation[0] || 'Schedule generated from deterministic DAA rules.'}</p>
              </article>
            `,
          )
          .join('')}
      </div>
    </section>
  `
}

function renderControls() {
  return `
    <section class="control-bar" aria-label="Scheduler controls">
      <label>
        Strategy
        <select id="strategy">
          <option value="compare" ${state.strategy === 'compare' ? 'selected' : ''}>Compare both</option>
          <option value="greedy" ${state.strategy === 'greedy' ? 'selected' : ''}>Greedy priority queue</option>
          <option value="dp" ${state.strategy === 'dp' ? 'selected' : ''}>Dynamic programming</option>
        </select>
      </label>
      <label>
        Cognitive budget
        <input id="capacity" type="range" min="18" max="48" value="${state.totalCapacity}" />
      </label>
      <output>${state.totalCapacity} load units</output>
    </section>
  `
}

function render() {
  const { greedy, optimized, selected } = runSchedulers()

  app.innerHTML = `
    <header class="hero-section">
      <div>
        <p class="eyebrow">DAA Lab Project | Even 2026</p>
        <h1>Cognitive Load-Aware Task Scheduling System</h1>
        <p class="hero-copy">
          A deterministic scheduler that treats mental energy as a limited resource and uses classical
          data structures and algorithms to produce explainable task plans.
        </p>
      </div>
      <div class="hero-visual" aria-label="Algorithm concept diagram">
        <div class="node primary">DAG</div>
        <div class="connector"></div>
        <div class="node">Heap</div>
        <div class="node">DP</div>
        <div class="node">Sessions</div>
      </div>
    </header>

    <main>
      ${renderControls()}

      <section class="panel">
        <div class="section-heading">
          <div>
            <p class="eyebrow">Input data</p>
            <h2>Tasks, dependencies, and cognitive cost</h2>
          </div>
          <span class="status-pill">${sampleTasks.length} tasks</span>
        </div>
        ${renderTaskTable(sampleTasks)}
      </section>

      ${renderSchedule(selected)}
      ${state.strategy === 'compare' ? renderComparison(greedy, optimized) : ''}

      <section class="panel concepts">
        <div>
          <p class="eyebrow">Data structures used</p>
          <h2>Mapping of DAA concepts</h2>
        </div>
        <div class="concept-grid">
          <article><strong>Graph (DAG)</strong><span>Maintains prerequisite order and detects cycles.</span></article>
          <article><strong>Priority Queue</strong><span>Selects the best feasible task using a score.</span></article>
          <article><strong>Dynamic Programming</strong><span>Optimizes output under cognitive capacity.</span></article>
          <article><strong>Bin Packing</strong><span>Groups selected tasks into focus sessions.</span></article>
        </div>
      </section>
    </main>
  `

  document.querySelector('#strategy').addEventListener('change', (event) => {
    state.strategy = event.target.value
    render()
  })

  document.querySelector('#capacity').addEventListener('input', (event) => {
    state.totalCapacity = Number(event.target.value)
    render()
  })
}

render()
