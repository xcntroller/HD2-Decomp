"""Read-only source navigation and paginated compiler-backed analysis for Tk."""
import hashlib
import json
import tkinter as tk
from tkinter import ttk, messagebox

PAGE=250


class ScriptPanels:
    def __init__(self,owner,source_tab):
        self.owner=owner;self.report=None;self.source=b'';self.identity=None;self.reader=None
        self.rows=[];self.visible={};self.offset=0;self.timer=None
        bar=ttk.Frame(source_tab);bar.pack(fill='x',before=owner.source_text.master,pady=(0,6))
        self.choice=tk.StringVar();self.sources=ttk.Combobox(bar,textvariable=self.choice,state='readonly')
        self.sources.pack(side='left',fill='x',expand=True)
        self.sources.bind('<<ComboboxSelected>>',self.choose_source)
        self.position=ttk.Label(bar,text='');self.position.pack(side='right',padx=8)
        self.source_tab=source_tab
        text=owner.source_text
        for name,color in [('comment','#657567'),('string','#98622b'),('number','#7054a3'),('keyword','#245c9e')]:
            text.tag_configure(name,foreground=color)
        text.tag_configure('diagnostic',underline=True,foreground='#b71c1c')
        text.tag_configure('navigation',background='#ffe8a4')
        tab=owner.tab('Analysis');self.tab=tab
        toolbar=ttk.Frame(tab);toolbar.pack(fill='x',pady=(0,8))
        self.query=tk.StringVar();self.kind=tk.StringVar(value='All')
        ttk.Label(toolbar,text='Search').pack(side='left')
        ttk.Entry(toolbar,textvariable=self.query).pack(side='left',fill='x',expand=True,padx=8)
        box=ttk.Combobox(toolbar,textvariable=self.kind,state='readonly',width=18,
            values=['All','Diagnostic','Event','Declaration','Label','Control','Argument','Symbol use','Reference','Include','Statement'])
        box.pack(side='left');box.bind('<<ComboboxSelected>>',lambda _:self.search(True))
        self.query.trace_add('write',self.schedule)
        foot=ttk.Frame(tab);foot.pack(side='bottom',fill='x',pady=(8,0))
        ttk.Button(foot,text='Previous',command=lambda:self.page(-1)).pack(side='left')
        ttk.Button(foot,text='Next',command=lambda:self.page(1)).pack(side='left',padx=6)
        ttk.Button(foot,text='Go to source',command=self.navigate_selected).pack(side='left',padx=6)
        ttk.Button(foot,text='Follow mission reference',command=self.follow).pack(side='left')
        self.count=ttk.Label(foot,text='');self.count.pack(side='right')
        self.detail=owner.text(tab);self.detail.master.pack_forget();self.detail.master.pack(side='bottom',fill='x',pady=8)
        self.detail.configure(height=8,wrap='word')
        self.table=owner.tree(tab,('Kind','Name / message','Source','Position','Evidence'))
        self.table.bind('<<TreeviewSelect>>',self.select)
        self.table.bind('<Double-1>',lambda _:self.navigate_selected())
        owner.references.bind('<Double-1>',lambda _:self.navigate_tree(owner.references,'references'))
        owner.instructions.bind('<Double-1>',lambda _:self.navigate_tree(owner.instructions,'instructions'))
        owner.variables.bind('<Double-1>',lambda _:self.navigate_symbol())

    def schedule(self,*_):
        if self.timer:self.owner.root.after_cancel(self.timer)
        self.timer=self.owner.root.after(150,lambda:self.search(True))

    def clear(self):
        self.report=None;self.rows=[];self.visible={};self.reader=None
        self.choice.set('');self.sources.configure(values=[])
        self.search(True)

    def set_report(self,source,report,reader=None):
        self.report=report;self.reader=reader;self.root_source=source;self.rows=[]
        self.sources.configure(values=[s['identity'] for s in report['sources']])
        self.choice.set(report['file']);self.display(source,report['file'])
        for kind,key in [('Diagnostic','diagnostics'),('Event','events'),('Declaration','declarations'),('Label','labels'),
                         ('Control','controls'),('Argument','arguments'),('Symbol use','symbol_references'),
                         ('Reference','references'),('Include','includes'),('Statement','statements')]:
            for item in report.get(key,[]):
                label=item.get('message',item.get('name',item.get('literal',item.get('text',item.get('kind','')))))
                locations=item.get('locations',[])
                position=locations[0] if locations else None
                values=(kind,str(label),position['source'] if position else '',
                        f"{position['begin']['line']}:{position['begin']['column']}" if position else '',item.get('evidence','0042C0B0'))
                self.rows.append((values,item,' '.join(map(str,values)).casefold()))
        self.search(True)

    def display(self,data,identity,location=None):
        metadata=next((s for s in self.report['sources'] if s['identity']==identity),None)
        if metadata and hashlib.sha256(data).hexdigest()!=metadata['sha256']:
            raise ValueError('Source changed since analysis; inspect again before navigating')
        self.identity=identity;self.source=data;self.choice.set(identity)
        chars=[];offsets=[0];size=0;previous=None
        for byte in data:
            if byte==10 and previous==13:part=''
            elif byte in (10,13):part='\n'
            elif byte<32 and byte!=9 or byte==127:part=f'\\x{byte:02x}'
            else:part=chr(byte)
            chars.append(part);size+=len(part);offsets.append(size);previous=byte
        self.offsets=offsets;self.owner.set_text(self.owner.source_text,''.join(chars))
        names={m['span']['start'] for m in self.report['lexical_mentions']}
        for token in self.report['tokens']:
            kind='keyword' if token['span']['start'] in names else token['kind']
            if kind in ('comment','string','number','keyword'):
                for loc in token['locations']:
                    if loc['source']==identity:self.tag(kind,loc)
        for diagnostic in self.report['diagnostics']:
            for loc in diagnostic.get('locations',[]):
                if loc['source']==identity:self.tag('diagnostic',loc)
        if location:self.navigate(location)

    def tag(self,name,loc):
        start=min(loc['start'],len(self.offsets)-1);end=min(loc['end'],len(self.offsets)-1)
        self.owner.source_text.tag_add(name,f'1.0 + {self.offsets[start]} chars',f'1.0 + {self.offsets[end]} chars')

    def choose_source(self,_=None,location=None):
        identity=location['source'] if location else self.choice.get()
        if self.owner.pending or not self.report:return
        if identity==self.report['file']:self.display(self.root_source,identity,location)
        elif self.reader:
            self.owner.submit('source',lambda:(self.reader(identity),identity,location))
        else:messagebox.showinfo('Source unavailable','Reopen this script through its installation context.',parent=self.owner.root)

    def navigate(self,loc):
        if not self.report:return
        if loc['source']!=self.identity:self.choose_source(location=loc);return
        text=self.owner.source_text;text.tag_remove('navigation','1.0','end')
        self.tag('navigation',loc);index=f"1.0 + {self.offsets[min(loc['start'],len(self.offsets)-1)]} chars"
        text.mark_set('insert',index);text.see(index)
        self.position.configure(text=f"{loc['begin']['line']}:{loc['begin']['column']} · bytes {loc['start']}–{loc['end']}")
        self.owner.tabs.select(self.source_tab)

    def page(self,direction):self.offset=max(0,self.offset+direction*PAGE);self.search()

    def search(self,reset=False):
        self.timer=None
        if reset:self.offset=0
        terms=self.query.get().casefold().split();kind=self.kind.get()
        matches=[row for row in self.rows if (kind=='All' or row[0][0]==kind) and all(t in row[2] for t in terms)]
        if self.offset>=len(matches):self.offset=max(0,(len(matches)-1)//PAGE*PAGE)
        self.table.delete(*self.table.get_children());self.visible={}
        for values,item,_ in matches[self.offset:self.offset+PAGE]:
            key=self.table.insert('','end',values=values);self.visible[key]=item
        self.count.configure(text=f'{min(self.offset+PAGE,len(matches))} / {len(matches)}')
        self.owner.set_text(self.detail,'')

    def selected(self):
        selection=self.table.selection();return self.visible.get(selection[0]) if selection else None

    def select(self,_=None):
        item=self.selected()
        if item:self.owner.set_text(self.detail,json.dumps(item,indent=2,ensure_ascii=True))

    def navigate_selected(self):
        item=self.selected()
        if item and item.get('locations'):self.navigate(item['locations'][0])

    def navigate_tree(self,tree,key):
        selection=tree.selection()
        if not selection:return
        index=tree.index(selection[0]);items=self.report.get(key,[])
        if index<len(items) and items[index].get('locations'):self.navigate(items[index]['locations'][0])

    def navigate_symbol(self):
        selection=self.owner.variables.selection()
        if not selection:return
        name=self.owner.variables.item(selection[0],'values')[0]
        item=next((d for d in self.report['declarations'] if d['name'].lower()==name.lower()),None)
        if item and item['locations']:self.navigate(item['locations'][0])

    def follow(self):
        item=self.selected()
        if not item:return
        targets=item.get('targets',[]) or ([item['selected']] if item.get('selected') else [])
        explorer=self.owner.explorer
        if targets and explorer and explorer.session:
            explorer.panels.navigate(targets[0]);explorer.root.lift()
        else:messagebox.showinfo('Mission reference','No linked mission target is available. Open the script from Mission Explorer for resolved links.',parent=self.owner.root)
