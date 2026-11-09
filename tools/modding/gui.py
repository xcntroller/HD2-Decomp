#!/usr/bin/env python3
"""Local, read-only desktop frontend for the versioned hd2-inspect CLI."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import subprocess
import tempfile
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

KINDS = {2: "Integer", 4: "String", 5: "Frame", 8: "Whenever"}


def invoke(inspector, *args):
    """Run only the selected inspector binary; source is never executable input."""
    try:
        process = subprocess.run(
            [str(inspector), *map(str, args)], capture_output=True,
            encoding="utf-8", errors="replace", timeout=20, check=False,
        )
    except FileNotFoundError as exc:
        raise RuntimeError("Inspector not found. Build it with make -C tools/modding, "
                           "or select it using --inspector PATH.") from exc
    if process.returncode not in (0, 1):
        raise RuntimeError(process.stderr.strip() or f"Inspector exited with {process.returncode}")
    try:
        result = json.loads(process.stdout)
    except ValueError as exc:
        raise RuntimeError("Inspector returned invalid JSON.") from exc
    if not isinstance(result, dict) or result.get("schema_version") != 1:
        raise RuntimeError("Unsupported inspector JSON schema; expected version 1.")
    if args and args[0] == "--catalog":
        if process.returncode or not all(isinstance(result.get(k), list) for k in ("commands", "predicates")):
            raise RuntimeError("Invalid command catalog.")
    else:
        if (type(result.get("syntax_accepted")) is not bool
                or result["syntax_accepted"] != (process.returncode == 0)
                or not all(isinstance(result.get(k), list) for k in (
                    "variables", "instructions", "references", "lexical_mentions", "limitations", "event_slots"))):
            raise RuntimeError("Invalid inspection result.")
    return result


def inspect_file(inspector, path):
    # The preview and report describe the same bounded snapshot, even if the
    # selected file changes during inspection. Never normalize its line endings.
    with path.open("rb") as stream:
        source = stream.read(65535)
    with tempfile.TemporaryDirectory(prefix="hd2-inspection-") as directory:
        snapshot = Path(directory) / "source.scr"
        snapshot.write_bytes(source)
        report = invoke(inspector, "--json", snapshot)
    report["file"] = str(path)
    return source, report


class InspectorWindow:
    def __init__(self, root, inspector):
        self.root, self.inspector = root, inspector
        self.executor = ThreadPoolExecutor(max_workers=1)
        self.pending = None
        self.path = None
        self.report = None
        self.catalog = []
        self.closed = False
        root.title("H&D2 · Script Inspector")
        root.geometry("1180x800")
        root.minsize(850, 580)
        root.protocol("WM_DELETE_WINDOW", self.close)
        style = ttk.Style(root)
        style.theme_use("clam")
        style.configure("TFrame", background="#f4f5f2")
        style.configure("TLabel", background="#f4f5f2", foreground="#25312d")
        style.configure("Title.TLabel", font=("TkDefaultFont", 20, "bold"))
        style.configure("Muted.TLabel", foreground="#58675f")
        style.configure("TButton", padding=(12, 7))
        style.configure("Treeview", rowheight=27)
        style.configure("Treeview.Heading", padding=6)

        shell = ttk.Frame(root, padding=18)
        shell.pack(fill="both", expand=True)
        header = ttk.Frame(shell)
        header.pack(fill="x")
        ttk.Label(header, text="Script Inspector", style="Title.TLabel").pack(side="left")
        ttk.Label(header, text="HIDDEN & DANGEROUS 2  /  MODDING TOOLS", style="Muted.TLabel").pack(side="right")
        ttk.Label(shell, text="Inspect mission scripts, explore known commands and trace established references.",
                  style="Muted.TLabel").pack(anchor="w", pady=(5, 14))
        toolbar = ttk.Frame(shell)
        toolbar.pack(fill="x")
        self.open_button = ttk.Button(toolbar, text="Open script…", command=self.open_file)
        self.open_button.pack(side="left")
        self.reload_button = ttk.Button(toolbar, text="Inspect again", command=self.reload, state="disabled")
        self.reload_button.pack(side="left", padx=8)
        self.export_button = ttk.Button(toolbar, text="Export JSON…", command=self.export, state="disabled")
        self.export_button.pack(side="right")
        self.file_label = ttk.Label(shell, text="No script selected", style="Muted.TLabel", wraplength=1000)
        self.file_label.pack(anchor="w", pady=(12, 4))
        self.status = ttk.Label(shell, text="Loading command reference…", font=("TkDefaultFont", 11, "bold"))
        self.status.pack(anchor="w", pady=(0, 10))

        ttk.Label(shell, text="Prepared source only · includes are unresolved · resources are not checked · no script execution",
                  style="Muted.TLabel", wraplength=800).pack(side="bottom", anchor="w", pady=(12, 0))
        self.tabs = ttk.Notebook(shell)
        self.tabs.pack(fill="both", expand=True)
        self.overview = self.tab("Overview")
        self.summary = self.text(self.overview)
        self.summary.configure(wrap="word")
        self.set_text(self.summary, "Open a .scr file to begin.\n\nSource files are read-only. "
                      "Syntax acceptance does not establish runtime compatibility.\n\n"
                      "The Command reference tab can be searched without opening a script.")
        source_tab = self.tab("Source")
        ttk.Label(source_tab, text="Read-only byte preview · Latin-1 display · first 65,535 bytes only",
                  style="Muted.TLabel").pack(anchor="w", pady=(0, 8))
        self.source_text = self.text(source_tab)
        self.references = self.tree(self.tab("References"), ("PC", "Kind", "Reference", "Resolution"))
        self.variables = self.tree(self.tab("Symbols"), ("Name", "Kind", "Pool index"))
        instructions_tab = self.tab("Instructions")
        ttk.Label(instructions_tab, text="Operands are raw pool / program indices, not evaluated values.",
                  style="Muted.TLabel").pack(anchor="w", pady=(0, 8))
        self.instructions = self.tree(instructions_tab, ("PC", "Construct", "Opcode", "Operands"))
        reference_tab = self.tab("Command reference")
        search_row = ttk.Frame(reference_tab)
        search_row.pack(fill="x", pady=(0, 8))
        ttk.Label(search_row, text="Search commands and predicates").pack(side="left")
        self.query = tk.StringVar()
        search = ttk.Entry(search_row, textvariable=self.query)
        search.pack(side="left", fill="x", expand=True, padx=(12, 0))
        self.query.trace_add("write", lambda *_: self.filter_catalog())
        self.catalog_count = ttk.Label(reference_tab, text="", style="Muted.TLabel")
        self.catalog_count.pack(anchor="w", pady=(0, 6))
        self.command_tree = self.tree(reference_tab, ("Name", "Category", "Opcode", "Argument types"))
        ttk.Label(reference_tab, text="Table signatures only. Special constructs use dedicated grammar; "
                  "runtime semantics and actor applicability may be unknown.", wraplength=1000,
                  style="Muted.TLabel").pack(anchor="w", pady=(8, 0))
        self.raw = self.text(self.tab("JSON"))
        root.bind("<Control-o>", lambda _: self.open_file())
        self.submit("catalog", lambda: invoke(self.inspector, "--catalog"))

    def tab(self, label):
        frame = ttk.Frame(self.tabs, padding=12)
        self.tabs.add(frame, text=label)
        return frame

    @staticmethod
    def text(parent):
        box = ttk.Frame(parent)
        box.pack(fill="both", expand=True)
        widget = tk.Text(box, wrap="none", font="TkFixedFont", background="#ffffff",
                         foreground="#24322a", relief="flat", padx=12, pady=12, state="disabled")
        vertical = ttk.Scrollbar(box, orient="vertical", command=widget.yview)
        horizontal = ttk.Scrollbar(box, orient="horizontal", command=widget.xview)
        widget.configure(yscrollcommand=vertical.set, xscrollcommand=horizontal.set)
        box.rowconfigure(0, weight=1)
        box.columnconfigure(0, weight=1)
        widget.grid(row=0, column=0, sticky="nsew")
        vertical.grid(row=0, column=1, sticky="ns")
        horizontal.grid(row=1, column=0, sticky="ew")
        return widget

    @staticmethod
    def tree(parent, columns):
        box = ttk.Frame(parent)
        box.pack(fill="both", expand=True)
        tree = ttk.Treeview(box, columns=columns, show="headings", selectmode="browse")
        for column in columns:
            tree.heading(column, text=column)
            tree.column(column, width=180, minwidth=60)
        scroll = ttk.Scrollbar(box, orient="vertical", command=tree.yview)
        horizontal = ttk.Scrollbar(box, orient="horizontal", command=tree.xview)
        tree.configure(yscrollcommand=scroll.set, xscrollcommand=horizontal.set)
        box.rowconfigure(0, weight=1)
        box.columnconfigure(0, weight=1)
        tree.grid(row=0, column=0, sticky="nsew")
        scroll.grid(row=0, column=1, sticky="ns")
        horizontal.grid(row=1, column=0, sticky="ew")
        return tree

    @staticmethod
    def set_text(widget, value):
        widget.configure(state="normal")
        widget.delete("1.0", "end")
        widget.insert("1.0", value)
        widget.configure(state="disabled")

    def set_busy(self, busy):
        self.open_button.configure(state="disabled" if busy else "normal")
        self.reload_button.configure(state="normal" if self.path and not busy else "disabled")
        self.export_button.configure(state="normal" if self.report and not busy else "disabled")

    def submit(self, kind, function):
        self.set_busy(True)
        self.pending = (kind, self.executor.submit(function))
        self.root.after(50, self.poll)

    def poll(self):
        if self.closed:
            return
        kind, future = self.pending
        if not future.done():
            self.root.after(50, self.poll)
            return
        self.pending = None
        try:
            result = future.result()
            if kind == "catalog":
                self.catalog = [(category, item) for category, key in (
                    ("Command", "commands"), ("Predicate", "predicates")) for item in result[key]]
                self.filter_catalog()
                self.status.configure(text="Ready · open a script to inspect")
            else:
                self.show_report(*result)
        except Exception as exc:
            self.status.configure(text="Inspection unavailable")
            self.set_text(self.summary, str(exc))
            messagebox.showerror("Inspector unavailable", str(exc), parent=self.root)
        finally:
            self.set_busy(False)

    def open_file(self):
        if self.pending:
            return
        selected = filedialog.askopenfilename(parent=self.root, title="Open mission script",
                    filetypes=[("Mission scripts", "*.scr"), ("All files", "*")])
        if selected:
            self.path = Path(selected).resolve()
            self.reload()

    def reload(self):
        if self.pending or not self.path:
            return
        self.report = None
        self.file_label.configure(text=str(self.path))
        self.status.configure(text="Inspecting…")
        for tree in (self.references, self.variables, self.instructions):
            tree.delete(*tree.get_children())
        for widget in (self.source_text, self.raw, self.summary):
            self.set_text(widget, "")
        self.tabs.select(self.overview)
        self.submit("inspection", lambda: inspect_file(self.inspector, self.path))

    def show_report(self, source, report):
        self.report = report
        accepted = report["syntax_accepted"]
        title = "Supported syntax accepted" if accepted else "Validation unavailable / source rejected"
        self.status.configure(text=title)
        summary = [title, "", report["diagnostic"] or "No syntax diagnostic.", "",
                   f"{len(report['instructions'])} instructions  ·  {len(report['variables'])} symbols  ·  "
                   f"{len(report['references'])} references",
                   f"Start PC: {report['start_pc']}  ·  Whenever records: {report['whenever_count']}",
                   f"Event slots (raw DC..F8 order): {report['event_slots']}", "", "Limitations", *report["limitations"]]
        if report.get("include_directive_present"):
            summary += ["", "An #include occurrence was detected. Expansion and dependency resolution are unavailable."]
        summary += ["", "Recognized lexical mentions (these are not confirmed calls)"]
        summary += [f"{m['category']}: {m['name']}" for m in report["lexical_mentions"]]
        self.set_text(self.summary, "\n".join(summary))
        # Render control bytes visibly; retain tabs and line breaks for reading.
        preview = "".join(c if ord(c) >= 32 or c in "\t\r\n" else f"\\x{ord(c):02x}"
                          for c in source.decode("latin-1"))
        self.set_text(self.source_text, preview.replace("\r\n", "\n").replace("\r", "\n"))
        for ref in report["references"]:
            self.references.insert("", "end", values=(ref["pc"], ref["kind"],
                "<string variable>" if ref["dynamic"] else ref["literal"],
                "Dynamic" if ref["dynamic"] else "Literal · existence unchecked"))
        for symbol in report["variables"]:
            self.variables.insert("", "end", values=(symbol["name"], KINDS.get(symbol["kind"], symbol["kind"]), symbol["pool_index"]))
        for instruction in report["instructions"]:
            self.instructions.insert("", "end", values=(instruction["pc"], instruction["name"],
                f"0x{instruction['opcode']:X}", ", ".join(map(str, instruction["operands"]))))
        self.set_text(self.raw, json.dumps(report, indent=2, ensure_ascii=True))

    def filter_catalog(self):
        query = self.query.get().strip().casefold()
        self.command_tree.delete(*self.command_tree.get_children())
        count = 0
        for category, item in self.catalog:
            if query not in item["name"].casefold():
                continue
            kinds = item["argument_kinds"]
            signature = "Special grammar / no table signature" if kinds is None else ", ".join(
                KINDS.get(kind, f"kind {kind}") for kind in kinds) or "No ordinary arguments"
            self.command_tree.insert("", "end", values=(item["name"], category, f"0x{item['opcode']:X}", signature))
            count += 1
        self.catalog_count.configure(text=f"{count} of {len(self.catalog)} table entries")

    def export(self):
        if not self.report or self.pending:
            return
        selected = filedialog.asksaveasfilename(parent=self.root, title="Export inspection JSON",
                   defaultextension=".json", initialfile=self.path.stem + ".inspection.json",
                   filetypes=[("JSON", "*.json")])
        if selected:
            destination = Path(selected)
            try:
                if destination.resolve() == self.path or (destination.exists() and destination.samefile(self.path)):
                    messagebox.showerror("Cannot overwrite source", "Choose a separate JSON file.", parent=self.root)
                    return
                destination.write_text(json.dumps(self.report, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")
            except OSError as exc:
                messagebox.showerror("Export failed", str(exc), parent=self.root)

    def close(self):
        self.closed = True
        self.executor.shutdown(wait=False, cancel_futures=True)
        self.root.destroy()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inspector", type=Path, default=Path("/tmp/hd2-inspect"), help="Path to the compiled inspector")
    args = parser.parse_args()
    root = tk.Tk()
    InspectorWindow(root, args.inspector.resolve())
    root.mainloop()


if __name__ == "__main__":
    main()
