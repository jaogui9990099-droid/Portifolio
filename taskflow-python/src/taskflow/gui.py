from __future__ import annotations

import os
import tkinter as tk
from pathlib import Path
from tkinter import messagebox, ttk

from .models import Priority, TaskStatus, parse_datetime
from .planner import TaskBook, TaskFilters
from .storage import JsonTaskStore


class TaskflowApp(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("Taskflow")
        self.geometry("980x620")
        self.minsize(860, 520)
        self.configure(bg="#111318")

        store_path = os.environ.get("TASKFLOW_STORE") or str(Path.home() / ".taskflow" / "tasks.json")
        self.store = JsonTaskStore(store_path)
        self.book = self.store.load()

        self.title_var = tk.StringVar()
        self.priority_var = tk.StringVar(value=Priority.NORMAL.value)
        self.tags_var = tk.StringVar()
        self.due_var = tk.StringVar()
        self.notes_var = tk.StringVar()
        self.status_filter = tk.StringVar(value="all")
        self.priority_filter = tk.StringVar(value="all")
        self.search_var = tk.StringVar()
        self.selected_id: str | None = None

        self._style()
        self._build()
        self.refresh()

    def _style(self) -> None:
        style = ttk.Style(self)
        style.theme_use("clam")
        style.configure("TFrame", background="#111318")
        style.configure("Panel.TFrame", background="#181b22")
        style.configure("TLabel", background="#111318", foreground="#e8edf2", font=("Segoe UI", 10))
        style.configure("Muted.TLabel", background="#181b22", foreground="#99a3ad", font=("Segoe UI", 9))
        style.configure("Title.TLabel", background="#111318", foreground="#f5f7fb", font=("Segoe UI Semibold", 18))
        style.configure("TButton", font=("Segoe UI Semibold", 10), padding=(10, 7))
        style.configure("Accent.TButton", background="#4f8cff", foreground="#ffffff")
        style.map("Accent.TButton", background=[("active", "#6aa0ff")])
        style.configure("TEntry", fieldbackground="#232832", foreground="#f5f7fb", insertcolor="#f5f7fb")
        style.configure("TCombobox", fieldbackground="#232832", foreground="#f5f7fb")
        style.configure("Treeview", background="#151922", foreground="#e8edf2", fieldbackground="#151922", rowheight=30)
        style.configure("Treeview.Heading", background="#222733", foreground="#f5f7fb", font=("Segoe UI Semibold", 10))

    def _build(self) -> None:
        header = ttk.Frame(self)
        header.pack(fill="x", padx=22, pady=(18, 8))
        ttk.Label(header, text="Taskflow", style="Title.TLabel").pack(side="left")
        ttk.Label(header, text="local task automation dashboard", foreground="#99a3ad").pack(side="left", padx=(12, 0), pady=(8, 0))

        body = ttk.Frame(self)
        body.pack(fill="both", expand=True, padx=22, pady=14)

        left = ttk.Frame(body, style="Panel.TFrame")
        left.pack(side="left", fill="y", padx=(0, 14))
        left.configure(width=310)
        left.pack_propagate(False)

        form = ttk.Frame(left, style="Panel.TFrame")
        form.pack(fill="both", padx=16, pady=16)

        self._field(form, "Task", self.title_var)
        ttk.Label(form, text="Priority", style="Muted.TLabel").pack(anchor="w", pady=(10, 3))
        ttk.Combobox(form, textvariable=self.priority_var, values=[p.value for p in Priority], state="readonly").pack(fill="x")
        self._field(form, "Tags, comma separated", self.tags_var)
        self._field(form, "Due date, YYYY-MM-DD", self.due_var)
        self._field(form, "Notes", self.notes_var)

        ttk.Button(form, text="Add Task", style="Accent.TButton", command=self.add_task).pack(fill="x", pady=(14, 6))
        ttk.Button(form, text="Complete Selected", command=self.complete_selected).pack(fill="x", pady=3)
        ttk.Button(form, text="Reopen Selected", command=self.reopen_selected).pack(fill="x", pady=3)
        ttk.Button(form, text="Delete Selected", command=self.delete_selected).pack(fill="x", pady=3)

        self.stats_label = ttk.Label(form, text="", style="Muted.TLabel", wraplength=260)
        self.stats_label.pack(fill="x", pady=(16, 0))

        right = ttk.Frame(body)
        right.pack(side="left", fill="both", expand=True)

        filters = ttk.Frame(right)
        filters.pack(fill="x", pady=(0, 10))
        ttk.Label(filters, text="Status").pack(side="left")
        ttk.Combobox(filters, textvariable=self.status_filter, values=["all", "todo", "done"], state="readonly", width=10).pack(side="left", padx=(6, 14))
        ttk.Label(filters, text="Priority").pack(side="left")
        ttk.Combobox(filters, textvariable=self.priority_filter, values=["all", "low", "normal", "high", "urgent"], state="readonly", width=10).pack(side="left", padx=(6, 14))
        ttk.Label(filters, text="Search").pack(side="left")
        search = ttk.Entry(filters, textvariable=self.search_var)
        search.pack(side="left", fill="x", expand=True, padx=(6, 10))
        ttk.Button(filters, text="Refresh", command=self.refresh).pack(side="left")

        self.status_filter.trace_add("write", lambda *_: self.refresh())
        self.priority_filter.trace_add("write", lambda *_: self.refresh())
        self.search_var.trace_add("write", lambda *_: self.refresh())

        columns = ("status", "priority", "due", "tags", "title")
        self.table = ttk.Treeview(right, columns=columns, show="headings")
        for col, text, width in [
            ("status", "Status", 78),
            ("priority", "Priority", 86),
            ("due", "Due", 105),
            ("tags", "Tags", 160),
            ("title", "Task", 360),
        ]:
            self.table.heading(col, text=text)
            self.table.column(col, width=width, anchor="w")
        self.table.pack(fill="both", expand=True)
        self.table.bind("<<TreeviewSelect>>", self.on_select)

    def _field(self, parent: ttk.Frame, label: str, var: tk.StringVar) -> None:
        ttk.Label(parent, text=label, style="Muted.TLabel").pack(anchor="w", pady=(10, 3))
        ttk.Entry(parent, textvariable=var).pack(fill="x")

    def add_task(self) -> None:
        try:
            tags = [tag.strip() for tag in self.tags_var.get().split(",") if tag.strip()]
            self.book.add(
                self.title_var.get(),
                priority=Priority(self.priority_var.get()),
                tags=tags,
                notes=self.notes_var.get(),
                due=parse_datetime(self.due_var.get() or None),
            )
            self.store.save(self.book)
        except Exception as exc:
            messagebox.showerror("Taskflow", str(exc))
            return
        self.title_var.set("")
        self.tags_var.set("")
        self.due_var.set("")
        self.notes_var.set("")
        self.refresh()

    def _selected_task_id(self) -> str | None:
        selection = self.table.selection()
        if not selection:
            return None
        return selection[0]

    def complete_selected(self) -> None:
        task_id = self._selected_task_id()
        if not task_id:
            return
        self.book.complete(task_id)
        self.store.save(self.book)
        self.refresh()

    def reopen_selected(self) -> None:
        task_id = self._selected_task_id()
        if not task_id:
            return
        self.book.reopen(task_id)
        self.store.save(self.book)
        self.refresh()

    def delete_selected(self) -> None:
        task_id = self._selected_task_id()
        if not task_id:
            return
        self.book.delete(task_id)
        self.store.save(self.book)
        self.refresh()

    def on_select(self, _event) -> None:
        self.selected_id = self._selected_task_id()

    def refresh(self) -> None:
        for row in self.table.get_children():
            self.table.delete(row)
        filters = TaskFilters(
            status=None if self.status_filter.get() == "all" else TaskStatus(self.status_filter.get()),
            priority=None if self.priority_filter.get() == "all" else Priority(self.priority_filter.get()),
            query=self.search_var.get() or None,
        )
        for task in self.book.list(filters):
            due = task.due.date().isoformat() if task.due else "-"
            tags = ", ".join(task.tags) if task.tags else "-"
            self.table.insert("", "end", iid=task.id, values=(task.status.value, task.priority.value, due, tags, task.title))
        stats = self.book.stats()
        self.stats_label.configure(text=f"Total {stats['total']} | Todo {stats['todo']} | Done {stats['done']} | Overdue {stats['overdue']}")


def main() -> None:
    TaskflowApp().mainloop()


if __name__ == "__main__":
    main()
