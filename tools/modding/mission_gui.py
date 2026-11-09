"""Mission explorer widgets. Parsing and models live in mission.py."""
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

from mission import MissionSession, walk_records


class MissionExplorer:
    def __init__(self, owner):
        self.owner = owner
        self.root = tk.Toplevel(owner.root)
        self.root.title("H&D2 · Mission Explorer")
        self.root.geometry("1280x850")
        self.root.minsize(900,650)
        self.root.protocol("WM_DELETE_WINDOW", self.close)
        self.executor = ThreadPoolExecutor(max_workers=1)
        self.pending = None
        self.poll_timer = None
        self.session = None
        self.folder = None
        self.reference_roots = []
        self.current = None
        self.target = None
        self.closed = False
        self.nodes = {}
        shell = ttk.Frame(self.root,padding=16)
        shell.pack(fill="both",expand=True)
        ttk.Label(shell,text="Mission Explorer",style="Title.TLabel").pack(anchor="w")
        ttk.Label(shell,text="Read-only files and documented records · double-click a reference to follow a local candidate",
                  style="Muted.TLabel").pack(anchor="w",pady=(4,12))
        bar=ttk.Frame(shell);bar.pack(fill="x")
        self.open_button=ttk.Button(bar,text="Open mission folder…",command=self.open_folder)
        self.open_button.pack(side="left")
        self.add_button=ttk.Button(bar,text="Add reference folder…",command=self.add_folder,state="disabled")
        self.add_button.pack(side="left",padx=8)
        self.reload_button=ttk.Button(bar,text="Refresh",command=self.refresh,state="disabled")
        self.reload_button.pack(side="left")
        self.script_button=ttk.Button(bar,text="View script",command=self.view_script,state="disabled")
        self.script_button.pack(side="left",padx=8)
        self.export_button=ttk.Button(bar,text="Export mission JSON…",command=self.export,state="disabled")
        self.export_button.pack(side="right")
        self.status=ttk.Label(shell,text="Choose a mission directory; optional reference folders supply loose scripts/assets.",wraplength=1100)
        self.status.pack(anchor="w",pady=10)
        ttk.Label(shell,text="Local matches are candidates, not runtime resolution. Unknown payloads remain opaque. No files are edited.",
                  style="Muted.TLabel",wraplength=1000).pack(side="bottom",anchor="w",pady=(10,0))
        panes=ttk.Panedwindow(shell,orient="horizontal");panes.pack(fill="both",expand=True)
        left=ttk.Frame(panes);panes.add(left,weight=1)
        ttk.Label(left,text="FILES",style="Muted.TLabel").pack(anchor="w",pady=(0,6))
        self.files=owner.tree(left,("File","Status"))
        self.files.column("File",width=260);self.files.column("Status",width=150)
        self.files.bind("<<TreeviewSelect>>",self.select_file)
        right=ttk.Frame(panes);panes.add(right,weight=3)
        tabs=ttk.Notebook(right);tabs.pack(fill="both",expand=True)
        record_tab=ttk.Frame(tabs,padding=8);tabs.add(record_tab,text="Records")
        vertical=ttk.Panedwindow(record_tab,orient="vertical");vertical.pack(fill="both",expand=True)
        tree_box=ttk.Frame(vertical);vertical.add(tree_box,weight=2)
        self.records=ttk.Treeview(tree_box,columns=("offset","kind"),show="tree headings")
        self.records.heading("#0",text="Documented records / opaque chunks")
        self.records.heading("offset",text="File offset")
        self.records.heading("kind",text="Record ID")
        self.records.column("#0",width=350);self.records.column("offset",width=110);self.records.column("kind",width=100)
        scroll=ttk.Scrollbar(tree_box,orient="vertical",command=self.records.yview)
        self.records.configure(yscrollcommand=scroll.set)
        scroll.pack(side="right",fill="y");self.records.pack(fill="both",expand=True)
        self.records.bind("<<TreeviewOpen>>",self.expand)
        self.records.bind("<<TreeviewSelect>>",self.select_record)
        detail_box=ttk.Frame(vertical);vertical.add(detail_box,weight=1)
        self.detail=owner.text(detail_box)
        self.detail.configure(wrap="word",height=10)
        ref_tab=ttk.Frame(tabs,padding=8);tabs.add(ref_tab,text="References")
        self.ref_tree=owner.tree(ref_tab,("Kind","Value","Status","Target"))
        self.ref_tree.bind("<<TreeviewSelect>>",self.select_reference)
        self.ref_tree.bind("<Double-1>",lambda _:self.follow())
        ttk.Button(ref_tab,text="Show all file references",command=self.show_all_refs).pack(anchor="w",pady=(8,0))
        self.follow_button=ttk.Button(ref_tab,text="Follow selected candidate",command=self.follow,state="disabled")
        self.follow_button.pack(anchor="w",pady=(8,0))
        self.targets={}
        self.info=owner.text(self._tab(tabs,"Coverage / limitations"))
        owner.set_text(self.info,"Open a mission folder to inspect coverage. The tool does not mount archives or load a world.")

    @staticmethod
    def _tab(tabs,title):
        frame=ttk.Frame(tabs,padding=8);tabs.add(frame,text=title);return frame

    def busy(self,value):
        self.open_button.configure(state="disabled" if value else "normal")
        for button in (self.add_button,self.reload_button,self.export_button):
            button.configure(state="normal" if self.session and not value else "disabled")
        self.follow_button.configure(state="disabled")
        self.script_button.configure(state="normal" if not value and self.current and self.current.format=="script" else "disabled")

    def submit(self,kind,work):
        if self.pending:return
        self.busy(True)
        self.status.configure(text="Reading selected local files…")
        self.pending=(kind,self.executor.submit(work))
        self.poll_timer=self.root.after(50,self.poll)

    def poll(self):
        self.poll_timer=None
        if self.closed:return
        kind,future=self.pending
        if not future.done():self.poll_timer=self.root.after(50,self.poll);return
        self.pending=None
        try:
            result=future.result()
            if kind=="scan":
                self.session=result
                self.files.delete(*self.files.get_children())
                for entry in self.session.report.files:
                    self.files.insert("","end",iid=entry.path,values=(entry.path,entry.status))
                self.status.configure(text=f"{self.folder} · {len(self.session.report.files)} local files · {len(self.reference_roots)} reference folders")
                self.coverage()
            else:
                entry,target=result
                self.render_file(entry,target)
                self.files.item(entry.path,values=(entry.path,entry.status))
                self.coverage()
        except Exception as exc:
            self.status.configure(text="Inspection unavailable")
            messagebox.showerror("Mission inspection",str(exc),parent=self.root)
        finally:self.busy(False)

    def open_folder(self):
        if self.pending:return
        selected=filedialog.askdirectory(parent=self.root,title="Open local mission directory")
        if selected:
            self.folder=Path(selected).resolve();self.reference_roots=[];self.refresh()

    def add_folder(self):
        if self.pending or not self.folder:return
        selected=filedialog.askdirectory(parent=self.root,title="Add loose script or asset reference folder")
        if selected:
            path=Path(selected).resolve()
            if path not in [self.folder,*self.reference_roots]:self.reference_roots.append(path)
            self.refresh()

    def refresh(self):
        if self.pending or not self.folder:return
        self.current=None;self.session=None
        self.files.delete(*self.files.get_children());self.records.delete(*self.records.get_children())
        self.ref_tree.delete(*self.ref_tree.get_children());self.targets={};self.nodes={}
        self.owner.set_text(self.detail,"");self.owner.set_text(self.info,"")
        self.submit("scan",lambda:MissionSession(self.folder,self.reference_roots,self.owner.inspector))

    def coverage(self):
        report=self.session.report
        summary={"mission":report.mission_name,"files":len(report.files),
                 "recognized_files":[{"path":f.path,"format":f.format,"status":f.status,
                     "records":sum(1 for _ in walk_records(f.records)),"references":len(f.references),
                     "diagnostics":f.diagnostics} for f in report.files if f.format!="opaque"],
                 "diagnostics":report.diagnostics,"limitations":report.limitations}
        self.owner.set_text(self.info,json.dumps(summary,indent=2,ensure_ascii=True))

    def select_file(self,_=None):
        if self.pending or not self.session:return
        selected=self.files.selection()
        if not selected or self.current and selected[0]==self.current.path:return
        self.load_file(selected[0])

    def load_file(self,identity,target=None):
        if self.pending:return
        self.current=None
        self.records.delete(*self.records.get_children());self.nodes={}
        self.ref_tree.delete(*self.ref_tree.get_children());self.targets={}
        self.owner.set_text(self.detail,"")
        self.submit("file",lambda:(self.session.inspect(identity),target))

    def insert_node(self,node,parent=""):
        self.nodes[node.identity]=node
        self.records.insert(parent,"end",iid=node.identity,text=node.label,
                            values=(f"0x{node.offset:x}",node.kind))
        if node.children:self.records.insert(node.identity,"end",text="…")

    def expand_node(self,identity):
        children=self.records.get_children(identity)
        if children and children[0] not in self.nodes:
            self.records.delete(*children)
            for child in self.nodes[identity].children:self.insert_node(child,identity)

    def expand(self,_=None):
        identity=self.records.focus()
        if identity in self.nodes:self.expand_node(identity)

    def select_record(self,_=None):
        selected=self.records.selection()
        if selected and selected[0] in self.nodes:
            node=self.nodes[selected[0]]
            # Show only this node's data, not a huge nested subtree.
            value={k:getattr(node,k) for k in ("identity","offset","size","kind","label","evidence","values")}
            value["child_count"]=len(node.children)
            self.owner.set_text(self.detail,json.dumps(value,indent=2,ensure_ascii=True))
            self.populate_refs(node.identity)

    def show_all_refs(self):
        if self.current and not self.pending:self.populate_refs()

    def view_script(self):
        if self.pending or not self.current or self.current.format!="script" or self.owner.pending:return
        self.owner.path=self.session.paths[self.current.path]
        self.owner.reload()
        self.owner.root.lift()

    def populate_refs(self,identity=None):
        self.ref_tree.delete(*self.ref_tree.get_children());self.targets={}
        self.follow_button.configure(state="disabled")
        scope={n.identity for n in walk_records([self.nodes[identity]] if identity else self.current.records)}
        if identity:scope.add(identity)
        for ref in self.current.references:
            if identity and ref.source not in scope:continue
            for target in ref.targets or [None]:
                iid=self.ref_tree.insert("","end",values=(ref.kind,ref.value if ref.value is not None else "<dynamic>",ref.status,target or "—"))
                if target:self.targets[iid]=target
        for entry in self.session.report.files:
            for ref in entry.references:
                if any(target in scope for target in ref.targets):
                    iid=self.ref_tree.insert("","end",values=("incoming "+ref.kind,ref.value,ref.status,ref.source))
                    self.targets[iid]=ref.source

    def render_file(self,entry,target=None):
        self.current=entry
        self.files.selection_set(entry.path)
        self.files.see(entry.path)
        for node in entry.records:self.insert_node(node)
        summary={"path":entry.path,"format":entry.format,"status":entry.status,"size":entry.size,
                 "sha256":entry.sha256,"diagnostics":entry.diagnostics,"script":entry.script}
        self.owner.set_text(self.detail,json.dumps(summary,indent=2,ensure_ascii=True))
        self.populate_refs()
        self.status.configure(text=f"{entry.path} · {entry.status} · {len(entry.references)} references")
        if target and target!=entry.path:
            def reveal(nodes,chain):
                for node in nodes:
                    if node.identity==target:return chain+[node]
                    found=reveal(node.children,chain+[node])
                    if found:return found
                return None
            chain=reveal(entry.records,[])
            if chain:
                for node in chain[:-1]:
                    self.expand_node(node.identity);self.records.item(node.identity,open=True)
                self.records.selection_set(target);self.records.focus(target);self.records.see(target)
                self.select_record()

    def select_reference(self,_=None):
        selection=self.ref_tree.selection()
        self.follow_button.configure(state="normal" if not self.pending and selection and selection[0] in self.targets else "disabled")

    def follow(self):
        if self.pending or not self.session:return
        selection=self.ref_tree.selection()
        if not selection or selection[0] not in self.targets:return
        target=self.targets[selection[0]]
        identity=target if target in self.session.paths else target.rsplit("@0x",1)[0].split("#pc=",1)[0]
        if identity in self.session.paths:
            # Do not generate a second load from the file selection event.
            self.files.selection_set(identity);self.files.see(identity)
            self.load_file(identity,target)

    def export(self):
        if self.pending or not self.session:return
        selected=filedialog.asksaveasfilename(parent=self.root,title="Export mission report outside inspected folders",
                    initialfile=self.session.report.mission_name+".inspection.json",defaultextension=".json")
        if selected:
            try:self.session.export(selected)
            except (OSError,ValueError) as exc:messagebox.showerror("Export failed",str(exc),parent=self.root)

    def close(self):
        self.closed=True
        if self.poll_timer is not None:self.root.after_cancel(self.poll_timer)
        if self.owner.explorer is self:self.owner.explorer=None
        self.executor.shutdown(wait=False,cancel_futures=True)
        self.root.destroy()
