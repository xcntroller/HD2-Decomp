#!/usr/bin/env python3
"""Local, read-only desktop frontend for the versioned hd2-inspect CLI."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog, ttk

from script_client import invoke, inspect_file
from mission_gui import MissionExplorer
from script_panels import ScriptPanels

KINDS = {2: "Integer", 4: "String", 5: "Frame", 8: "Whenever"}


class InspectorWindow:
    def __init__(self, root, inspector):
        self.root, self.inspector = root, inspector
        self.executor = ThreadPoolExecutor(max_workers=1)
        self.pending = None
        self.poll_timer = None
        self.explorer = None
        self.path = None
        self.protected_roots = []
        self.report = None
        self.catalog = []
        self.include_context = None
        self.source_reader = None
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
        ttk.Button(toolbar, text="Mission explorer…", command=self.open_explorer).pack(side="left", padx=(8,0))
        ttk.Button(toolbar, text="Include context…", command=self.choose_context).pack(side="left", padx=(8,0))
        self.reload_button = ttk.Button(toolbar, text="Inspect again", command=self.reload, state="disabled")
        self.reload_button.pack(side="left", padx=8)
        self.export_button = ttk.Button(toolbar, text="Export JSON…", command=self.export, state="disabled")
        self.export_button.pack(side="right")
        self.file_label = ttk.Label(shell, text="No script selected", style="Muted.TLabel", wraplength=1000)
        self.file_label.pack(anchor="w", pady=(12, 4))
        self.status = ttk.Label(shell, text="Loading command reference…", font=("TkDefaultFont", 11, "bold"))
        self.status.pack(anchor="w", pady=(0, 10))

        ttk.Label(shell, text="Read-only source analysis · explicit installation context for includes · no script execution",
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
        self.source_tab=source_tab
        ttk.Label(source_tab, text="Read-only Latin-1 source · byte spans and normalized line positions · double-click analysis rows to navigate",
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
        self.analysis=ScriptPanels(self,source_tab)
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
        self.analysis.sources.configure(state="disabled" if busy else "readonly")
        self.open_button.configure(state="disabled" if busy else "normal")
        self.reload_button.configure(state="normal" if self.path and not busy else "disabled")
        self.export_button.configure(state="normal" if self.report and not busy else "disabled")

    def submit(self, kind, function):
        self.set_busy(True)
        self.pending = (kind, self.executor.submit(function))
        self.poll_timer = self.root.after(50, self.poll)

    def poll(self):
        self.poll_timer = None
        if self.closed:
            return
        kind, future = self.pending
        if not future.done():
            self.poll_timer = self.root.after(50, self.poll)
            return
        self.pending = None
        try:
            result = future.result()
            if kind == "catalog":
                self.catalog = [(category, item) for category, key in (
                    ("Command", "commands"), ("Predicate", "predicates")) for item in result[key]]
                self.filter_catalog()
                self.status.configure(text="Ready · open a script to inspect")
            elif kind == "source":
                self.analysis.display(*result)
            elif kind == "context":
                self.include_context=result
                self.source_reader=result.source
                self.status.configure(text="Include context selected: "+result.resource+" · Inspect again to apply")
            else:
                self.show_report(*result)
        except Exception as exc:
            self.status.configure(text="Inspection unavailable")
            self.set_text(self.summary, str(exc))
            messagebox.showerror("Inspector unavailable", str(exc), parent=self.root)
        finally:
            self.set_busy(False)

    def choose_context(self):
        if self.pending:return
        selected=filedialog.askdirectory(parent=self.root,title="Installation for include resolution")
        if not selected:return
        namespace=simpledialog.askstring("Include lookup", "Mission resource namespace (for example tutorial or alps1):",parent=self.root)
        if not namespace:return
        from resource_store import ResourceStore
        from script_source import InstallationIncludes
        # Uses the established default loose/Patch/base profile. Alternate profiles
        # are selected in Mission Explorer, whose snapshots preserve their context.
        self.protected_roots.append(Path(selected).resolve())
        self.submit('context',lambda:InstallationIncludes(ResourceStore(selected),namespace))

    def open_file(self):
        if self.pending:
            return
        selected = filedialog.askopenfilename(parent=self.root, title="Open mission script",
                    filetypes=[("Mission scripts", "*.scr"), ("All files", "*")])
        if selected:
            self.source_reader=self.include_context.source if self.include_context else None
            self.path = Path(selected).resolve()
            self.protected_roots = [self.path.parent]+([self.include_context.store.root] if self.include_context else [])
            self.reload()

    def reload(self):
        if self.pending or not self.path:
            return
        self.report = None
        self.analysis.clear()
        self.file_label.configure(text=str(self.path))
        self.status.configure(text="Inspecting…")
        for tree in (self.references, self.variables, self.instructions):
            tree.delete(*tree.get_children())
        for widget in (self.source_text, self.raw, self.summary):
            self.set_text(widget, "")
        self.tabs.select(self.overview)
        self.submit("inspection", lambda: inspect_file(self.inspector, self.path,self.include_context))

    def show_report(self, source, report):
        self.report = report
        for tree in (self.references,self.variables,self.instructions):tree.delete(*tree.get_children())
        self.analysis.set_report(source,report,self.source_reader)
        accepted = report["syntax_accepted"]
        title = "Supported syntax accepted" if accepted else "Validation unavailable / source rejected"
        self.status.configure(text=title)
        summary = [title, "", report["diagnostic"] or "No syntax diagnostic.", "",
                   f"{len(report['instructions'])} instructions  ·  {len(report['variables'])} symbols  ·  "
                   f"{len(report['references'])} references",
                   f"Start PC: {report['start_pc']}  ·  Whenever records: {report['whenever_count']}",
                   f"Event slots (raw DC..F8 order): {report['event_slots']}",
                   "References / Symbols / Instructions show the first 2,000 rows; Analysis search covers all constructs.", "", "Limitations", *report["limitations"]]
        if report.get("include_directive_present"):
            summary += ["", "Preprocessing: "+report.get("preprocessing_status","unknown")]
        summary += ["", "Recognized lexical mentions (these are not confirmed calls)"]
        summary += [f"{m['category']}: {m['name']}" for m in report["lexical_mentions"][:250]]
        self.set_text(self.summary, "\n".join(summary))
        for ref in report["references"][:2000]:
            self.references.insert("", "end", values=(ref["pc"], ref["kind"],
                "<string variable>" if ref["dynamic"] else ref["literal"],
                "Dynamic" if ref["dynamic"] else ref.get("resolution_status","Literal · existence unchecked")))
        for symbol in report["variables"][:2000]:
            self.variables.insert("", "end", values=(symbol["name"], KINDS.get(symbol["kind"], symbol["kind"]), symbol["pool_index"]))
        for instruction in report["instructions"][:2000]:
            self.instructions.insert("", "end", values=(instruction["pc"], instruction["name"],
                f"0x{instruction['opcode']:X}", ", ".join(map(str, instruction["operands"]))))
        parts=[];length=0
        for chunk in json.JSONEncoder(indent=2,ensure_ascii=True).iterencode(report):
            parts.append(chunk);length+=len(chunk)
            if length>262144:break
        raw=''.join(parts)
        self.set_text(self.raw,raw[:262144]+("\n[Preview limited to 256 KiB; export contains the full report]" if length>262144 else ""))

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

    def show_snapshot(self,source,report,protected_roots=(),source_reader=None):
        self.source_reader=source_reader
        self.path=None
        self.protected_roots=list(protected_roots)
        self.file_label.configure(text=report["file"]+" · read-only resource snapshot")
        self.show_report(source,report)
        self.set_busy(False)
        self.root.lift()

    def export(self):
        if not self.report or self.pending:
            return
        selected = filedialog.asksaveasfilename(parent=self.root, title="Export inspection JSON",
                   defaultextension=".json", initialfile=Path(self.report["file"]).stem + ".inspection.json",
                   filetypes=[("JSON", "*.json")])
        if selected:
            destination = Path(selected)
            try:
                if any(destination.resolve().is_relative_to(root) for root in self.protected_roots):
                    raise OSError("Save reports outside the inspected source folders/installation")
                if self.path is not None and (destination.resolve() == self.path or (destination.exists() and destination.samefile(self.path))):
                    messagebox.showerror("Cannot overwrite source", "Choose a separate JSON file.", parent=self.root)
                    return
                with destination.open("x", encoding="utf-8") as stream:
                    stream.writelines(json.JSONEncoder(indent=2,ensure_ascii=True,allow_nan=False).iterencode(self.report))
                    stream.write("\n")
            except OSError as exc:
                messagebox.showerror("Export failed", str(exc), parent=self.root)

    def open_explorer(self):
        if self.explorer is None:
            self.explorer = MissionExplorer(self)
        else:
            self.explorer.root.lift()

    def close(self):
        self.closed = True
        if self.explorer is not None:
            self.explorer.close()
        if self.poll_timer is not None:
            self.root.after_cancel(self.poll_timer)
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
