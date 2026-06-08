# Taskflow

Taskflow is a small Python task automation tool with a command-line interface and JSON persistence.

It is designed as a practical productivity/backend exercise: tasks are stored locally, can be filtered by status, priority, tag and due date, and the code is split between domain logic, storage and CLI handling.

## Features

- Add tasks with priority, tags, notes and due dates.
- Mark tasks as completed.
- List tasks with useful filters.
- Delete tasks.
- Generate simple productivity stats.
- Store data in a readable JSON file.
- Unit-tested domain behavior.

## Run

From this folder:

```bash
python -m taskflow.gui
```

The graphical app uses Tkinter and stores tasks in `~/.taskflow/tasks.json`.

CLI usage:

```bash
python -m taskflow.cli add "Ship portfolio update" --priority high --tag github --due 2026-06-10
python -m taskflow.cli list
python -m taskflow.cli stats
```

For local development without installing the package:

```bash
set PYTHONPATH=src
python -m taskflow.cli list
```

On Linux/macOS:

```bash
PYTHONPATH=src python -m taskflow.cli list
```

## Test

```bash
set PYTHONPATH=src
python -m unittest discover -s tests
```

## Code Map

- `src/taskflow/models.py`: task model and serialization.
- `src/taskflow/planner.py`: task operations and filters.
- `src/taskflow/storage.py`: JSON storage.
- `src/taskflow/cli.py`: command-line interface.
- `tests`: unit tests for the task planner.
