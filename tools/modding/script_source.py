"""Bounded, provenance-preserving implementation of 0042C580/0042C4B0.

The shared native lexer cursor is deliberately retained. Defensive rejections
are labelled separately from original-compatible missing-include behavior.
"""
from dataclasses import dataclass, asdict
import hashlib
import re
from bisect import bisect_right

MAX_SOURCE=65535


@dataclass(frozen=True)
class Piece:
    start: int
    end: int
    source: str
    origin: int
    lineage: tuple[str,...]


class SourceMap:
    def __init__(self, data, identity, provenance=None):
        self.data=data
        self.sources={identity:(data,provenance)}
        self.lines={}
        self.pieces=[Piece(0,len(data),identity,0,(identity,))]

    def insert(self,at,data,identity,provenance,lineage):
        output=[];shift=len(data)
        for p in self.pieces:
            if p.end<=at:output.append(p)
            elif p.start>=at:output.append(Piece(p.start+shift,p.end+shift,p.source,p.origin,p.lineage))
            else:
                output.append(Piece(p.start,at,p.source,p.origin,p.lineage))
                output.append(Piece(at+shift,p.end+shift,p.source,p.origin+at-p.start,p.lineage))
        if data:output.append(Piece(at,at+shift,identity,0,lineage+(identity,)))
        self.pieces=sorted(output,key=lambda p:p.start)
        self.sources[identity]=(data,provenance)
        self.data=self.data[:at]+data+self.data[at:]

    def locations(self,span):
        start,end=span['start'],span['end'];result=[]
        for p in self.pieces:
            a,b=max(start,p.start),min(end,p.end)
            if a<b or start==end and p.start<=start<=p.end:
                offset=p.origin+a-p.start;finish=p.origin+b-p.start
                data=self.sources[p.source][0]
                def position(n):
                    if p.source not in self.lines:self.lines[p.source]=[0]+[m.end() for m in re.finditer(rb'\r\n|\r|\n',data)]
                    lines=self.lines[p.source];index=bisect_right(lines,n)-1
                    return {'line':index+1,'column':n-lines[index]+1}
                result.append({'source':p.source,'start':offset,'end':finish,
                               'begin':position(offset),'finish':position(finish)})
                if start==end:break
        # Empty source / final EOF still has a meaningful file position.
        if not result and self.sources:
            identity=next(iter(self.sources));data=self.sources[identity][0]
            if not data:result=[{'source':identity,'start':0,'end':0,'begin':{'line':1,'column':1},'finish':{'line':1,'column':1}}]
        return result

    def metadata(self):
        return [{'identity':name,'size':len(data),'sha256':hashlib.sha256(data).hexdigest(),
                 'provenance':provenance} for name,(data,provenance) in sorted(self.sources.items())]


class InstallationIncludes:
    """Caller chooses the active ResourceStore profile and mission namespace."""
    def __init__(self,store,resource):
        from resource_store import virtual_name
        resource=virtual_name(resource)
        if '/' in resource:raise ValueError('Include resource must be one mission namespace')
        self.store,self.resource=store,resource

    def read(self,path):
        from resource_store import virtual_name
        name=virtual_name(path)
        resolution=self.store.resolution(name)
        if resolution.status=='missing':return None
        # read_result rejects invalid/unavailable selected members. Never fall back.
        try:result=self.store.read_result(name)
        except (OSError,ValueError) as exc:
            exc.include_target='resource/'+name
            exc.include_provenance={'resolution':asdict(resolution)}
            raise
        return result.data,'resource/'+name,{'resolution':asdict(resolution),'decoding':result.decoding}

    def source(self,identity):
        if not identity.startswith('resource/'):raise ValueError('Not an installation source identity')
        value=self.read(identity[9:])
        if value is None:raise ValueError('Selected source is missing')
        return value[0]

    def profile(self):return {'resource':self.resource,'resolution':self.store.profile()}


