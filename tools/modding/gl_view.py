"""Small OpenGL/GLX viewport embedded as a child of the existing Tk window.

Uses system libGL/libX11 through explicit ctypes ABIs (Linux X11/XWayland).
No engine, extra event loop or third-party Python rendering package is required.
"""
import ctypes as C
from ctypes.util import find_library
import math
import tkinter as tk


class VisualInfo(C.Structure):
    _fields_=[('visual',C.c_void_p),('visualid',C.c_ulong),('screen',C.c_int),('depth',C.c_int),('class_',C.c_int),
              ('red_mask',C.c_ulong),('green_mask',C.c_ulong),('blue_mask',C.c_ulong),('colormap_size',C.c_int),('bits_per_rgb',C.c_int)]


class WindowAttributes(C.Structure):
    _fields_=[('background_pixmap',C.c_ulong),('background_pixel',C.c_ulong),('border_pixmap',C.c_ulong),('border_pixel',C.c_ulong),
              ('bit_gravity',C.c_int),('win_gravity',C.c_int),('backing_store',C.c_int),('backing_planes',C.c_ulong),
              ('backing_pixel',C.c_ulong),('save_under',C.c_int),('event_mask',C.c_long),('do_not_propagate_mask',C.c_long),
              ('override_redirect',C.c_int),('colormap',C.c_ulong),('cursor',C.c_ulong)]


class PointerEvent(C.Structure):
    _fields_=[('type',C.c_int),('serial',C.c_ulong),('send_event',C.c_int),('display',C.c_void_p),('window',C.c_ulong),
              ('root',C.c_ulong),('subwindow',C.c_ulong),('time',C.c_ulong),('x',C.c_int),('y',C.c_int),
              ('x_root',C.c_int),('y_root',C.c_int),('state',C.c_uint),('button',C.c_uint),('same_screen',C.c_int)]


class Event(C.Union):
    _fields_=[('type',C.c_int),('pointer',PointerEvent),('pad',C.c_long*24)]


def bind(lib,name,result,*arguments):
    function=getattr(lib,name);function.restype=result;function.argtypes=list(arguments);return function


