from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
from datetime import datetime, timezone

from .models import Priority, Task, TaskStatus


@dataclass(frozen=True)
class TaskFilters:
    status: TaskStatus | None = None
    priority: Priority | None = None
    tag: str | None = None
    query: str | None = None
    due_before: datetime | None = None


class TaskBook:
    def __init__(self, tasks: list[Task] | None = None) -> None:
        self._tasks: dict[str, Task] = {task.id: task for task in tasks or []}

    def add(
        self,
        title: str,
        priority: Priority = Priority.NORMAL,
        tags: list[str] | None = None,
        notes: str = "",
        due: datetime | None = None,
    ) -> Task:
        title = title.strip()
        if not title:
            raise ValueError("title cannot be empty")
        task = Task(title=title, priority=priority, tags=sorted(set(tags or [])), notes=notes, due=due)
        self._tasks[task.id] = task
        return task

    def get(self, task_id: str) -> Task:
        try:
            return self._tasks[task_id]
        except KeyError as exc:
            raise KeyError(f"task not found: {task_id}") from exc

    def complete(self, task_id: str) -> Task:
        task = self.get(task_id)
        task.complete()
        return task

    def reopen(self, task_id: str) -> Task:
        task = self.get(task_id)
        task.reopen()
        return task

    def delete(self, task_id: str) -> Task:
        try:
            return self._tasks.pop(task_id)
        except KeyError as exc:
            raise KeyError(f"task not found: {task_id}") from exc

    def list(self, filters: TaskFilters | None = None) -> list[Task]:
        filters = filters or TaskFilters()
        tasks = list(self._tasks.values())

        if filters.status is not None:
            tasks = [task for task in tasks if task.status == filters.status]
        if filters.priority is not None:
            tasks = [task for task in tasks if task.priority == filters.priority]
        if filters.tag:
            tasks = [task for task in tasks if filters.tag in task.tags]
        if filters.query:
            query = filters.query.lower()
            tasks = [task for task in tasks if query in task.title.lower() or query in task.notes.lower()]
        if filters.due_before is not None:
            tasks = [task for task in tasks if task.due is not None and task.due <= filters.due_before]

        return sorted(
            tasks,
            key=lambda task: (
                task.status != TaskStatus.TODO,
                task.due is None,
                task.due or datetime.max.replace(tzinfo=timezone.utc),
                -task.priority.weight,
                task.created_at,
            ),
        )

    def stats(self) -> dict:
        tasks = list(self._tasks.values())
        by_status = Counter(task.status.value for task in tasks)
        by_priority = Counter(task.priority.value for task in tasks)
        return {
            "total": len(tasks),
            "todo": by_status[TaskStatus.TODO.value],
            "done": by_status[TaskStatus.DONE.value],
            "overdue": sum(1 for task in tasks if task.is_overdue),
            "by_priority": dict(sorted(by_priority.items())),
        }

    def to_list(self) -> list[dict]:
        return [task.to_dict() for task in self.list()]

    @classmethod
    def from_list(cls, rows: list[dict]) -> "TaskBook":
        return cls([Task.from_dict(row) for row in rows])
