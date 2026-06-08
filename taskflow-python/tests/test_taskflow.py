from __future__ import annotations

import tempfile
import unittest
from datetime import datetime, timezone
from pathlib import Path

from taskflow.models import Priority, TaskStatus
from taskflow.planner import TaskBook, TaskFilters
from taskflow.storage import JsonTaskStore


class TaskBookTest(unittest.TestCase):
    def test_add_and_complete_task(self) -> None:
        book = TaskBook()
        task = book.add("write docs", priority=Priority.HIGH, tags=["docs", "github"])

        self.assertEqual(task.status, TaskStatus.TODO)
        completed = book.complete(task.id)

        self.assertEqual(completed.status, TaskStatus.DONE)
        self.assertIsNotNone(completed.completed_at)

    def test_filters_by_tag_and_priority(self) -> None:
        book = TaskBook()
        book.add("fix server", priority=Priority.URGENT, tags=["server"])
        book.add("write readme", priority=Priority.LOW, tags=["docs"])

        result = book.list(TaskFilters(priority=Priority.URGENT, tag="server"))

        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].title, "fix server")

    def test_stats_counts_overdue_tasks(self) -> None:
        book = TaskBook()
        book.add("old task", due=datetime(2020, 1, 1, tzinfo=timezone.utc))
        book.add("done task").complete()

        stats = book.stats()

        self.assertEqual(stats["total"], 2)
        self.assertEqual(stats["todo"], 1)
        self.assertEqual(stats["done"], 1)
        self.assertEqual(stats["overdue"], 1)

    def test_json_store_roundtrip(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "tasks.json"
            store = JsonTaskStore(path)
            book = TaskBook()
            created = book.add("persist me", priority=Priority.NORMAL, tags=["disk"])
            store.save(book)

            loaded = store.load()

            self.assertEqual(loaded.get(created.id).title, "persist me")
            self.assertEqual(loaded.get(created.id).tags, ["disk"])


if __name__ == "__main__":
    unittest.main()
