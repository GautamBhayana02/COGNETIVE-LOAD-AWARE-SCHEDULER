import './style.css'
import { runDpScheduler } from './algorithms/dpScheduler.js'
import { runGreedyScheduler } from './algorithms/greedyScheduler.js'
import { formatDependencyList } from './algorithms/metrics.js'
import { packIntoSessions } from './algorithms/sessionPacking.js'
import { cognitiveSessions, sampleTasks } from './data/tasks.js'

const state = {
  strategy: 'compare',
  totalCapacity: 36,
  tasks: sampleTasks.map((task) => ({ ...task, dependencies: [...task.dependencies] })),
}

const app = document.querySelector('#app')

function runSchedulers() {
  const greedy = runGreedyScheduler(state.tasks, state.totalCapacity)
  const optimized = runDpScheduler(state.tasks, state.totalCapacity)
  const selected = state.strategy === 'greedy' ? greedy : optimized

  return {
    greedy,
    optimized,
    selected: state.strategy === 'compare' ? optimized : selected,
  }
}

function escapeHtml(value) {
  return String(value)
    .replaceAll('&', '&amp;')
    .replaceAll('<', '&lt;')
    .replaceAll('>', '&gt;')
    .replaceAll('"', '&quot;')
    .replaceAll("'", '&#039;')
}

function createTaskId(title) {
  const base =
    title
      .toLowerCase()
      .trim()
      .replace(/[^a-z0-9]+/g, '-')
      .replace(/^-|-$/g, '') || `task-${Date.now()}`
  let candidate = base
  let suffix = 2

  while (state.tasks.some((task) => task.id === candidate)) {
    candidate = `${base}-${suffix}`
    suffix += 1
  }

  return candidate
}

function addTask(formData) {
  const title = formData.get('title').trim()
  if (!title) return

  const task = {
    id: createTaskId(title),
    title,
    category: formData.get('category').trim() || 'Personal',
    difficulty: Number(formData.get('difficulty')),
    cognitiveLoad: Number(formData.get('cognitiveLoad')),
    duration: Number(formData.get('duration')),
    priority: Number(formData.get('priority')),
    dependencies: formData.getAll('dependencies'),
  }

  state.tasks = [...state.tasks, task]
}

function deleteTask(taskId) {
  state.tasks = state.tasks
    .filter((task) => task.id !== taskId)
    .map((task) => ({
      ...task,
      dependencies: task.dependencies.filter((dependencyId) => dependencyId !== taskId),
    }))
}

function renderMetric(label, value, helper = '') {
  return `
    <div class="metric">
      <span>${escapeHtml(label)}</span>
      <strong>${escapeHtml(value)}</strong>
      ${helper ? `<small>${escapeHtml(helper)}</small>` : ''}
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
            <th>Action</th>
          </tr>
        </thead>
        <tbody>
          ${tasks
            .map(
              (task) => `
                <tr>
                  <td>
                    <strong>${escapeHtml(task.title)}</strong>
                    <span>${escapeHtml(task.id)}</span>
                  </td>
                  <td>${escapeHtml(task.category)}</td>
                  <td>${task.cognitiveLoad}</td>
                  <td>${task.priority}</td>
                  <td>${task.duration} min</td>
                  <td>${escapeHtml(formatDependencyList(task))}</td>
                  <td>
                    <button class="icon-button danger" type="button" data-delete-task="${escapeHtml(task.id)}" title="Delete task">
                      Delete
                    </button>
                  </td>
                </tr>
              `,
            )
            .join('')}
        </tbody>
      </table>
    </div>
  `
}

function renderTaskForm() {
  return `
    <section class="panel task-editor">
      <div class="section-heading">
        <div>
          <p class="eyebrow">Add your own task</p>
          <h2>Daily task input</h2>
        </div>
      </div>

      <form id="task-form" class="task-form">
        <label>
          Task name
          <input name="title" type="text" placeholder="Example: Call friend" required />
        </label>
        <label>
          Category
          <input name="category" type="text" placeholder="Personal" value="Personal" />
        </label>
        <label>
          Difficulty
          <input name="difficulty" type="number" min="1" max="5" value="3" />
        </label>
        <label>
          Cognitive load
          <input name="cognitiveLoad" type="number" min="1" max="10" value="4" />
        </label>
        <label>
          Duration
          <input name="duration" type="number" min="5" max="240" step="5" value="30" />
        </label>
        <label>
          Priority
          <input name="priority" type="number" min="1" max="10" value="6" />
        </label>
        <label class="dependency-field">
          Dependencies
          <select name="dependencies" multiple>
            ${state.tasks
              .map((task) => `<option value="${escapeHtml(task.id)}">${escapeHtml(task.title)}</option>`)
              .join('')}
          </select>
        </label>
        <button class="primary-button" type="submit">Add task</button>
      </form>
    </section>
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
                        <strong>${escapeHtml(task.title)}</strong>
                        <p>${escapeHtml(task.category)} | load ${task.cognitiveLoad} | priority ${task.priority}</p>
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
                      ? session.tasks.map((task) => `<li>${escapeHtml(task.title)}</li>`).join('')
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
              .map((task) => escapeHtml(task.title))
              .join(', ')}.</p>`
          : '<p class="note">All tasks fit inside the selected cognitive budget.</p>'
      }
      ${
        packed.overflow.length
          ? `<p class="note warning-note">Scheduled but not placed in a session because of session time limits: ${packed.overflow
              .map((task) => escapeHtml(task.title))
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
                <h3>${escapeHtml(result.strategy)}</h3>
                <div class="compact-metrics">
                  ${renderMetric('Tasks', result.metrics.taskCount)}
                  ${renderMetric('Load', result.metrics.cognitiveLoad)}
                  ${renderMetric('Penalty', result.metrics.switchingPenalty)}
                  ${renderMetric('Score', result.metrics.efficiency)}
                </div>
                <p>${escapeHtml(result.explanation[0] || 'Schedule generated from deterministic DAA rules.')}</p>
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
          data structures and algorithms to produce explainable everyday task plans.
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
      ${renderTaskForm()}

      <section class="panel">
        <div class="section-heading">
          <div>
            <p class="eyebrow">Input data</p>
            <h2>Daily tasks, dependencies, and cognitive cost</h2>
          </div>
          <span class="status-pill">${state.tasks.length} tasks</span>
        </div>
        ${renderTaskTable(state.tasks)}
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

  document.querySelector('#task-form').addEventListener('submit', (event) => {
    event.preventDefault()
    addTask(new FormData(event.currentTarget))
    render()
  })

  document.querySelectorAll('[data-delete-task]').forEach((button) => {
    button.addEventListener('click', () => {
      deleteTask(button.dataset.deleteTask)
      render()
    })
  })
}

render()