def expand(inspector,source,identity,context,lex_one):
    provenance=None
    if context and identity.startswith('resource/'):
        from dataclasses import asdict
        provenance={'resolution':asdict(context.store.resolution(identity[9:]))}
    mapping=SourceMap(source,identity,provenance);dependencies=[];diagnostics=[]
    cursor=0;scan=0
    def issue(category,message,where,end,severity='error',evidence='tool defensive restriction'):
        diagnostics.append({'category':category,'message':message,'severity':severity,'evidence':evidence,
                            'origin':'recovered_preprocessing' if category=='missing_include' else 'selected_resource_failure' if category=='include_selected_unavailable' else 'tool_restriction',
                            'span':{'start':where,'end':end},'span_space':'preprocessing_step','step':len(dependencies),'locations':mapping.locations({'start':where,'end':end})})
    if len(source)>65534 or b'\0' in source:
        # Native prefix reads/NUL truncation would hide bytes. Do not truncate.
        nul=source.find(b'\0');start=nul if nul>=0 else 65534;end=start+1 if nul>=0 else len(source)
        issue('source_read_restriction','Inspector rejects oversized input or embedded NUL instead of native prefix/truncation',start,end)
        return mapping,dependencies,diagnostics
    while True:
        where=mapping.data.find(b'#include',scan)
        if where<0:break
        after=where+8;scan=where
        if context is None:
            issue('include_context_required','Select an installation and mission namespace to resolve includes',where,after,'error','0042C580/0042C4B0')
            break
        if len(dependencies)>=128:
            issue('include_count_limit','Inspector include expansion limit (128)',where,after);break
        # Reject raw substring hazards, instead of silently imposing a C preprocessor.
        match=re.match(rb'#include[ \t]+("[^"\r\n\0]+")',mapping.data[where:])
        if not match or where and mapping.data[where-1:where] not in b' \t\r\n':
            issue('unsafe_include_directive','Malformed/unsupported raw include directive',where,after);break
        # Raw native search also sees directives inside comments/strings. Verify
        # the directive is preceded only by whitespace on its line.
        line=max(mapping.data.rfind(b'\n',0,where),mapping.data.rfind(b'\r',0,where))+1
        if mapping.data[line:where].strip():
            issue('unsafe_include_context','Raw directive inside another construct is not safely supported',where,after);break
        quoted=match[1];span={'start':where,'end':where+match.end()}
        origins=mapping.locations(span)
        piece=next(p for p in mapping.pieces if p.start<=where<p.end)
        mapping.data=mapping.data[:where]+b' '*9+mapping.data[after+1:]
        try:token=lex_one(inspector,mapping.data,cursor)
        except RuntimeError as exc:
            issue('include_lexer_rejected',str(exc),where,span['end'],'error','0042CC00 + defensive bounded lexer');break
        cursor=token['cursor']
        if token['token'].encode('latin-1')!=quoted or token['span']['start']!=where+match.start(1):
            issue('include_cursor_restriction','Original shared lexer cursor does not select this directive filename; inspection stops',where,span['end']);break
        if after+len(quoted)>=len(mapping.data):
            issue('include_bounds','Native blanking range exceeds source buffer',where,span['end'],'error','0042C4B0');break
        mapping.data=mapping.data[:after]+b' '*(len(quoted)+1)+mapping.data[after+len(quoted)+1:]
        name=quoted[1:-1].decode('latin-1')
        from resource_store import virtual_name
        try:
            # Native path construction is retained; traversal is rejected as a tool guard.
            paths=[virtual_name(f'Scripts/{context.resource}/{name}'),virtual_name(f'Scripts/include/{name}')]
        except ValueError as exc:
            issue('unsafe_include_path',str(exc),where,span['end']);break
        dep={'name':name,'span':span,'span_space':'preprocessing_step','step':len(dependencies)+1,'locations':origins,'attempts':[], 'selected':None,'status':'missing',
             'evidence':'0042C580/0042C4B0','expanded_at':where}
        dependencies.append(dep);found=None
        try:
            for path in paths:
                dep['attempts'].append(path)
                found=context.read(path)
                if found is not None:break
        except (OSError,ValueError) as exc:
            dep['status']='selected_resource_invalid' if getattr(exc,'kind','') in ('invalid','integrity_error','bounds_error') else 'selected_resource_unavailable'
            dep['selected']=getattr(exc,'include_target',None);dep['provenance']=getattr(exc,'include_provenance',None);dep['error']=str(exc)
            issue('include_selected_unavailable',str(exc),where,span['end'],'error','0042C580 + resource resolver');break
        if found is None:
            issue('missing_include','Original preprocessing blanks and ignores missing include: '+name,where,span['end'],'warning','0042C4B0');continue
        data,target,provenance=found;dep.update(selected=target,status='resolved',provenance=provenance)
        if target in piece.lineage or len(piece.lineage)>=32:
            dep['status']='rejected';issue('include_recursion','Inspector rejects recursive/depth>32 includes',where,span['end']);break
        if len(data)>65534 or b'\0' in data or len(mapping.data)+len(data)>MAX_SOURCE:
            dep['status']='rejected';issue('include_size_or_nul','Include exceeds read/expanded limit or contains embedded NUL',where,span['end']);break
        if target in mapping.sources and mapping.sources[target][0]!=data:
            dep['status']='rejected';issue('include_source_changed','Included source changed during this analysis',where,span['end']);break
        mapping.insert(where,data,target,provenance,piece.lineage)
        # Do NOT adjust the saved cursor for insertion (original behavior).
    return mapping,dependencies,diagnostics
