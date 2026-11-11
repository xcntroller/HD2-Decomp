"""Docked static script neighborhood/source inspection; parsing stays elsewhere."""
from copy import deepcopy
from dataclasses import asdict
from pathlib import Path
import tkinter as tk
from tkinter import ttk,filedialog,messagebox
from script_panels import source_display


class SpatialScriptPanel:
    def __init__(self,window,parent):
        self.window=window;self.app=window.owner;self.root=window.root;self.frame=parent
        self.index=None;self.identity=None;self.item=None;self.offset=0;self.generation=0
        self.pending=None;self.queued=None;self.timer=None;self.debounce=None;self.closed=False
        self.rows={};self.result=None;self.edges=[];self.overlay_report=None;self.anchor=None
        bar=ttk.Frame(parent);bar.pack(fill='x')
        self.query=tk.StringVar();self.direction=tk.StringVar(value='All');self.kind=tk.StringVar(value='All')
        ttk.Entry(bar,textvariable=self.query,width=22).pack(side='left',fill='x',expand=True)
        self.query.trace_add('write',self.schedule)
        for variable,values,width in ((self.direction,('All','Outgoing','Incoming'),10),
                (self.kind,('All','Binding','Reference','Event','Command','Label','Variable','Control','Include','Diagnostic','Notice','Unavailable'),12)):
            box=ttk.Combobox(bar,textvariable=variable,values=values,state='readonly',width=width);box.pack(side='left',padx=3)
            box.bind('<<ComboboxSelected>>',lambda _:self.refresh(True))
        self.overlay=tk.BooleanVar(value=False);self.labels=tk.BooleanVar(value=False);self.single=tk.BooleanVar(value=True)
        ttk.Checkbutton(bar,text='Static links',variable=self.overlay,command=self.display_edges).pack(side='left')
        ttk.Checkbutton(bar,text='Labels',variable=self.labels,command=self.display_edges).pack(side='left')
        ttk.Checkbutton(bar,text='Selected only',variable=self.single,command=self.display_edges).pack(side='left')
        ttk.Label(parent,text='Static references, never execution · dashed orange: name candidate · unresolved/nonspatial targets stay in this panel',style='Muted.TLabel').pack(anchor='w')
        self.context=ttk.Label(parent,text='Select a spatial object.',wraplength=1100);self.context.pack(anchor='w')
        actions=ttk.Frame(parent);actions.pack(fill='x')
        ttk.Button(actions,text='Previous',command=lambda:self.page(-1)).pack(side='left')
        ttk.Button(actions,text='Next',command=lambda:self.page(1)).pack(side='left',padx=3)
        ttk.Button(actions,text='Script Inspector',command=self.inspector).pack(side='left',padx=3)
        ttk.Button(actions,text='Resource / record',command=self.resource).pack(side='left')
        ttk.Button(actions,text='Source object',command=lambda:self.endpoint(False)).pack(side='left')
        ttk.Button(actions,text='Target object',command=lambda:self.endpoint(True)).pack(side='left',padx=3)
        ttk.Button(actions,text='Back to context',command=self.back).pack(side='left')
        ttk.Button(actions,text='Export neighborhood…',command=self.export).pack(side='left',padx=3)
        self.count=ttk.Label(actions,text='');self.count.pack(side='right')
        split=ttk.Panedwindow(parent,orient='horizontal');split.pack(fill='both',expand=True)
        left=ttk.Frame(split);right=ttk.Frame(split);split.add(left,weight=1);split.add(right,weight=1)
        self.tree=ttk.Treeview(left,columns=('kind','state','location'),show='tree headings',selectmode='browse',height=8)
        for col,label,width in (('#0','Object → script → construct/reference',260),('kind','Kind',70),('state','Certainty / state',160),('location','Source',120)):
            self.tree.heading(col,text=label);self.tree.column(col,width=width)
        scroll=ttk.Scrollbar(left,orient='vertical',command=self.tree.yview);scroll.pack(side='right',fill='y');self.tree.configure(yscrollcommand=scroll.set);self.tree.pack(fill='both',expand=True)
        horizontal=ttk.Scrollbar(left,orient='horizontal',command=self.tree.xview);horizontal.pack(side='bottom',fill='x',before=self.tree)
        self.tree.configure(xscrollcommand=horizontal.set)
        self.tree.bind('<<TreeviewSelect>>',self.select);self.tree.bind('<Double-1>',lambda _:self.inspector())
        tabs=ttk.Notebook(right);tabs.pack(fill='both',expand=True)
        src=ttk.Frame(tabs);detail=ttk.Frame(tabs);tabs.add(src,text='Original source');tabs.add(detail,text='Exact evidence / provenance')
        self.source_label=ttk.Label(src,text='Select a construct/reference to show its original source.',wraplength=650);self.source_label.pack(anchor='w')
        self.location=tk.StringVar();self.location_choice=ttk.Combobox(src,textvariable=self.location,state='readonly')
        self.location_choice.pack(fill='x');self.location_choice.bind('<<ComboboxSelected>>',self.location_selected)
        self.source=self.app.text(src);self.source.configure(height=8,wrap='none')
        self.source.tag_configure('navigation',background='#ffe8a4');self.source.tag_configure('diagnostic',underline=True,foreground='#b71c1c')
        for name,color in [('string','#98622b'),('number','#7054a3'),('keyword','#245c9e'),('comment','#657567')]:self.source.tag_configure(name,foreground=color)
        self.evidence=self.app.text(detail);self.evidence.configure(wrap='word',height=8)

    def reset(self,index):
        self.generation+=1;self.queued=None;self.index=index;self.identity=None;self.item=None;self.anchor=None
        self.edges=[];self.rows={};self.tree.delete(*self.tree.get_children());self.result=None;self.overlay_report=None
        self.context.configure(text='Select a spatial object.');self.count.configure(text='');self.location.set('');self.location_choice.configure(values=[])
        self.app.set_text(self.source,'');self.app.set_text(self.evidence,'');self.display_edges()

    def choose(self,identity):
        self.identity=identity;self.offset=0;self.item=None
        self.result=None;self.overlay_report=None;self.location.set('');self.location_choice.configure(values=[])
        self.edges=[];self.rows={};self.tree.delete(*self.tree.get_children());self.display_edges()
        self.app.set_text(self.source,'Loading bounded script neighborhood…')
        self.generation+=1
        self.refresh()

    def schedule(self,*_):
        if self.debounce:self.root.after_cancel(self.debounce)
        self.debounce=self.root.after(180,lambda:self.refresh(True))

    def refresh(self,reset=False):
        self.debounce=None
        if not self.index or self.identity not in self.index.scene.nodes:return
        if reset:self.offset=0
        self.generation+=1;index=self.index;identity=self.identity
        query=self.query.get();direction=self.direction.get();kind=self.kind.get();offset=self.offset
        def work():
            result=index.query(identity,query,direction,kind,offset)
            overlays=index.overlays(identity,query,direction) if kind in ('All','Reference') else {'edges':[],'total':0,'omitted':0}
            return result,overlays
        self.submit('query',work)

    def submit(self,kind,function):
        if self.closed:return
        #One running job and one replaceable latest request; no unbounded queue.
        request=(self.generation,kind,function)
        if self.pending:self.queued=request;return
        generation,kind,function=request
        self.pending=(generation,kind,self.window.executor.submit(function));self.timer=self.root.after(40,self.poll)

    def poll(self):
        self.timer=None
        if self.closed:return
        generation,kind,future=self.pending
        if not future.done():self.timer=self.root.after(40,self.poll);return
        if kind=='inspector' and self.app.pending:self.timer=self.root.after(40,self.poll);return
        self.pending=None
        try:
            result=future.result()
            if generation==self.generation:
                if kind=='query':self.show(*result)
                elif kind=='source':self.show_source(*result)
                elif kind=='inspector':
                    data,report,location=result
                    self.app.show_snapshot(data,report,[self.index.scene.store.root],self.window.session.include_context.source,on_return=self.window.comeback)
                    if location:self.app.analysis.navigate(location)
        except Exception as exc:
            if generation==self.generation:
                self.source_label.configure(text='Relationship/source inspection unavailable: '+str(exc))
                self.app.set_text(self.source,'Unavailable; no alternate or shadowed source was substituted.')
        if self.queued:
            request=self.queued;self.queued=None
            if request[0]==self.generation:self.submit(request[1],request[2])

    def show(self,result,overlays):
        self.result=result;self.edges=overlays['edges'];self.overlay_report={**overlays,'edges':[asdict(e) for e in self.edges]};self.rows={};self.tree.delete(*self.tree.get_children());self.item=None
        node=self.index.scene.nodes[self.identity]
        actors=', '.join(dict.fromkeys(a['type_label'] for i in self.index.scope(self.identity) for a in self.index.scene.nodes[i].actors)) or node.category
        self.context.configure(text=f'{node.name} · {actors} · {node.identity} · {result["interpretation"]}')
        self.count.configure(text=f'{min(result["offset"]+len(result["items"]),result["total"])}/{"≥" if not result["total_is_exact"] else ""}{result["total"]} rows · {len(self.edges)}/{"≥" if not overlays.get("total_is_exact",True) else ""}{overlays["total"]} eligible links')
        self.tree.insert('','end',iid='object',text=node.name or node.identity,open=True)
        parents={};contexts={}
        for n,item in enumerate(result['items']):
            group=(item['direction'],item['script'])
            if group not in parents:
                key='group:'+str(len(parents));parents[group]=key
                self.tree.insert('object','end',iid=key,text=item['direction']+' · '+(item['script'] or 'Unresolved / notice'),open=True)
            key='row:'+str(n);self.rows[key]=item
            loc=item['locations'][0] if item['locations'] else None
            position=f'{Path(loc["source"]).name}:{loc["begin"]["line"]}:{loc["begin"]["column"]}' if loc else ''
            parent=parents[group]
            event_contexts=item['detail'].get('events',[])
            if event_contexts:
                ctx=event_contexts[0];ctxkey=(group,ctx['statement'])
                if ctxkey not in contexts:
                    branch='context:'+str(len(contexts));contexts[ctxkey]=branch
                    event=self.index.files[item['script']].script['events'][ctx['event_index']]
                    label=ctx['name']+(' '+str(ctx['identity']) if ctx['identity'] is not None else '')
                    self.tree.insert(parent,'end',iid=branch,text=label,values=('Event block','compiler containment',''),open=True)
                    self.rows[branch]=dict(item,identity=ctx['statement'],kind='Event',label=label,locations=event['locations'],detail=event,target_object=None)
                parent=contexts[ctxkey]
            self.tree.insert(parent,'end',iid=key,text=item['label'],values=(item['kind'],item['certainty']+' / '+item['status'],position))
            if item['kind']=='Reference' and item['target_object']:
                node=self.index.scene.nodes[item['target_object']]
                leaf=key+':target';self.rows[leaf]=item
                self.tree.insert(key,'end',iid=leaf,text='Target: '+node.name,values=('Spatial name','candidate; '+node.world_status,''))
            if item['kind']=='Reference':
                for n,target in enumerate(item['detail'].get('resources',[])[:64]):
                    if target not in self.index.files:continue
                    leaf=key+':resource:'+str(n);entry=self.index.files[target]
                    self.rows[leaf]=dict(item,identity=target,kind='Resource',label=target,
                        script=target if entry.format=='script' else None,locations=[],target_object=None,
                        detail={'resource':target,'origin_reference':item['identity'],'origin_script':item['script'],'resolution':entry.resolution})
                    self.tree.insert(key,'end',iid=leaf,text='Resource: '+target,values=('Resource',item['certainty']+' / '+entry.status,''))
        self.app.set_text(self.source,'Select a script construct or reference.');self.source_label.configure(text='Original source; byte spans and archive selection are preserved.')
        self.display_edges()

    def select(self,_=None):
        keys=self.tree.selection();item=self.rows.get(keys[0]) if keys else None
        if not item:return
        self.generation+=1
        self.item=item;self.window.text(self.evidence,self.index.evidence(item))
        self.set_locations(item)
        edges=[e for e in self.edges if e.reference==item['identity']]
        self.display_edges()
        self.window.viewport.relationships.selected=edges[0].identity if edges else None;self.window.viewport.dirty=True
        if item.get('script'):
            self.app.set_text(self.source,'Loading original source…')
            index=self.index;snapshot=dict(item)
            self.submit('source',lambda:index.source(snapshot))
        else:self.app.set_text(self.source,'No source script is associated with this row.')

    def show_source(self,data,report,identity,location):
        text,offsets=source_display(data);self.app.set_text(self.source,text)
        def tag(name,loc):
            if loc['source']!=identity:return
            a=min(loc['start'],len(offsets)-1);b=min(loc['end'],len(offsets)-1)
            self.source.tag_add(name,f'1.0 + {offsets[a]} chars',f'1.0 + {offsets[b]} chars')
        keywords={m['span']['start'] for m in report['lexical_mentions']}
        for token in report['tokens']:
            kind='keyword' if token['span']['start'] in keywords else token['kind']
            if kind in ('comment','keyword','string','number'):
                for loc in token['locations']:tag(kind,loc)
        for diagnostic in report['diagnostics']:
            for loc in diagnostic.get('locations',[]):tag('diagnostic',loc)
        if location:
            tag('navigation',location);self.source.see(f'1.0 + {offsets[min(location["start"],len(offsets)-1)]} chars')
        self.source_label.configure(text=identity+(f' · {location["begin"]["line"]}:{location["begin"]["column"]} · bytes {location["start"]}–{location["end"]}' if location else ''))

    def display_edges(self):
        layer=self.window.viewport.relationships
        selected=[e for e in self.edges if self.item and e.reference==self.item['identity']]
        layer.edges=list(selected if self.single.get() and self.item else self.edges) if self.overlay.get() else []
        layer.labels=self.labels.get();layer.selected=None;self.window.viewport.dirty=True

    def set_locations(self,item):
        locations=item.get('locations',[])
        self.location_choice.configure(values=[f'{loc["source"]}:{loc["begin"]["line"]} · bytes {loc["start"]}–{loc["end"]}' for loc in locations])
        self.location.set('')
        if locations:self.location_choice.current(0)

    def location_selected(self,_=None):
        if not self.item:return
        self.generation+=1;snapshot=dict(self.item,location_index=max(0,self.location_choice.current()))
        self.item=snapshot;index=self.index;self.submit('source',lambda:index.source(snapshot))

    def edge_selected(self,edge):
        if not self.index:return
        self.generation+=1
        ref=self.index.references[edge.reference]
        self.item=self.index.reference_item(ref,'Incoming' if edge.target in self.index.scope(self.identity) else 'Outgoing',edge.source).metadata()
        self.set_locations(self.item)
        self.window.text(self.evidence,self.index.evidence(self.item))
        self.window.viewport.relationships.selected=edge.identity;self.window.tabs.select(self.frame)
        index=self.index;item=dict(self.item);self.submit('source',lambda:index.source(item))

    def page(self,step):
        if step>0 and self.result and self.offset+250>=self.result['total']:return
        self.offset=max(0,self.offset+step*250);self.refresh()
    def endpoint(self,target):
        if not self.item:return
        identity=self.item['target_object'] if target else self.item['source_object']
        if not identity:
            self.source_label.configure(text='No unique supported spatial endpoint; runtime/ambiguous/nonspatial references stay unresolved.');return
        if not self.anchor:self.anchor=(self.identity,deepcopy(self.window.viewport.camera))
        self.window.viewport_select(identity);self.window.viewport.selected=identity;self.window.viewport.dirty=True
    def back(self):
        if not self.anchor:return
        identity,camera=self.anchor;self.anchor=None;self.window.viewport_select(identity)
        self.window.viewport.camera=camera;self.window.mode.set(camera.mode);self.window.viewport.selected=identity;self.window.viewport.dirty=True
    def resource(self):
        if not self.item:return
        node=self.index.scene.nodes.get(self.item.get('target_object'))
        if node:identity=node.inspection_file;target=node.inspection_source
        else:
            identity=self.item['detail'].get('resource')
            resources=self.item['detail'].get('resources',[])
            if not identity and len(resources)==1 and resources[0] in self.index.files:identity=resources[0]
            if not identity and self.item['kind']=='Binding':identity=self.item['script']
            target=identity
        if not identity:self.source_label.configure(text='No unique resource/record target is established.');return
        self.app.open_explorer();self.app.explorer.navigate_session(self.window.session,identity,target)
    def inspector(self):
        if not self.item or not self.item.get('script') or self.app.pending:return
        index=self.index;item=dict(self.item);context=self.window.session.include_context;inspector=self.app.inspector
        def work():
            entry=index.files.get(item['script'])
            if entry and entry.script:
                _,report,_,location=index.source(item)
                root=dict(item,locations=[]);data,_,_,_=index.source(root)
            else:
                from script_client import inspect_bytes
                data=index.scene.store.read(item['script'][9:])
                if len(data)>65535:raise ValueError('Script exceeds the source inspection bound')
                report=inspect_bytes(inspector,data,item['script'],context);location=None
            return data,report,location
        self.submit('inspector',work)
    def export(self):
        if not self.result:return
        path=filedialog.asksaveasfilename(parent=self.root,title='Export bounded static script neighborhood',defaultextension='.json')
        if not path:return
        try:
            value={'schema_version':1,'mission':self.index.scene.mission,'query':self.result,'overlays':self.overlay_report,
                'coverage':self.index.coverage(),'resolution_profile':self.index.scene.store.profile()}
            self.index.export(path,value)
        except (ValueError,OSError) as exc:messagebox.showerror('Relationship export',str(exc),parent=self.root)
    def close(self):
        self.closed=True;self.queued=None;self.index=None
        for timer in (self.timer,self.debounce):
            if timer:self.root.after_cancel(timer)
