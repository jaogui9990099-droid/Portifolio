from __future__ import annotations

import json
from pathlib import Path

from .planner import TaskBook


class JsonTaskStore:
    def __init__(self, path: str | Path) -> None:
        self.path = Path(path)

    def load(self) -> TaskBook:
        if not self.path.exists():
            return TaskBook()
        with self.path.open("r", encoding="utf-8") as file:
            data = json.load(file)
        if not isinstance(data, list):
            raise ValueError("task store must contain a JSON list")
        return TaskBook.from_list(data)

    def save(self, book: TaskBook) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        tmp = self.path.with_suffix(self.path.suffix + ".tmp")
        with tmp.open("w", encoding="utf-8") as file:
            json.dump(book.to_list(), file, indent=2)
            file.write("\n")
        tmp.replace(self.path)
