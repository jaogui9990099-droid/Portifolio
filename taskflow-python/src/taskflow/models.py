from __future__ import annotations

from dataclasses import dataclass, field
from datetime import datetime, timezone
from enum import Enum
from uuid import uuid4


def utc_now() -> datetime:
    return datetime.now(timezone.utc)


def parse_datetime(value: str | None) -> datetime | None:
    if not value:
        return None
    if len(value) == 10:
        value = value + "T23:59:59+00:00"
    if value.endswith("Z"):
        value = value[:-1] + "+00:00"
    parsed = datetime.fromisoformat(value)
    if parsed.tzinfo is None:
        parsed = parsed.replace(tzinfo=timezone.utc)
    return parsed.astimezone(timezone.utc)


def format_datetime(value: datetime | None) -> str | None:
    if value is None:
        return None
    return value.astimezone(timezone.utc).isoformat().replace("+00:00", "Z")


class TaskStatus(str, Enum):
    TODO = "todo"
    DONE = "done"


class Priority(str, Enum):
    LOW = "low"
    NORMAL = "normal"
    HIGH = "high"
    URGENT = "urgent"

    @property
    def weight(self) -> int:
        return {
            Priority.LOW: 1,
            Priority.NORMAL: 2,
            Priority.HIGH: 3,
            Priority.URGENT: 4,
        }[self]


@dataclass(slots=True)
class Task:
    title: str
    priority: Priority = Priority.NORMAL
    tags: list[str] = field(default_factory=list)
    notes: str = ""
    due: datetime | None = None
    status: TaskStatus = TaskStatus.TODO
    id: str = field(default_factory=lambda: uuid4().hex[:12])
    created_at: datetime = field(default_factory=utc_now)
    completed_at: datetime | None = None

    def complete(self, when: datetime | None = None) -> None:
        self.status = TaskStatus.DONE
        self.completed_at = when or utc_now()

    def reopen(self) -> None:
        self.status = TaskStatus.TODO
        self.completed_at = None

    @property
    def is_overdue(self) -> bool:
        return self.status == TaskStatus.TODO and self.due is not None and self.due < utc_now()

    def to_dict(self) -> dict:
        return {
            "id": self.id,
            "title": self.title,
            "priority": self.priority.value,
            "tags": self.tags,
            "notes": self.notes,
            "due": format_datetime(self.due),
            "status": self.status.value,
            "created_at": format_datetime(self.created_at),
            "completed_at": format_datetime(self.completed_at),
        }

    @classmethod
    def from_dict(cls, data: dict) -> "Task":
        return cls(
            id=str(data["id"]),
            title=str(data["title"]),
            priority=Priority(data.get("priority", Priority.NORMAL.value)),
            tags=list(data.get("tags", [])),
            notes=str(data.get("notes", "")),
            due=parse_datetime(data.get("due")),
            status=TaskStatus(data.get("status", TaskStatus.TODO.value)),
            created_at=parse_datetime(data.get("created_at")) or utc_now(),
            completed_at=parse_datetime(data.get("completed_at")),
        )
