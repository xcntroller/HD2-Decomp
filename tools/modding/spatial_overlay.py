"""Bounded static relationship annotations; never runtime action rendering."""
import ctypes as C
import math
from gl_view import bind

STYLES={'serialized':((.35,.85,.95),0xffff),'resolver_selected':((.4,.9,.5),0x3f3f),
        'candidate':((1.,.62,.18),0x00ff),'unresolved':((.8,.45,.8),0x1111)}


class RelationshipOverlay:
    def __init__(self,viewport):
        self.viewport=viewport;self.edges=[];self.selected=None;self.labels=False;self.bound=False;self.font=0
        self.label_status='not_requested'
    def bind(self):
        if self.bound:return
        g=self.viewport.gl
        for name,result,args in (
            ('glLineStipple',None,(C.c_int,C.c_ushort)),('glRasterPos3f',None,(C.c_float,C.c_float,C.c_float)),
            ('glGenLists',C.c_uint,(C.c_int,)),('glDeleteLists',None,(C.c_uint,C.c_int)),
            ('glListBase',None,(C.c_uint,)),('glCallLists',None,(C.c_int,C.c_uint,C.c_void_p)),
            ('glXUseXFont',None,(C.c_ulong,C.c_int,C.c_int,C.c_int))):bind(g,name,result,*args)
        bind(self.viewport.x,'XLoadFont',C.c_ulong,C.c_void_p,C.c_char_p)
        bind(self.viewport.x,'XUnloadFont',C.c_int,C.c_void_p,C.c_ulong)
        self.bound=True
    def font_ready(self):
        if self.font:return True
        v=self.viewport;g=v.gl
        font=v.x.XLoadFont(v.display,b'fixed');base=g.glGenLists(96)
        if not font or not base:
            if font:v.x.XUnloadFont(v.display,font)
            self.label_status='unavailable';return False
        g.glXUseXFont(font,32,96,base);v.x.XUnloadFont(v.display,font)
        self.font=base;self.label_status='fixed_X11_font';return True
    def valid_edges(self):
        v=self.viewport
        if not v.scene:return []
        return [e for e in self.edges[:96] if e.source in v.scene.nodes and e.target in v.scene.nodes
            and v.scene.nodes[e.source].world is not None and v.scene.nodes[e.target].world is not None]
    def draw(self,picking=False):
        edges=self.valid_edges()
        if not edges:return
        self.bind();v=self.viewport;g=v.gl
        #X-ray annotation is explicit; hiding behind surfaces would imply false
        #absence. Never used as an object's placement or geometry substitute.
        g.glDisable(0x0b71);g.glDisable(0x0b50);g.glDisable(0x0de1);g.glDisable(0x0be2);g.glDisable(0x0bc0)
        for n,e in enumerate(edges):
            a=v.scene.nodes[e.source].world[12:15];b=v.scene.nodes[e.target].world[12:15]
            color,pattern=STYLES.get(e.certainty,STYLES['unresolved'])
            selected=e.identity==self.selected
            if picking:
                index=len(v.draw_list)+n+1;g.glColor4ub(index&255,(index>>8)&255,(index>>16)&255,255)
                g.glDisable(0x0b24);g.glLineWidth(7)
            else:
                g.glColor4f(*((1.,.95,.3) if selected else color),1)
                g.glEnable(0x0b24);g.glLineStipple(1,pattern);g.glLineWidth(3 if selected else 1.5)
            g.glBegin(1);g.glVertex3f(*a);g.glVertex3f(*b)
            direction=[b[k]-a[k] for k in range(3)];length=math.sqrt(sum(x*x for x in direction))
            if length>.001:
                _,right,up=v.camera.basis();size=min(1.,length*.12)
                #Billboard head indicates reference direction only.
                side=[direction[1]*up[2]-direction[2]*up[1],direction[2]*up[0]-direction[0]*up[2],direction[0]*up[1]-direction[1]*up[0]]
                norm=math.sqrt(sum(x*x for x in side));side=[x/norm for x in side] if norm>1e-8 else right
                for sign in (-1,1):
                    g.glVertex3f(*b);g.glVertex3f(*(b[k]-direction[k]/length*size+side[k]*size*.4*sign for k in range(3)))
            g.glEnd()
            if not picking and selected:
                g.glDisable(0x0b24)
                for identity in (e.source,e.target):v.box(v.scene.nodes[identity].box)
            if not picking and self.labels and self.font_ready():
                raw=(str(n+1)+' '+e.label).encode('ascii','replace')[:96]
                text=bytes(c if 32<=c<127 else ord('?') for c in raw)
                g.glRasterPos3f(*((a[k]+b[k])*.5 for k in range(3)))
                g.glListBase(self.font-32);g.glCallLists(len(text),0x1401,C.c_char_p(text))
        g.glDisable(0x0b24);g.glLineWidth(1);g.glEnable(0x0b71)
    def picked(self,index):
        edges=self.valid_edges();n=index-len(self.viewport.draw_list)-1
        return edges[n] if 0<=n<len(edges) else None
    def close(self):
        if self.font:self.viewport.gl.glDeleteLists(self.font,96);self.font=0
        self.edges=[];self.selected=None
