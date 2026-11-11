"""Integrated read-only spatial hierarchy, navigation and mission viewport."""
from concurrent.futures import ThreadPoolExecutor
from collections import Counter,deque
import json
import math
from pathlib import Path
from threading import Event
import tkinter as tk
from tkinter import ttk,filedialog,messagebox

from mission_view import MissionViewport,CATEGORIES
from spatial_scene import SpatialScene,freeze_session

PAGE=250


class SpatialWindow:
    def __init__(self,owner):
        self.owner=owner;self.root=tk.Toplevel(owner.root);self.root.title('H&D2 · 3D Mission Explorer')
        self.root.geometry('1460x960');self.root.minsize(1000,650);self.root.protocol('WM_DELETE_WINDOW',self.close)
        self.executor=ThreadPoolExecutor(max_workers=1);self.pending=None;self.timer=None;self.tree_timer=None;self.query_timer=None
        self.closed=False;self.cancel=Event();self.scene=None;self.session=None;self.selected=None;self.page_offset=0
        self.pending_source=None
        self.loading_session=None
        self.progress=deque(maxlen=1);self.tree_generation=0;self.setting_selection=False
        shell=ttk.Frame(self.root,padding=10);shell.pack(fill='both',expand=True)
        bar=ttk.Frame(shell);bar.pack(fill='x')
        ttk.Button(bar,text='Choose mission…',command=self.choose).pack(side='left')
        self.reload_button=ttk.Button(bar,text='Reload snapshot',command=self.reload,state='disabled');self.reload_button.pack(side='left',padx=5)
        self.inspect_button=ttk.Button(bar,text='Inspect record',command=self.inspect,state='disabled');self.inspect_button.pack(side='left',padx=5)
        self.model_button=ttk.Button(bar,text='Inspect model',command=self.model,state='disabled');self.model_button.pack(side='left',padx=5)
        self.script_button=ttk.Button(bar,text='Script relationships',command=self.script,state='disabled');self.script_button.pack(side='left',padx=5)
        self.export_button=ttk.Button(bar,text='Export spatial report…',command=self.export,state='disabled');self.export_button.pack(side='right')
        self.status=ttk.Label(shell,text='Choose an installation-aware mission in the existing Mission Explorer.',wraplength=1380)
        self.status.pack(anchor='w',pady=(7,4))
        ttk.Label(shell,text='Partial serialized snapshot · recovered alpha sources / approximate preview lighting and blending · no lightmaps, skin/animation or runtime movement',style='Muted.TLabel').pack(anchor='w')
        controls=ttk.Frame(shell);controls.pack(fill='x',pady=6)
        self.mode=tk.StringVar(value='orbit');box=ttk.Combobox(controls,textvariable=self.mode,values=('orbit','fly'),state='readonly',width=7);box.pack(side='left');box.bind('<<ComboboxSelected>>',self.camera_options)
        ttk.Button(controls,text='Fit scene',command=self.fit).pack(side='left',padx=4)
        ttk.Button(controls,text='Focus selected (F)',command=self.focus).pack(side='left',padx=4)
        ttk.Label(controls,text='Speed (units/s)').pack(side='left',padx=(8,2));self.speed=tk.StringVar(value='20')
        ttk.Spinbox(controls,from_=.1,to=1000,increment=5,textvariable=self.speed,width=6,command=self.camera_options).pack(side='left')
        ttk.Label(controls,text='Far/cull range').pack(side='left',padx=(8,2));self.far=tk.StringVar(value='1000')
        ttk.Spinbox(controls,from_=10,to=10000,increment=100,textvariable=self.far,width=6,command=self.camera_options).pack(side='left')
        self.options={}
        for label,key,value in [('Wire','wire',False),('Textures','textured',True),('Grid aid','grid',True),('Axes','axes',True),('Bounds','boxes',True),('Origins','origins',False),('Include hidden','show_hidden',False)]:
            variable=tk.BooleanVar(value=value);self.options[key]=variable
            ttk.Checkbutton(controls,text=label,variable=variable,command=self.display_options).pack(side='left',padx=2)
        ttk.Button(controls,text='Apply camera values',command=self.camera_options).pack(side='right')
        categories=ttk.Frame(shell);categories.pack(fill='x')
        ttk.Label(categories,text='Visibility:').pack(side='left')
        self.categories={}
        for name in CATEGORIES:
            variable=tk.BooleanVar(value=name not in ('frames','sectors'));self.categories[name]=variable
            ttk.Checkbutton(categories,text=name.title(),variable=variable,command=self.display_options).pack(side='left',padx=5)
        ttk.Label(shell,text='Click: select · fly: WASD, Q/E vertical, Shift faster, right drag look · orbit: left drag · middle drag pan · wheel zoom/speed',style='Muted.TLabel').pack(anchor='w',pady=4)
        panes=ttk.Panedwindow(shell,orient='horizontal');panes.pack(fill='both',expand=True)
        left=ttk.Frame(panes);panes.add(left,weight=1)
        self.query=tk.StringVar();ttk.Entry(left,textvariable=self.query).pack(fill='x',pady=(0,4));self.query.trace_add('write',self.search_later)
        pager=ttk.Frame(left);pager.pack(side='bottom',fill='x',pady=3)
        ttk.Button(pager,text='Previous',command=lambda:self.page(-1)).pack(side='left')
        ttk.Button(pager,text='Next',command=lambda:self.page(1)).pack(side='left',padx=4)
        self.count=ttk.Label(pager,text='');self.count.pack(side='right')
        self.tree=ttk.Treeview(left,columns=('category','state'),show='tree headings',selectmode='browse')
        self.tree.heading('#0',text='Serialized hierarchy / object');self.tree.heading('category',text='Category');self.tree.heading('state',text='Transform')
        self.tree.column('#0',width=200);self.tree.column('category',width=80);self.tree.column('state',width=100)
        scroll=ttk.Scrollbar(left,orient='vertical',command=self.tree.yview);scroll.pack(side='right',fill='y');self.tree.configure(yscrollcommand=scroll.set);self.tree.pack(fill='both',expand=True)
        self.tree.bind('<<TreeviewSelect>>',self.tree_select);self.tree.bind('<Double-1>',lambda _:self.focus())
        right=ttk.Frame(panes);panes.add(right,weight=4)
        vertical=ttk.Panedwindow(right,orient='vertical');vertical.pack(fill='both',expand=True)
        view=ttk.Frame(vertical);vertical.add(view,weight=4)
        self.viewport=MissionViewport(view,self.viewport_select,self.statistics,self.error,height=520);self.viewport.pack(fill='both',expand=True)
        detail=ttk.Frame(vertical);vertical.add(detail,weight=2)
        self.tabs=ttk.Notebook(detail);self.tabs.pack(fill='both',expand=True)
        props=ttk.Frame(self.tabs);self.tabs.add(props,text='Selected object / links');self.properties=owner.text(props);self.properties.configure(wrap='word',height=10)
        issues=ttk.Frame(self.tabs);self.tabs.add(issues,text='Coverage / diagnostics');self.issues=owner.text(issues);self.issues.configure(wrap='word')
        from spatial_script_gui import SpatialScriptPanel
        scripts=ttk.Frame(self.tabs);self.tabs.add(scripts,text='Script Relationships')
        self.script_panel=SpatialScriptPanel(self,scripts)
        self.viewport.on_relationship=self.script_panel.edge_selected
        self.stats=ttk.Label(shell,text='OpenGL pending',wraplength=1400);self.stats.pack(anchor='w',pady=(5,0))

    def choose(self):
        self.owner.open_explorer();self.owner.explorer.root.lift()
        self.status.configure(text='Open an installation, inspect a mission, then choose Explore 3D in Mission Explorer.')

    def load(self,session):
        if self.pending:return
        self.loading_session=session;self.cancel=Event();self.progress.clear()
        snapshot=freeze_session(session)
        self.status.configure(text='Composing original serialized scene…')
        self.pending=('scene',self.executor.submit(SpatialScene,snapshot,self.progress.append,self.cancel.is_set));self.busy(True)
        self.timer=self.root.after(50,self.poll);self.root.lift()

    def reload(self):
        if self.session and not self.pending:self.load(self.session)

    def busy(self,value):
        self.reload_button.configure(state='normal' if self.session and not value else 'disabled')
        self.export_button.configure(state='normal' if self.scene and not value else 'disabled')
        if value:
            for button in (self.inspect_button,self.model_button,self.script_button):button.configure(state='disabled')
        elif self.selected:self.details(self.selected)

    def poll(self):
        self.timer=None
        if self.closed:return
        kind,future=self.pending
        if not future.done():
            if self.progress:self.status.configure(text=self.progress[-1])
            self.timer=self.root.after(50,self.poll);return
        self.pending=None
        try:
            result=future.result()
            self.session=self.loading_session;self.loading_session=None;self.show(result)
        except Exception as exc:
            self.loading_session=None;self.error(str(exc))
        finally:self.busy(False)

    def show(self,scene):
        self.scene=scene;self.selected=None;self.viewport.set_scene(scene)
        self.script_panel.reset(scene.script_relations)
        self.mode.set('orbit');self.speed.set(f'{self.viewport.camera.speed:.2f}');self.far.set(f'{self.viewport.camera.far:.1f}')
        c=scene.counts
        self.status.configure(text=f"{scene.mission} · partial serialized spatial view · {c['mesh_instances']} placed mesh instances · {c['unresolved_transforms']} unresolved transforms · {scene.coverage['spatial']['unresolved_parent_identity']} unresolved parent identities · {c['unresolved_actors']} unresolved actors · {c['assets']} unique assets")
        summary={**scene.coverage,'geometry':{key:value for key,value in scene.coverage['geometry'].items() if key not in ('omissions','assets')}}
        summary['geometry']['omission_samples']=scene.coverage['geometry']['omissions'][:100]
        summary['geometry']['asset_statuses']=[{'identity':a['identity'],'status':a['status'],'decoded_frames':a['decoded_frames'],'declared_frames':a['declared_frames']} for a in scene.coverage['geometry']['assets']]
        self.text(self.issues,summary);self.owner.set_text(self.properties,'Select an actual mesh, origin marker or hierarchy entry.');self.build_tree()
        if self.pending_source:
            source=self.pending_source;self.pending_source=None
            self.focus_source(source)

    def focus_source(self,source):
        if not self.scene:return
        identities=self.scene.source_nodes.get(source,[])
        if not identities:
            # Fields inherit the enclosing mapped record, never a filename guess.
            def find(records,owners=()):
                for record in records:
                    current=self.scene.source_nodes.get(record.identity,owners)
                    if record.identity==source:return current
                    result=find(record.children,current)
                    if result:return result
                return ()
            for entry in self.scene.session.report.files:
                identities=find(entry.records)
                if identities:break
        if len(identities)==1:
            self.viewport_select(identities[0]);self.viewport.selected=identities[0];self.focus()
        else:self.status.configure(text='Selected record has no unique supported spatial association.')

    def search_later(self,*_):
        if self.query_timer:self.root.after_cancel(self.query_timer)
        self.query_timer=self.root.after(180,lambda:self.build_tree(True))

    def page(self,direction):self.page_offset=max(0,self.page_offset+direction*PAGE);self.build_tree()

    def build_tree(self,reset=False):
        self.query_timer=None
        if not self.scene:return
        if reset:self.page_offset=0
        self.tree_generation+=1;generation=self.tree_generation
        if self.tree_timer:self.root.after_cancel(self.tree_timer);self.tree_timer=None
        self.tree.delete(*self.tree.get_children());terms=self.query.get().casefold().split()
        if terms:
            def haystack(n):return f'{n.name} {n.identity} {n.category} {n.status} {n.model_resource} {n.parent_status}'.casefold()
            matches=[n for n in self.scene.nodes.values() if all(t in haystack(n) for t in terms)]
            if self.page_offset>=len(matches):self.page_offset=max(0,(len(matches)-1)//PAGE*PAGE)
            rows=matches[self.page_offset:self.page_offset+PAGE]
            self.count.configure(text=f'{min(self.page_offset+PAGE,len(matches))}/{len(matches)} matches (flat)')
        else:
            rows=[self.scene.nodes[i] for i in self.scene.order];self.count.configure(text=f'{len(rows)} objects')
        self.tree.insert('','end',iid='scene-root',text='Serialized hierarchy' if not terms else 'Search matches (flat)',open=True)
        self.tree.insert('','end',iid='unresolved-root',text='Unresolved parent/transform',open=False)
        position=0
        def batch():
            nonlocal position
            self.tree_timer=None
            if self.closed or generation!=self.tree_generation:return
            for node in rows[position:position+200]:
                parent='unresolved-root' if node.world is None else 'scene-root'
                self.tree.insert(parent,'end',iid=node.identity,text=node.name or '<unnamed>',values=(node.category,'world known / parent unknown' if node.world_status=='verified_translation_only_sector_invariant' else 'resolved' if node.world is not None else node.parent_status))
            position+=200
            if position<len(rows):self.tree_timer=self.root.after(1,batch)
            else:
                if not terms:
                    for node in rows:
                        if node.world is not None and node.parent and self.tree.exists(node.parent):self.tree.move(node.identity,node.parent,'end')
                if self.selected and self.tree.exists(self.selected):
                    self.tree.selection_set(self.selected);self.tree.see(self.selected)
        batch()

    def tree_select(self,_=None):
        if self.setting_selection or not self.scene:return
        values=self.tree.selection()
        if values and values[0] in self.scene.nodes:
            self.selected=values[0];self.viewport.selected=self.selected;self.viewport.dirty=True;self.details(self.selected)

    def viewport_select(self,identity):
        self.selected=identity
        if identity:
            if self.query.get():self.query.set('');self.build_tree()
            self.setting_selection=True
            try:
                if self.tree.exists(identity):self.tree.selection_set(identity);self.tree.see(identity)
            finally:self.setting_selection=False
            self.details(identity)

    def selected_node(self):return self.scene.nodes.get(self.selected) if self.scene else None
    def owner_node(self,node):return self.scene.nodes.get(node.model_owner,node) if node and node.model_owner else node
    def associated_links(self,node):
        if not node:return []
        owner=self.owner_node(node)
        return node.links+(owner.links if owner is not node else [])

    def details(self,identity):
        node=self.scene.nodes.get(identity)
        if not node:return
        if self.script_panel.identity!=identity:self.script_panel.choose(identity)
        owner=self.owner_node(node)
        value=node.metadata()
        value['omission_reasons']=next((r['reasons'] for r in self.scene.coverage['geometry']['omissions'] if r['identity']==identity),[])
        if owner is not node:value['mission_owner']=owner.metadata()
        asset=self.scene.assets.get(node.asset)
        if asset:
            value['model_asset']={'identity':asset.model.identity,'status':asset.model.status,
                'provenance':asset.model.provenance,'texture_status':dict(Counter(t['status'] for t in asset.textures)),
                'material_preview':asset.material_preview,
                'fallback_material':'Neutral gray for unavailable sources; supported alpha sources use approximate preview blending/cutoffs. Original lighting/lightmaps/effects are not reproduced.'}
        value['scope']='Serialized placement, not actual runtime actor/script transform'
        self.text(self.properties,value)
        self.inspect_button.configure(state='normal' if node.inspection_file else 'disabled')
        self.model_button.configure(state='normal' if node.asset in self.scene.assets else 'disabled')
        self.script_button.configure(state='normal')

    def text(self,widget,value):
        chunks=[];size=0
        for part in json.JSONEncoder(indent=2,ensure_ascii=True,allow_nan=False).iterencode(value):
            chunks.append(part);size+=len(part)
            if size>262144:chunks.append('\n[Preview limited; export retains full metadata]');break
        self.owner.set_text(widget,''.join(chunks))

    def inspect(self):
        node=self.selected_node()
        if not node or not node.inspection_file:return
        self.owner.open_explorer();explorer=self.owner.explorer
        explorer.navigate_session(self.session,node.inspection_file,node.inspection_source)

    def comeback(self):
        if not self.closed:
            self.root.lift()
            if self.selected:self.viewport.selected=self.selected;self.viewport.dirty=True

    def model(self):
        node=self.selected_node()
        if not node or node.asset not in self.scene.assets:return
        self.owner.open_model();viewer=self.owner.model_viewer
        if viewer.pending:return
        viewer.set_store(self.scene.store);viewer.origin=self.comeback;viewer.back_button.configure(state='normal')
        viewer.show(self.scene.assets[node.asset]);viewer.busy(False);viewer.root.lift()

    def script(self):
        self.tabs.select(self.script_panel.frame)

    def focus(self):
        self.viewport.focus(self.selected);self.mode.set(self.viewport.camera.mode)
    def fit(self):
        self.viewport.fit();self.mode.set('orbit');self.far.set(str(round(self.viewport.camera.far,1)))
    def camera_options(self,_=None):
        try:
            speed=float(self.speed.get());far=float(self.far.get())
            if not all(math.isfinite(v) for v in (speed,far)):raise ValueError('Nonfinite camera setting')
            self.viewport.camera.switch(self.mode.get())
            self.viewport.camera.speed=max(.1,min(1000,speed))
            self.viewport.camera.far=max(10,min(10000,far))
            self.viewport.dirty=True
        except ValueError:self.status.configure(text='Enter finite numeric camera speed and far/cull range')
    def display_options(self):
        for key,value in self.options.items():setattr(self.viewport,key,value.get())
        self.viewport.categories={name for name,value in self.categories.items() if value.get()};self.viewport.dirty=True
    def statistics(self,value):
        self.stats.configure(text=f"{self.viewport.backend} · {value['mesh_instances']} mesh draws / {value['triangles']} triangles · {value['frame_ms']} ms · culled {value['culled']} / filtered {value['filtered']} / draw-budget omissions {value['draw_budget_omissions']} · camera "+', '.join(f'{v:.2f}' for v in value['camera']))
    def error(self,message):self.status.configure(text='Spatial inspection unavailable: '+message)
    def export(self):
        if not self.scene or self.pending:return
        path=filedialog.asksaveasfilename(parent=self.root,title='Export read-only spatial snapshot',defaultextension='.json')
        if path:
            try:self.scene.export(path)
            except (ValueError,OSError) as exc:messagebox.showerror('Spatial export',str(exc),parent=self.root)
    def close(self):
        if self.closed:return
        self.closed=True;self.cancel.set()
        self.script_panel.close()
        for timer in (self.timer,self.tree_timer,self.query_timer):
            if timer:self.root.after_cancel(timer)
        self.viewport.close();self.executor.shutdown(wait=False,cancel_futures=True)
        self.scene=None;self.session=None;self.loading_session=None;self.pending=None
        if getattr(self.owner,'navigation_return',None)==self.comeback:
            self.owner.navigation_return=None;self.owner.return_button.configure(state='disabled')
        viewer=self.owner.model_viewer
        if viewer and viewer.origin==self.comeback:
            viewer.origin=None;viewer.back_button.configure(state='disabled')
        self.root.destroy();self.owner.spatial_viewer=None
