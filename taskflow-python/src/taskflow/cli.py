from __future__ import annotations

import argparse
import os
from pathlib import Path

from .models import Priority, TaskStatus, parse_datetime
from .planner import TaskBook, TaskFilters
from .storage import JsonTaskStore


def default_store_path() -> Path:
    configured = os.environ.get("TASKFLOW_STORE")
    if configured:
        return Path(configured)
    return Path.home() / ".taskflow" / "tasks.json"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="taskflow", description="Local task automation CLI")
    parser.add_argument("--store", default=str(default_store_path()), help="path to the JSON task store")
    sub = parser.add_subparsers(dest="command", required=True)

    add = sub.add_parser("add", help="add a new task")
    add.add_argument("title")
    add.add_argument("--priority", choices=[p.value for p in Priority], default=Priority.NORMAL.value)
    add.add_argument("--tag", action="append", default=[])
    add.add_argument("--notes", default="")
    add.add_argument("--due")

    list_cmd = sub.add_parser("list", help="list tasks")
    list_cmd.add_argument("--status", choices=[s.value for s in TaskStatus])
    list_cmd.add_argument("--priority", choices=[p.value for p in Priority])
    list_cmd.add_argument("--tag")
    list_cmd.add_argument("--query")
    list_cmd.add_argument("--due-before")

    done = sub.add_parser("done", help="complete a task")
    done.add_argument("id")

    reopen = sub.add_parser("reopen", help="reopen a completed task")
    reopen.add_argument("id")

    delete = sub.add_parser("delete", help="delete a task")
    delete.add_argument("id")

    sub.add_parser("stats", help="show task stats")
    return parser


def load_book(path: str) -> tuple[JsonTaskStore, TaskBook]:
    store = JsonTaskStore(path)
    return store, store.load()


def print_task(task) -> None:
    due = task.due.date().isoformat() if task.due else "-"
    tags = ",".join(task.tags) if task.tags else "-"
    print(f"{task.id}  {task.status.value:4}  {task.priority.value:6}  due={due:10}  tags={tags:18}  {task.title}")


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    store, book = load_book(args.store)

    try:
        if args.command == "add":
            task = book.add(
                args.title,
                priority=Priority(args.priority),
                tags=args.tag,
                notes=args.notes,
                due=parse_datetime(args.due),
            )
            store.save(book)
            print_task(task)
            return 0

        if args.command == "list":
            filters = TaskFilters(
                status=TaskStatus(args.status) if args.status else None,
                priority=Priority(args.priority) if args.priority else None,
                tag=args.tag,
                query=args.query,
                due_before=parse_datetime(args.due_before),
            )
            for task in book.list(filters):
                print_task(task)
            return 0

        if args.command == "done":
            task = book.complete(args.id)
            store.save(book)
            print_task(task)
            return 0

        if args.command == "reopen":
            task = book.reopen(args.id)
            store.save(book)
            print_task(task)
            return 0

        if args.command == "delete":
            task = book.delete(args.id)
            store.save(book)
            print(f"deleted {task.id}: {task.title}")
            return 0

        if args.command == "stats":
            stats = book.stats()
            print(f"total={stats['total']} todo={stats['todo']} done={stats['done']} overdue={stats['overdue']}")
            for priority, count in stats["by_priority"].items():
                print(f"{priority}={count}")
            return 0

        parser.error("unknown command")
        return 2
    except (KeyError, ValueError) as exc:
        parser.exit(1, f"error: {exc}\n")


if __name__ == "__main__":
    raise SystemExit(main())
