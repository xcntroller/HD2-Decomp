"""Searchable entity/graph/validation views integrated into the existing explorer."""
from dataclasses import asdict
import json
import tkinter as tk
from tkinter import ttk

from dependency_graph import search_graph

PAGE=250


class ExplorerPanels:
    def __init__(self, explorer, tabs):
        self.explorer=explorer;self.owner=explorer.owner;self.report=None
        self.nodes={};self.edges={};self.entities={};self.issues={};self.offsets={}
        self.views={};self.current_edge=None
        for name,columns in [
            ("Actors / objects",("Name","Category","Type","Identity")),
            ("Dependency graph",("Source","Relationship","Target","Confidence","Status")),
            ("Validation",("Severity","Code","Source","Message"))]:
            frame=ttk.Frame(tabs,padding=8);tabs.add(frame,text=name)
            bar=ttk.Frame(frame);bar.pack(fill="x",pady=(0,8))
            ttk.Label(bar,text="Search").pack(side="left")
            query=tk.StringVar()
            ttk.Entry(bar,textvariable=query).pack(side="left",fill="x",expand=True,padx=8)
            count=ttk.Label(bar,text="");count.pack(side="right")
            # Reserve actions/details before the expanding list.
            footer=ttk.Frame(frame);footer.pack(side="bottom",fill="x",pady=(8,0))
            ttk.Button(footer,text="Previous",command=lambda n=name:self.page(n,-1)).pack(side="left")
            ttk.Button(footer,text="Next",command=lambda n=name:self.page(n,1)).pack(side="left",padx=6)
            if name=="Dependency graph":
                ttk.Button(footer,text="Inspect source",command=lambda:self.inspect_edge(False)).pack(side="right")
                ttk.Button(footer,text="Inspect target",command=lambda:self.inspect_edge(True)).pack(side="right",padx=6)
                canvas=tk.Canvas(frame,height=245,background="#ffffff",highlightthickness=0)
                canvas.pack(side="bottom",fill="x",pady=(6,0))
                self.canvas=canvas
                canvas.bind("<Configure>",lambda _:self.draw(self.current_edge) if self.current_edge else None)
            else:
                detail=self.owner.text(frame)
                # Repack the helper frame beneath the table, with a bounded height.
                detail.master.pack_forget();detail.master.pack(side="bottom",fill="x",pady=(8,0))
                detail.configure(height=9,wrap="word")
            table=self.owner.tree(frame,columns)
            self.views[name]={"query":query,"table":table,"count":count,"detail":None if name=="Dependency graph" else detail}
            self.offsets[name]=0
            query.trace_add("write",lambda *_,n=name:self.search(n,True))
            table.bind("<<TreeviewSelect>>",lambda _,n=name:self.select(n))
            table.bind("<Double-1>",lambda _,n=name:self.inspect_selected(n))

    def refresh(self,report):
        self.report=report
        self.nodes={n.identity:n for n in report.graph.get("nodes",[])}
        self.edges={e.identity:e for e in report.graph.get("edges",[])}
        self.entities={e.identity:e for e in report.entities}
        self.issues={i.identity:i for i in report.validation}
        for name in self.views:self.search(name,True,force=True)

    def clear(self):
        self.report=None;self.nodes={};self.edges={};self.entities={};self.issues={}
        for view in self.views.values():
            view['table'].delete(*view['table'].get_children());view['count'].configure(text="")
            if view['detail']:self.owner.set_text(view['detail'],"")
        self.canvas.delete("all");self.current_edge=None

    def page(self,name,direction):
        self.offsets[name]=max(0,self.offsets[name]+direction*PAGE)
        self.search(name)

    def search(self,name,reset=False,force=False):
        if not self.report or self.explorer.pending and not force:return
        if reset:self.offsets[name]=0
        view=self.views[name];query=view['query'].get().casefold();offset=self.offsets[name]
        if name=="Dependency graph":
            total,items=search_graph(self.report,query,PAGE,offset)
        else:
            source=self.report.entities if name=="Actors / objects" else self.report.validation
            def text(item):
                if name=="Actors / objects":return f"{item.label} {item.category} {item.type_label} {item.type_id} {item.identity}"
                return f"{item.severity} {item.code} {item.source} {item.message} {item.evidence}"
            terms=query.split();matches=[item for item in source if all(t in text(item).casefold() for t in terms)]
            total=len(matches);items=matches[offset:offset+PAGE]
        if offset and not items:
            self.offsets[name]=max(0,((total-1)//PAGE)*PAGE)
            if total:return self.search(name,force=force)
        view['table'].delete(*view['table'].get_children())
        if name=="Dependency graph":self.current_edge=None;self.canvas.delete("all")
        elif view['detail']:self.owner.set_text(view['detail'],"")
        for item in items:
            if name=="Dependency graph":
                values=(self.nodes[item.source].category+" · "+self.nodes[item.source].label,item.relation,
                        self.nodes[item.target].category+" · "+self.nodes[item.target].label,item.certainty,item.status)
            elif name=="Actors / objects":values=(item.label,item.category,f"{item.type_label} ({item.type_id})",item.identity)
            else:values=(item.severity,item.code,item.source,item.message)
            view['table'].insert("","end",iid=item.identity,values=values)
        view['count'].configure(text=f"{offset+1 if total else 0}–{min(offset+PAGE,total)} / {total}")

    def select(self,name):
        if self.explorer.pending:return
        view=self.views[name];selection=view['table'].selection()
        if not selection:return
        identity=selection[0]
        if name=="Dependency graph":
            edge=self.edges[identity];self.current_edge=edge
            self.draw(edge)
        else:
            item=self.entities[identity] if name=="Actors / objects" else self.issues[identity]
            value=asdict(item)
            if name=="Actors / objects":
                entry=next(f for f in self.report.files if f.path==item.file)
                from mission import walk_records
                props={p.identity:p for p in walk_records(entry.records) if p.identity in item.property_records}
                value['source_resolution']=entry.resolution
                value['archive_decoding']=entry.archive_decoding
                value['serialized_properties']=[asdict(p) for p in props.values()]
                value['relations']=[asdict(e) for e in self.report.graph['edges'] if e.source==identity or e.target==identity]
            self.owner.set_text(view['detail'],json.dumps(value,indent=2,ensure_ascii=True))

    def navigate(self,identity):
        if self.explorer.pending:return
        node=self.nodes.get(identity)
        if node and node.file in self.explorer.session.paths:
            self.explorer.tabs.select(self.explorer.record_tab)
            self.explorer.load_file(node.file,identity)

    def inspect_edge(self,target):
        if self.current_edge:self.navigate(self.current_edge.target if target else self.current_edge.source)

    def inspect_selected(self,name):
        if name=="Dependency graph":self.inspect_edge(True);return
        selection=self.views[name]['table'].selection()
        if not selection:return
        item=(self.entities if name=="Actors / objects" else self.issues)[selection[0]]
        if name=="Actors / objects":self.navigate(item.identity)
        elif item.file in self.explorer.session.paths:
            self.explorer.tabs.select(self.explorer.record_tab);self.explorer.load_file(item.file,item.source)

    def draw(self,selected):
        c=self.canvas;c.delete("all")
        width=max(c.winfo_width(),400)
        half=min(120,max(80,(width-60)/4));center=(half+12,105)
        focus=selected.source
        neighbors=[e for e in self.report.graph['edges'] if e.source==focus or e.target==focus]
        neighbors=[selected]+[e for e in neighbors if e.identity!=selected.identity][:3]
        c.create_text(10,8,anchor="nw",text="Ego view · solid: serialized / selected resource · dashed: candidate or unresolved",fill="#58675f")
        def box(x,y,node,fill):
            tag="node"+str(len(c.find_all()))
            c.create_rectangle(x-half,y-16,x+half,y+16,fill=fill,outline="#66786d",tags=tag)
            label=node.category+" · "+node.label
            maximum=max(16,int((half*2-12)/7))
            label=label if len(label)<=maximum else label[:maximum-1]+"…"
            c.create_text(x,y,text=label,width=half*2-12,tags=tag)
            c.tag_bind(tag,"<Double-1>",lambda _,identity=node.identity:self.navigate(identity))
        for index,edge in enumerate(neighbors):
            target=edge.target if edge.source==focus else edge.source
            y=45+index*43;x=width-half-12
            solid=edge.certainty in ("serialized","resolver_selected")
            c.create_line(center[0]+half,center[1],x-half,y,arrow="last" if edge.source==focus else "first",
                          fill="#396957" if solid else "#9b773d",dash=() if solid else (5,3))
            box(x,y,self.nodes[target],"#eff3ed")
        box(*center,self.nodes[focus],"#dce9dd")
        c.create_text(10,240,anchor="sw",text=f"{selected.relation} · {selected.certainty} · {selected.evidence}",width=width-20,fill="#25312d")
