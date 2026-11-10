"""Integrated read-only model browser, hierarchy/properties and OpenGL viewport."""
from concurrent.futures import ThreadPoolExecutor
from dataclasses import asdict
import json
from pathlib import Path
import tkinter as tk
from tkinter import ttk,filedialog,messagebox

from gl_view import GLViewport
from model_resources import ModelResources,open_local
from model4ds import Model
from resource_store import ResourceStore

PAGE=250


class ModelWindow:
    def __init__(self,owner):
        self.owner=owner;self.root=tk.Toplevel(owner.root);self.root.title('H&D2 · 4DS Model Inspector')
        self.root.geometry('1380x900');self.root.minsize(950,620);self.root.protocol('WM_DELETE_WINDOW',self.close)
        self.executor=ThreadPoolExecutor(max_workers=1);self.pending=None;self.timer=None;self.closed=False
        self.store=None;self.asset=None;self.names=[];self.offset=0;self.rows={};self.origin=None;self.query_timer=None
        shell=ttk.Frame(self.root,padding=12);shell.pack(fill='both',expand=True)
        bar=ttk.Frame(shell);bar.pack(fill='x',pady=(0,8))
        self.open_button=ttk.Button(bar,text='Open local 4DS…',command=self.open_file);self.open_button.pack(side='left')
        self.install_button=ttk.Button(bar,text='Open installation…',command=self.open_installation);self.install_button.pack(side='left',padx=6)
        self.back_button=ttk.Button(bar,text='Back to mission record',command=self.back,state='disabled');self.back_button.pack(side='left',padx=6)
        self.export_button=ttk.Button(bar,text='Export model report…',command=self.export,state='disabled');self.export_button.pack(side='right')
        self.profile=ttk.Label(shell,text='No installation selected · local models use explicit fallback materials',wraplength=1250)
        self.profile.pack(anchor='w',pady=(0,5))
        self.status=ttk.Label(shell,text='Open a local model or choose an installation.',wraplength=1250);self.status.pack(anchor='w')
        panes=ttk.Panedwindow(shell,orient='horizontal');panes.pack(fill='both',expand=True,pady=(8,0))
        left=ttk.Frame(panes);panes.add(left,weight=1)
        ttk.Label(left,text='INSTALLATION MODELS').pack(anchor='w')
        self.query=tk.StringVar();ttk.Entry(left,textvariable=self.query).pack(fill='x',pady=5)
        self.query.trace_add('write',self.schedule_search)
        pager=ttk.Frame(left);pager.pack(side='bottom',fill='x',pady=5)
        ttk.Button(pager,text='Previous',command=lambda:self.page(-1)).pack(side='left')
        ttk.Button(pager,text='Next',command=lambda:self.page(1)).pack(side='left',padx=4)
        self.count=ttk.Label(pager,text='');self.count.pack(side='right')
        self.resources=owner.tree(left,('Resource','Selection'))
        self.resources.column('Resource',width=220);self.resources.column('Selection',width=100)
        self.resources.bind('<Double-1>',self.select_resource)
        self.load_button=ttk.Button(left,text='Open selected model',command=self.select_resource,state='disabled')
        self.load_button.pack(side='bottom',fill='x',pady=5)
        right=ttk.Frame(panes);panes.add(right,weight=4)
        tools=ttk.Frame(right);tools.pack(fill='x',pady=(0,6))
        ttk.Button(tools,text='Fit model',command=self.fit).pack(side='left')
        self.options={}
        for label,key,default in [('Wireframe','wire',False),('Textures','textured',True),('Grid','grid',True),('Axes','axes',True),('Bounds','boxes',True)]:
            variable=tk.BooleanVar(value=default);self.options[key]=variable
            ttk.Checkbutton(tools,text=label,variable=variable,command=self.options_changed).pack(side='left',padx=3)
        ttk.Label(tools,text='LOD').pack(side='left',padx=(8,3));self.lod=tk.StringVar(value='0')
        self.lod_box=ttk.Combobox(tools,textvariable=self.lod,values=['0'],state='readonly',width=3)
        self.lod_box.pack(side='left');self.lod_box.bind('<<ComboboxSelected>>',self.options_changed)
        ttk.Label(right,text='Left drag: orbit · middle/right or Shift+left: pan · wheel: zoom · Y-up display convention',style='Muted.TLabel').pack(anchor='w',pady=(0,5))
        vertical=ttk.Panedwindow(right,orient='vertical');vertical.pack(fill='both',expand=True)
        view_box=ttk.Frame(vertical);vertical.add(view_box,weight=3)
        self.viewport=GLViewport(view_box,self.viewport_error,height=460);self.viewport.pack(fill='both',expand=True)
        self.backend=ttk.Label(right,text='OpenGL initialization pending',style='Muted.TLabel');self.backend.pack(side='bottom',anchor='w')
        details=ttk.Frame(vertical);vertical.add(details,weight=2)
        inner=ttk.Panedwindow(details,orient='horizontal');inner.pack(fill='both',expand=True)
        tree_box=ttk.Frame(inner);inner.add(tree_box,weight=1)
        self.hierarchy=ttk.Treeview(tree_box,columns=('type','status'),show='tree headings',selectmode='browse')
        for key,text in (('#0','Frames / meshes / materials'),('type','Type'),('status','Status')):self.hierarchy.heading(key,text=text)
        self.hierarchy.column('#0',width=190);self.hierarchy.column('type',width=90);self.hierarchy.column('status',width=90)
        scroll=ttk.Scrollbar(tree_box,orient='vertical',command=self.hierarchy.yview);self.hierarchy.configure(yscrollcommand=scroll.set)
        scroll.pack(side='right',fill='y');self.hierarchy.pack(fill='both',expand=True)
        self.hierarchy.bind('<<TreeviewSelect>>',self.select_node)
        property_box=ttk.Frame(inner);inner.add(property_box,weight=2)
        self.properties=owner.text(property_box);self.properties.configure(height=12,wrap='word')
        ttk.Label(shell,text='Read-only · original coordinates and record identities · RGB/material preview is an approximation · no placement, editing or execution',style='Muted.TLabel').pack(anchor='w',pady=(8,0))

    def submit(self,kind,function):
        if self.pending or self.closed:return
        self.pending=(kind,self.executor.submit(function));self.busy(True)
        self.timer=self.root.after(50,self.poll)

    def busy(self,value):
        for button in (self.open_button,self.install_button,self.load_button):button.configure(state='disabled' if value else 'normal')
        self.export_button.configure(state='normal' if self.asset and not value else 'disabled')
        self.status.configure(text='Loading read-only model resources…' if value else self.status.cget('text'))

    def poll(self):
        self.timer=None
        if self.closed:return
        kind,future=self.pending
        if not future.done():self.timer=self.root.after(50,self.poll);return
        self.pending=None
        try:
            result=future.result()
            if kind=='installation':
                self.set_store(result);self.status.configure(text=f'{len(self.names)} model resources indexed; choose a model.')
            else:self.show(result)
        except Exception as exc:
            kind=getattr(exc,'kind','unavailable')
            state='invalid' if kind in ('invalid','integrity_error','bounds_error') else 'unavailable'
            self.status.configure(text='Selected resource '+state+'; no fallback: '+str(exc))
            # Do not leave old geometry looking like the failed resource.
            self.asset=None;self.rows={};self.viewport.set_model(Model('',0,''));self.hierarchy.delete(*self.hierarchy.get_children())
            self.owner.set_text(self.properties,str(exc))
        finally:self.busy(False)

    def set_store(self,store):
        self.store=store;self.names=sorted(n for n in store.entries if n.startswith('models/') and n.endswith('.4ds'))
        self.profile.configure(text=f'{store.root} · language {store.language} · '+('archives first' if store.archives_first else 'loose first')+' · selected Patch overrides retained')
        self.search(True)

    def open_file(self):
        if self.pending:return
        path=filedialog.askopenfilename(parent=self.root,title='Open original 4DS model (read-only)',filetypes=[('4DS models','*.4ds'),('All files','*')])
        if path:
            self.origin=None;self.back_button.configure(state='disabled')
            self.submit('model',lambda:open_local(path))

    def open_installation(self):
        if self.pending:return
        root=filedialog.askdirectory(parent=self.root,title='Open original installation (read-only)')
        if root:self.submit('installation',lambda:ResourceStore(root))

    def open_resource(self,store,name,origin=None):
        if self.pending:return
        self.set_store(store);self.origin=origin
        self.back_button.configure(state='normal' if origin else 'disabled')
        self.submit('model',lambda:ModelResources(store).open(name))
        self.root.lift()

    def select_resource(self,_=None):
        if self.pending or not self.store:return
        selection=self.resources.selection()
        if selection:self.open_resource(self.store,selection[0])

    def schedule_search(self,*_):
        if self.query_timer:self.root.after_cancel(self.query_timer)
        self.query_timer=self.root.after(150,lambda:self.search(True))

    def page(self,direction):self.offset=max(0,self.offset+direction*PAGE);self.search()

    def search(self,reset=False):
        self.query_timer=None
        if reset:self.offset=0
        terms=self.query.get().casefold().split();matches=[n for n in self.names if all(t in n for t in terms)]
        if self.offset>=len(matches):self.offset=max(0,(len(matches)-1)//PAGE*PAGE)
        self.resources.delete(*self.resources.get_children())
        for name in matches[self.offset:self.offset+PAGE]:
            resolution=self.store.resolution(name);source=resolution.selected
            self.resources.insert('','end',iid=name,values=(name,source.origin if source else resolution.status))
        self.count.configure(text=f'{min(self.offset+PAGE,len(matches))} / {len(matches)}')

    def show(self,asset):
        self.asset=asset;model=asset.model;stats=model.statistics()
        self.viewport.set_model(model,asset.images);self.backend.configure(text=self.viewport.backend)
        self.status.configure(text=f"{model.identity} · {model.status} · {stats['decoded_frames']}/{stats['declared_frames']} frames · {stats['stored_vertices']} stored vertices · {stats['lod0_instance_triangles']} LOD0 triangles · {len(asset.images)} RGB texture previews")
        self.lod_box.configure(values=[str(i) for i in range(max((len(g.lods) for g in model.geometries),default=1))]);self.lod.set('0')
        self.viewport.lod_index=0;self.rows={};self.hierarchy.delete(*self.hierarchy.get_children())
        self.hierarchy.insert('','end',iid='model',text=Path(model.identity).name,values=('4DS',model.status),open=True)
        self.rows['model']=asset.metadata()
        for frame in model.frames:
            key='frame:'+str(frame.index);parent='frame:'+str(frame.parent) if frame.parent else 'model'
            self.hierarchy.insert(parent,'end',iid=key,text=frame.name or f'Frame{frame.index}',values=(f'{frame.kind}/{frame.visual_type}' if frame.visual_type is not None else frame.kind,frame.status),open=False)
            self.rows[key]=frame
            if frame.geometry:
                for lod in frame.geometry.lods:
                    child=key+':lod:'+str(lod.index)
                    self.hierarchy.insert(key,'end',iid=child,text=f'LOD{lod.index}: {lod.vertex_count} vertices',values=('mesh','decoded'))
                    self.rows[child]=(frame,lod)
        self.hierarchy.insert('model','end',iid='materials',text='Materials / texture dependencies',values=('table',len(model.materials)))
        self.rows['materials']={'materials':[asdict(m) for m in model.materials],'textures':asset.textures}
        for material in model.materials:
            key='material:'+str(material.index)
            texture_status='RGB preview' if material.index in asset.images else 'fallback' if material.primary_texture else 'neutral preview'
            self.hierarchy.insert('materials','end',iid=key,text=f'{material.index}: {material.primary_texture or "untextured"}',values=('material',texture_status))
            self.rows[key]={'material':asdict(material),'textures':[t for t in asset.textures if t['material']==material.index]}
        self.hierarchy.selection_set('model');self.select_node()

    def select_node(self,_=None):
        selection=self.hierarchy.selection()
        if not selection:return
        key=selection[0];value=self.rows.get(key)
        if value is None:return
        if key.startswith('frame:'):
            if isinstance(value,tuple):
                frame,lod=value;details={'frame':frame.metadata(),'mesh':lod.metadata()}
                self.lod.set(str(lod.index));self.viewport.lod_index=lod.index
            else:frame=value;details=frame.metadata()
            self.viewport.selected=frame.identity
        else:details=value;self.viewport.selected=None
        # Bounds the property preview; the deterministic export retains all fields.
        chunks=[];size=0
        for chunk in json.JSONEncoder(indent=2,ensure_ascii=True,allow_nan=False).iterencode(details):
            chunks.append(chunk);size+=len(chunk)
            if size>131072:chunks.append('\n[Property preview limited; export full metadata]');break
        self.owner.set_text(self.properties,''.join(chunks));self.viewport.dirty=True

    def options_changed(self,_=None):
        for key,value in self.options.items():setattr(self.viewport,key,value.get())
        self.viewport.lod_index=int(self.lod.get());self.viewport.dirty=True

    def fit(self):self.viewport.fit()

    def viewport_error(self,message):
        self.backend.configure(text=message)

    def back(self):
        if self.origin:self.origin()

    def export(self):
        if not self.asset or self.pending:return
        name=filedialog.asksaveasfilename(parent=self.root,title='Export model inspection metadata',defaultextension='.json',filetypes=[('JSON','*.json')])
        if not name:return
        try:
            path=Path(name)
            if any(path.resolve().is_relative_to(root) for root in self.asset.protected_roots):raise ValueError('Export outside all original input folders/installations')
            with path.open('x',encoding='utf-8') as stream:
                stream.writelines(json.JSONEncoder(indent=2,sort_keys=True,ensure_ascii=True,allow_nan=False).iterencode(self.asset.metadata()));stream.write('\n')
        except (ValueError,OSError) as exc:messagebox.showerror('Model export',str(exc),parent=self.root)

    def close(self):
        if self.closed:return
        self.closed=True
        for timer in (self.timer,self.query_timer):
            if timer:self.root.after_cancel(timer)
        self.viewport.close();self.executor.shutdown(wait=False,cancel_futures=True)
        self.asset=None;self.rows={};self.store=None;self.pending=None
        self.root.destroy();self.owner.model_viewer=None