class GLViewport(tk.Frame):
    def __init__(self,parent,on_error=None,**kwargs):
        super().__init__(parent,background='#17212c',**kwargs)
        self.on_error=on_error;self.display=None;self.context=None;self.window=0;self.colormap=0
        self.timer=None;self.closed=False;self.dirty=True;self.ready=False;self.model=None;self.selected=None
        self.wire=False;self.grid=True;self.axes=True;self.boxes=True;self.textured=True;self.lod_index=0
        self.center=[0.,0.,0.];self.radius=1.;self.distance=3.;self.yaw=35.;self.pitch=22.;self.pan=[0.,0.]
        self.drag=None;self.packets=[];self.texture_ids={};self.backend='OpenGL not initialized'
        self.bind('<Map>',self.initialize);self.bind('<Configure>',self.resize)
        self.bind('<Destroy>',lambda event:self.close(False) if event.widget is self else None)

    def initialize(self,_=None):
        if self.ready or self.closed:return
        try:
            if self.tk.call('tk','windowingsystem')!='x11':raise RuntimeError('Viewport currently requires X11 or XWayland')
            self.gl=C.CDLL(find_library('GL') or 'libGL.so.1');self.x=C.CDLL(find_library('X11') or 'libX11.so.6')
            ptr=C.c_void_p;ul=C.c_ulong;i=C.c_int;ui=C.c_uint;f=C.c_float;d=C.c_double
            specs={
                'XOpenDisplay':(ptr,C.c_char_p),'XDefaultScreen':(i,ptr),'XCreateColormap':(ul,ptr,ul,ptr,i),
                'XCreateWindow':(ul,ptr,ul,i,i,ui,ui,ui,i,ui,ptr,ul,C.POINTER(WindowAttributes)),
                'XMapWindow':(i,ptr,ul),'XResizeWindow':(i,ptr,ul,ui,ui),'XDestroyWindow':(i,ptr,ul),
                'XFreeColormap':(i,ptr,ul),'XCloseDisplay':(i,ptr),'XFree':(i,ptr),'XFlush':(i,ptr),
                'XPending':(i,ptr),'XNextEvent':(i,ptr,C.POINTER(Event))}
            for name,signature in specs.items():bind(self.x,name,*signature)
            specs={
                'glXChooseVisual':(C.POINTER(VisualInfo),ptr,i,C.POINTER(i)),
                'glXCreateContext':(ptr,ptr,C.POINTER(VisualInfo),ptr,i),'glXMakeCurrent':(i,ptr,ul,ptr),
                'glXDestroyContext':(None,ptr,ptr),'glXSwapBuffers':(None,ptr,ul),
                'glGetString':(C.c_char_p,ui),'glGetError':(ui,),
                'glViewport':(None,i,i,i,i),'glClearColor':(None,f,f,f,f),'glClear':(None,ui),
                'glEnable':(None,ui),'glDisable':(None,ui),'glMatrixMode':(None,ui),'glLoadIdentity':(None,),
                'glFrustum':(None,d,d,d,d,d,d),'glTranslatef':(None,f,f,f),'glRotatef':(None,f,f,f,f),
                'glMultMatrixf':(None,C.POINTER(f)),'glPushMatrix':(None,),'glPopMatrix':(None,),
                'glColor4f':(None,f,f,f,f),'glPolygonMode':(None,ui,ui),'glLineWidth':(None,f),
                'glBegin':(None,ui),'glEnd':(None,),'glVertex3f':(None,f,f,f),
                'glEnableClientState':(None,ui),'glDisableClientState':(None,ui),
                'glVertexPointer':(None,i,ui,i,ptr),'glNormalPointer':(None,ui,i,ptr),
                'glTexCoordPointer':(None,i,ui,i,ptr),'glDrawElements':(None,ui,i,ui,ptr),
                'glLightfv':(None,ui,ui,C.POINTER(f)),'glColorMaterial':(None,ui,ui),
                'glGenTextures':(None,i,C.POINTER(ui)),'glDeleteTextures':(None,i,C.POINTER(ui)),
                'glBindTexture':(None,ui,ui),'glTexParameteri':(None,ui,ui,i),'glPixelStorei':(None,ui,i),
                'glTexImage2D':(None,ui,i,i,i,i,i,ui,ui,ptr),'glReadPixels':(None,i,i,i,i,ui,ui,ptr),
                'glReadBuffer':(None,ui),'glFinish':(None,)}
            for name,signature in specs.items():bind(self.gl,name,*signature)
            self.display=self.x.XOpenDisplay(None)
            if not self.display:raise RuntimeError('Cannot open the X11 display')
            attrs=(i*11)(4,5,8,8,9,8,10,8,12,24,0) # RGBA,double buffer,RGB8,depth24
            vi=self.gl.glXChooseVisual(self.display,self.x.XDefaultScreen(self.display),attrs)
            if not vi:raise RuntimeError('No compatible GLX visual (RGBA/depth/double buffer)')
            try:
                self.context=self.gl.glXCreateContext(self.display,vi,None,1)
                if not self.context:raise RuntimeError('Cannot create OpenGL context')
                self.colormap=self.x.XCreateColormap(self.display,self.winfo_id(),vi.contents.visual,0)
                wa=WindowAttributes();wa.colormap=self.colormap;wa.border_pixel=0
                wa.event_mask=(1<<15)|(1<<17)|(1<<2)|(1<<3)|(1<<6)|getattr(self,'extra_event_mask',0)
                self.window=self.x.XCreateWindow(self.display,self.winfo_id(),0,0,max(self.winfo_width(),1),max(self.winfo_height(),1),
                    0,vi.contents.depth,1,vi.contents.visual,(1<<3)|(1<<11)|(1<<13),C.byref(wa))
            finally:self.x.XFree(vi)
            if not self.window or not self.gl.glXMakeCurrent(self.display,self.window,self.context):raise RuntimeError('Cannot attach GLX context to viewport')
            self.x.XMapWindow(self.display,self.window);self.x.XFlush(self.display)
            version=self.gl.glGetString(0x1f02);renderer=self.gl.glGetString(0x1f01)
            if not version:raise RuntimeError('OpenGL context returned no version')
            self.backend=f"OpenGL {version.decode(errors='replace')} · {renderer.decode(errors='replace') if renderer else 'unknown renderer'}"
            self.ready=True
            self.upload_textures(getattr(self,'images',{}))
            self.dirty=True;self.tick()
        except (OSError,RuntimeError) as exc:
            self.close();self.backend='Viewport unavailable: '+str(exc)
            tk.Label(self,text=self.backend,fg='white',bg='#17212c',wraplength=450).pack(expand=True)
            if self.on_error:self.on_error(self.backend)

    def resize(self,_=None):
        if self.ready:
            self.x.XResizeWindow(self.display,self.window,max(1,self.winfo_width()),max(1,self.winfo_height()));self.x.XFlush(self.display)
        self.dirty=True

    def set_model(self,model,images=None):
        if self.closed:return
        self.model=model;self.images=images or {};self.selected=None;self.packets=[]
        # Keep packed arrays alive for client pointers. No Python tuple per vertex.
        for geometry in model.geometries:
            for lod in geometry.lods:
                if lod.vertices:self.packets.append(lod)
        if self.ready:self.upload_textures(self.images)
        self.fit();self.dirty=True

    def upload_textures(self,images):
        if not self.ready:return
        if not self.gl.glXMakeCurrent(self.display,self.window,self.context):raise RuntimeError('OpenGL context unavailable')
        self.release_textures()
        self.gl.glPixelStorei(0x0cf5,1)
        uploaded={}
        for identity,image in images.items():
            key=id(image)
            if key in uploaded:self.texture_ids[identity]=uploaded[key];continue
            texture=C.c_uint();self.gl.glGenTextures(1,C.byref(texture));self.texture_ids[identity]=texture.value;uploaded[key]=texture.value
            self.gl.glBindTexture(0x0de1,texture.value)
            for key,value in ((0x2801,0x2601),(0x2800,0x2601),(0x2802,0x2901),(0x2803,0x2901)):
                self.gl.glTexParameteri(0x0de1,key,value)
            self.gl.glTexImage2D(0x0de1,0,0x1907,image.width,image.height,0,0x1907,0x1401,image.pixels)
        self.gl.glBindTexture(0x0de1,0)

    def release_textures(self):
        ids=sorted(set(self.texture_ids.values()))
        if ids:self.gl.glDeleteTextures(len(ids),(C.c_uint*len(ids))(*ids))
        self.texture_ids={}

    def fit(self):
        box=self.model.bounds if self.model else None
        self.center=[(box[0][k]+box[1][k])*.5 for k in range(3)] if box else [0.,0.,0.]
        self.radius=max(math.dist(box[0],box[1])*.5,1e-4) if box else 1.
        self.distance=self.radius*3.3;self.pan=[0.,0.];self.dirty=True

    def pointer(self,event):
        p=event.pointer
        if event.type==4:
            if p.button in (4,5):self.distance=max(self.radius*.02,min(self.radius*1000,self.distance*(.88 if p.button==4 else 1/.88)))
            else:self.drag=(p.button,p.x,p.y)
        elif event.type==5:self.drag=None
        elif event.type==6 and self.drag:
            button,x,y=self.drag;dx=p.x-x;dy=p.y-y;self.drag=(button,p.x,p.y)
            if button==1 and not p.state&1:
                self.yaw=(self.yaw+dx*.45)%360;self.pitch=max(-89.,min(89.,self.pitch+dy*.45))
            else:
                factor=self.distance/max(self.winfo_height(),1)
                self.pan[0]+=dx*factor;self.pan[1]-=dy*factor
        self.dirty=True

    def tick(self):
        self.timer=None
        if self.closed or not self.ready:return
        try:
            for _ in range(min(self.x.XPending(self.display),256)):
                event=Event();self.x.XNextEvent(self.display,C.byref(event))
                if event.type in (4,5,6):self.pointer(event)
                else:self.dirty=True
            if self.dirty and self.winfo_ismapped():self.draw();self.dirty=False
        except (tk.TclError,RuntimeError) as exc:
            if self.on_error:self.on_error(str(exc))
            self.close();return
        self.timer=self.after(20,self.tick)

    def box(self,box):
        if not box:return
        low,high=box
        self.gl.glBegin(1)
        for bits in range(8):
            for k in range(3):
                if bits&(1<<k):continue
                for mask in (bits,bits|(1<<k)):
                    self.gl.glVertex3f(*(high[j] if mask&(1<<j) else low[j] for j in range(3)))
        self.gl.glEnd()

    def draw(self):
        if not self.gl.glXMakeCurrent(self.display,self.window,self.context):raise RuntimeError('OpenGL make-current failed')
        g=self.gl;w=max(self.winfo_width(),1);h=max(self.winfo_height(),1)
        g.glViewport(0,0,w,h);g.glClearColor(.075,.10,.14,1.);g.glClear(0x4100)
        g.glEnable(0x0b71);g.glDisable(0x0b44);g.glDisable(0x0be2)
        g.glMatrixMode(0x1701);g.glLoadIdentity();near=max(self.radius*.001,self.distance*.001);far=self.distance+self.radius*100
        top=near*math.tan(math.radians(40)/2);g.glFrustum(-top*w/h,top*w/h,-top,top,near,far)
        g.glMatrixMode(0x1700);g.glLoadIdentity();g.glTranslatef(self.pan[0],self.pan[1],-self.distance)
        g.glRotatef(self.pitch,1,0,0);g.glRotatef(self.yaw,0,1,0);g.glTranslatef(*(-c for c in self.center))
        g.glDisable(0x0b50);g.glDisable(0x0de1);g.glPolygonMode(0x0408,0x1b02);g.glLineWidth(1)
        if self.grid:
            step=self.radius/5;ground=self.model.bounds[0][1] if self.model and self.model.bounds else 0
            g.glColor4f(.22,.27,.31,1);g.glBegin(1)
            for i in range(-10,11):
                for p in ((i*step,ground,-10*step),(i*step,ground,10*step),(-10*step,ground,i*step),(10*step,ground,i*step)):g.glVertex3f(*p)
            g.glEnd()
        if self.axes:
            g.glBegin(1)
            for k,color in enumerate(((1,.3,.3),(.3,1,.3),(.3,.5,1))):
                g.glColor4f(*color,1);g.glVertex3f(0,0,0);g.glVertex3f(*(self.radius if j==k else 0 for j in range(3)))
            g.glEnd()
        if self.model:
            g.glEnable(0x0b50);g.glEnable(0x4000);g.glEnable(0x0ba1);g.glEnable(0x0b57)
            g.glLightfv(0x4000,0x1203,(C.c_float*4)(.5,1.,1.,0.));g.glLightfv(0x4000,0x1201,(C.c_float*4)(.8,.8,.8,1.))
            g.glColorMaterial(0x0408,0x1602)
            for state in (0x8074,0x8075,0x8078):g.glEnableClientState(state)
            g.glPolygonMode(0x0408,0x1b01 if self.wire else 0x1b02)
            for frame in self.model.frames:
                if not frame.geometry or not frame.geometry.lods:continue
                lod=frame.geometry.lods[min(self.lod_index,len(frame.geometry.lods)-1)]
                if not lod.vertex_count:continue
                address=lod.vertices.buffer_info()[0]
                g.glVertexPointer(3,0x1406,32,address);g.glNormalPointer(0x1406,32,address+12);g.glTexCoordPointer(2,0x1406,32,address+24)
                g.glPushMatrix();g.glMultMatrixf((C.c_float*16)(*frame.world_matrix))
                for group in lod.groups:
                    texture=self.texture_ids.get(group.material) if self.textured and not self.wire else None
                    selected=self.selected==frame.identity
                    if texture and not selected:g.glEnable(0x0de1);g.glBindTexture(0x0de1,texture)
                    else:g.glDisable(0x0de1)
                    g.glColor4f(*((1.,.62,.12,1.) if selected else (1.,1.,1.,1.) if texture else (.65,.70,.76,1.)))
                    g.glDrawElements(4,len(group.indices),0x1403,group.indices.buffer_info()[0])
                g.glPopMatrix()
            for state in (0x8074,0x8075,0x8078):g.glDisableClientState(state)
            g.glDisable(0x0b50);g.glDisable(0x0de1);g.glPolygonMode(0x0408,0x1b02)
            frame=next((f for f in self.model.frames if f.identity==self.selected),None)
            if frame:
                # A transform-origin marker also makes non-mesh frames selectable;
                # it is a viewer annotation, never substitute model geometry.
                point=frame.world_matrix[12:15];size=self.radius*.045
                g.glColor4f(1,.65,.1,1);g.glBegin(1)
                for axis in range(3):
                    for sign in (-1,1):g.glVertex3f(*(point[k]+(sign*size if k==axis else 0) for k in range(3)))
                g.glEnd()
            if self.boxes:
                g.glColor4f(.5,.65,.8,1);self.box(self.model.bounds)
                if frame:g.glColor4f(1,.65,.1,1);self.box(frame.world_bounds)
        error=g.glGetError()
        if error:raise RuntimeError(f'OpenGL error0x{error:04x}')
        g.glXSwapBuffers(self.display,self.window)

    def screenshot(self):
        """Return RGB bytes for explicit visual audits, never persisted by the app."""
        self.draw();self.gl.glFinish();self.gl.glReadBuffer(0x0404);self.gl.glPixelStorei(0x0d05,1)
        w,h=max(1,self.winfo_width()),max(1,self.winfo_height());pixels=(C.c_ubyte*(w*h*3))()
        self.gl.glReadPixels(0,0,w,h,0x1907,0x1401,pixels)
        return w,h,bytes(pixels)

    def close(self,destroy_window=True):
        if self.closed:return
        self.closed=True;self.ready=False
        if self.timer:
            try:self.after_cancel(self.timer)
            except tk.TclError:pass
            self.timer=None
        if self.display:
            if self.context:
                self.gl.glXMakeCurrent(self.display,0,None);self.gl.glXDestroyContext(self.display,self.context);self.context=None
            if self.window and destroy_window:self.x.XDestroyWindow(self.display,self.window)
            if self.colormap:self.x.XFreeColormap(self.display,self.colormap)
            self.x.XCloseDisplay(self.display);self.display=None
        self.window=0;self.texture_ids={};self.packets=[];self.model=None;self.images={}
