"""Multi-object mission viewport sharing the toolkit GLX infrastructure."""
import ctypes as C
import math
import time
import tkinter as tk

from gl_view import GLViewport,Event,PointerEvent,bind
from spatial_camera import Camera
from gl_material import apply as apply_material,reset as reset_material,blended

MAX_GPU_MESHES=8000
MAX_DRAWS=6000
CATEGORIES=('environment','props','actors','spawn','lights','frames','sectors')


class MissionViewport(GLViewport):
    def __init__(self,parent,on_select=None,on_stats=None,on_error=None,**kwargs):
        super().__init__(parent,on_error,**kwargs)
        self.extra_event_mask=1|2|(1<<21)
        self.scene=None;self.camera=Camera();self.on_select=on_select;self.on_stats=on_stats
        self.categories={'environment','props','actors','spawn','lights'};self.show_hidden=False
        self.origins=False
        self.keys=set();self.last_tick=time.monotonic();self.bound=False;self.buffers={};self.gpu_ready=False
        self.mouse_start=None;self.mouse_moved=False;self.draw_list=[];self.stats={};self.last_stats=0.
        from spatial_overlay import RelationshipOverlay
        self.relationships=RelationshipOverlay(self);self.on_relationship=None

    def extra_gl(self):
        if self.bound:return
        ptr=C.c_void_p;ui=C.c_uint;i=C.c_int
        for name,signature in {
            'glGenBuffers':(None,i,C.POINTER(ui)),'glDeleteBuffers':(None,i,C.POINTER(ui)),
            'glBindBuffer':(None,ui,ui),'glBufferData':(None,ui,C.c_ssize_t,ptr,ui),
            'glColor4ub':(None,C.c_ubyte,C.c_ubyte,C.c_ubyte,C.c_ubyte),'glPointSize':(None,C.c_float)
        }.items():bind(self.gl,name,*signature)
        bind(self.x,'XSetInputFocus',i,ptr,C.c_ulong,i,C.c_ulong)
        bind(self.x,'XLookupKeysym',C.c_ulong,C.POINTER(PointerEvent),i)
        self.bound=True

    def release_buffers(self):
        values=[v for packet in self.buffers.values() for v in [packet[0]]+[g[0] for g in packet[1]]]
        if values:self.gl.glDeleteBuffers(len(values),(C.c_uint*len(values))(*values))
        self.buffers={};self.gpu_ready=False

    def set_scene(self,scene):
        if self.closed:return
        if self.ready:
            self.extra_gl();self.gl.glXMakeCurrent(self.display,self.window,self.context);self.release_buffers()
        self.scene=scene;self.selected=None;self.keys.clear();self.draw_list=[]
        self.relationships.edges=[];self.relationships.selected=None
        self.images={(path,index):image for path,asset in scene.assets.items() for index,image in asset.images.items()}
        if self.ready:self.upload_textures(self.images)
        self.camera=Camera();self.camera.focus(scene.bounds);self.camera.far=max(500.,self.camera.distance*2.)
        self.camera.speed=max(2.,min(60.,self.camera.distance*.03));self.dirty=True

    def upload_buffers(self):
        if self.gpu_ready or not self.scene:return
        g=self.gl
        def buffer(target,array):
            name=C.c_uint();g.glGenBuffers(1,C.byref(name));g.glBindBuffer(target,name.value)
            g.glBufferData(target,len(array)*array.itemsize,array.buffer_info()[0],0x88e4);return name.value
        for asset in self.scene.assets.values():
            for geometry in asset.model.geometries:
                if len(self.buffers)>=MAX_GPU_MESHES:break
                if not geometry.lods or not geometry.lods[0].vertex_count:continue
                lod=geometry.lods[0]
                if not any(group.triangle_count for group in lod.groups):continue
                vertex=buffer(0x8892,lod.vertices);groups=[]
                self.buffers[geometry.identity]=(vertex,groups)
                for group in lod.groups:
                    if group.triangle_count:groups.append((buffer(0x8893,group.indices),len(group.indices),group.material))
        g.glBindBuffer(0x8892,0);g.glBindBuffer(0x8893,0);self.gpu_ready=True

    def fit(self):
        if self.scene:self.camera.focus(self.scene.bounds);self.dirty=True

    def focus(self,identity=None):
        node=self.scene.nodes.get(identity or self.selected) if self.scene else None
        if node and node.world is not None:self.camera.focus(node.box,node.world[12:15]);self.dirty=True

    def tick(self):
        self.timer=None
        if self.closed or not self.ready:return
        try:
            self.extra_gl();now=time.monotonic();dt=now-self.last_tick;self.last_tick=now
            for _ in range(min(self.x.XPending(self.display),256)):
                event=Event();self.x.XNextEvent(self.display,C.byref(event))
                if event.type in (2,3):
                    sym=self.x.XLookupKeysym(C.byref(event.pointer),0)
                    name=chr(sym).lower() if 32<=sym<127 else 'shift' if sym in (0xffe1,0xffe2) else ''
                    if event.type==2:
                        if name=='f':self.focus()
                        self.keys.add(name)
                    else:self.keys.discard(name)
                elif event.type==10:self.keys.clear()
                elif event.type in (4,5,6):self.pointer(event)
                else:self.dirty=True
            if self.camera.move(self.keys,dt):self.dirty=True
            if self.dirty and self.winfo_ismapped():self.draw();self.dirty=False
        except (tk.TclError,RuntimeError) as exc:
            if self.on_error:self.on_error(str(exc))
            self.close();return
        self.timer=self.after(16,self.tick)

    def pointer(self,event):
        p=event.pointer
        if event.type==4:
            self.x.XSetInputFocus(self.display,self.window,2,0)
            if p.button in (4,5):
                if self.camera.mode=='orbit':self.camera.distance=max(.1,min(100000,self.camera.distance*(.85 if p.button==4 else 1/.85)))
                else:self.camera.speed=max(.1,min(1000,self.camera.speed*(1.15 if p.button==4 else 1/1.15)))
            else:self.drag=(p.button,p.x,p.y);self.mouse_start=(p.x,p.y);self.mouse_moved=False
        elif event.type==6 and self.drag:
            button,x,y=self.drag;dx,dy=p.x-x,p.y-y;self.drag=(button,p.x,p.y)
            if self.mouse_start and math.dist((p.x,p.y),self.mouse_start)>3:self.mouse_moved=True
            if button==3 or self.camera.mode=='orbit' and button==1:self.camera.look(dx,dy)
            elif button==2:
                _,right,up=self.camera.basis();distance=self.camera.distance if self.camera.mode=='orbit' else self.camera.speed
                delta=[(-dx*right[k]+dy*up[k])*distance/max(self.winfo_height(),1) for k in range(3)]
                if self.camera.mode=='orbit':self.camera.target=[self.camera.target[k]+delta[k] for k in range(3)]
                else:self.camera.position=[self.camera.position[k]+delta[k] for k in range(3)]
        elif event.type==5:
            if p.button==1 and self.drag and not self.mouse_moved:self.pick(p.x,p.y)
            self.drag=None
        self.dirty=True

    def selection(self,identity):
        self.selected=identity;self.dirty=True
        if self.on_select:self.on_select(identity)

    def candidates(self):
        self.stats.update(culled=0,filtered=0,draw_budget_omissions=0)
        if not self.scene:return []
        aspect=max(self.winfo_width(),1)/max(self.winfo_height(),1);result=[];culled=hidden=limited=0
        for identity in self.scene.order:
            node=self.scene.nodes[identity]
            if node.world is None:continue
            if not node.geometry and not (self.origins or node.category in ('lights','spawn') or node.actors):hidden+=1;continue
            if node.category not in self.categories or not self.show_hidden and not node.display_visible:hidden+=1;continue
            if not self.camera.in_view(node.box,node.world[12:15],aspect):culled+=1;continue
            if len(result)>=MAX_DRAWS:limited+=1;continue
            result.append(node)
        self.stats.update(culled=culled,filtered=hidden,draw_budget_omissions=limited)
        return result

    def setup_camera(self):
        g=self.gl;w,h=max(self.winfo_width(),1),max(self.winfo_height(),1)
        g.glDepthMask(1);g.glViewport(0,0,w,h);g.glClearColor(.065,.085,.11,1);g.glClear(0x4100)
        g.glEnable(0x0b71);g.glDisable(0x0b44);g.glDisable(0x0be2)
        g.glMatrixMode(0x1701);g.glLoadIdentity();near=.05;top=near*math.tan(math.radians(60)*.5)
        g.glFrustum(-top*w/h,top*w/h,-top,top,near,max(self.camera.far,near+1))
        g.glMatrixMode(0x1700);g.glLoadIdentity()
        g.glMultMatrixf((C.c_float*16)(*self.camera.view_matrix()))

    def geometry(self,node,pick_index=None,blend_pass=None):
        packet=self.buffers.get(node.geometry.identity) if node.geometry else None
        if packet is None:return 0
        g=self.gl;g.glBindBuffer(0x8892,packet[0])
        g.glVertexPointer(3,0x1406,32,None);g.glNormalPointer(0x1406,32,12);g.glTexCoordPointer(2,0x1406,32,24)
        g.glPushMatrix();g.glMultMatrixf((C.c_float*16)(*node.world));triangles=0
        for ibo,count,material in packet[1]:
            state=self.scene.assets[node.asset].material_preview.get(material,{})
            if pick_index is not None and state.get('opacity')==0:continue
            if blend_pass is not None and blended(state)!=blend_pass:continue
            texture=self.texture_ids.get((node.asset,material)) if self.textured and not self.wire else None
            selected=self.selected is not None and (node.identity==self.selected or node.model_owner==self.selected)
            apply_material(g,state,texture,selected,pick_index is not None)
            g.glBindBuffer(0x8893,ibo);g.glDrawElements(4,count,0x1403,None);triangles+=count//3
        g.glPopMatrix();return triangles

    def render_objects(self,picking=False):
        g=self.gl;triangles=0;meshes=0
        for state in (0x8074,0x8075,0x8078):g.glEnableClientState(state)
        if picking:
            for index,node in enumerate(self.draw_list,1):
                g.glColor4ub(index&255,(index>>8)&255,(index>>16)&255,255)
                if node.geometry:triangles+=self.geometry(node,index);meshes+=1
        else:
            for node in self.draw_list:
                if node.geometry:triangles+=self.geometry(node,blend_pass=False);meshes+=1
            eye=self.camera.eye()
            #Preview approximation: back-to-front instance sort, not original
            #sector/triangle ordering. Deterministic identity breaks distance ties.
            transparent=[node for node in self.draw_list if node.geometry and any(blended(self.scene.assets[node.asset].material_preview.get(g.material,{})) for g in node.geometry.lods[0].groups)]
            transparent.sort(key=lambda n:(-math.dist(eye,n.world[12:15]),n.identity))
            for node in transparent:triangles+=self.geometry(node,blend_pass=True)
        reset_material(g)
        g.glBindBuffer(0x8892,0);g.glBindBuffer(0x8893,0)
        for state in (0x8074,0x8075,0x8078):g.glDisableClientState(state)
        g.glDisable(0x0b50);g.glDisable(0x0de1)
        g.glPointSize(9 if picking else 5);g.glBegin(0)
        for index,node in enumerate(self.draw_list,1):
            if node.geometry:continue
            if picking:g.glColor4ub(index&255,(index>>8)&255,(index>>16)&255,255)
            else:g.glColor4f(*((1,.6,.15,1) if node.identity==self.selected else (.4,.8,1,1)))
            g.glVertex3f(*node.world[12:15])
        g.glEnd();return meshes,triangles

    def pick(self,x,y):
        if not self.scene or not self.ready:return
        self.extra_gl();self.gl.glXMakeCurrent(self.display,self.window,self.context);self.upload_buffers();self.setup_camera()
        g=self.gl
        for state in (0x0b50,0x0de1,0x0bd0,0x809d,0x8db9):g.glDisable(state)
        g.glPolygonMode(0x0408,0x1b02);g.glClearColor(0,0,0,1);g.glClear(0x4100)
        self.draw_list=self.candidates();self.render_objects(True);self.relationships.draw(True);g.glFinish();g.glReadBuffer(0x0405)
        pixel=(C.c_ubyte*3)();g.glPixelStorei(0x0d05,1)
        if 0<=x<self.winfo_width() and 0<=y<self.winfo_height():
            g.glReadPixels(x,self.winfo_height()-1-y,1,1,0x1907,0x1401,pixel)
            index=pixel[0]+(pixel[1]<<8)+(pixel[2]<<16)
            edge=self.relationships.picked(index)
            if edge:
                self.relationships.selected=edge.identity
                if self.on_relationship:self.on_relationship(edge)
            else:self.selection(self.draw_list[index-1].identity if 0<index<=len(self.draw_list) else None)
        self.dirty=True

    def draw(self):
        self.extra_gl();g=self.gl
        if not g.glXMakeCurrent(self.display,self.window,self.context):raise RuntimeError('Mission GL context unavailable')
        start=time.perf_counter();self.upload_buffers();self.setup_camera();self.draw_list=self.candidates()
        g.glPolygonMode(0x0408,0x1b01 if self.wire else 0x1b02)
        g.glEnable(0x0b50);g.glEnable(0x4000);g.glEnable(0x0ba1);g.glEnable(0x0b57)
        g.glLightfv(0x4000,0x1203,(C.c_float*4)(.5,1,.8,0));g.glLightfv(0x4000,0x1201,(C.c_float*4)(.85,.85,.85,1))
        g.glColorMaterial(0x0408,0x1602)
        meshes,triangles=self.render_objects();g.glPolygonMode(0x0408,0x1b02)
        g.glLineWidth(1)
        if self.grid:
            # Explicit aid at Y=0. It is never presented as decoded terrain.
            g.glColor4f(.22,.25,.28,1);g.glBegin(1)
            for n in range(-10,11):
                for point in ((n*10,0,-100),(n*10,0,100),(-100,0,n*10),(100,0,n*10)):g.glVertex3f(*point)
            g.glEnd()
        if self.axes:
            g.glBegin(1)
            for axis,color in enumerate(((1,.3,.3),(.3,1,.3),(.3,.5,1))):
                g.glColor4f(*color,1);g.glVertex3f(0,0,0);g.glVertex3f(*(10 if k==axis else 0 for k in range(3)))
            g.glEnd()
        node=self.scene.nodes.get(self.selected) if self.scene else None
        if node and node.world is not None:
            g.glColor4f(1,.65,.1,1)
            if self.boxes:self.box(node.box)
            point=node.world[12:15];g.glBegin(1)
            for axis in range(3):
                for sign in (-1,1):g.glVertex3f(*(point[k]+(sign*.5 if k==axis else 0) for k in range(3)))
            g.glEnd()
        self.relationships.draw()
        error=g.glGetError()
        if error:raise RuntimeError(f'Mission OpenGL error0x{error:04x}')
        g.glXSwapBuffers(self.display,self.window)
        self.stats.update(mesh_instances=meshes,triangles=triangles,gpu_meshes=len(self.buffers),
            frame_ms=round((time.perf_counter()-start)*1000,2),camera=self.camera.eye(),speed=self.camera.speed)
        now=time.monotonic()
        if self.on_stats and now-self.last_stats>.2:self.on_stats(self.stats);self.last_stats=now

    def close(self,destroy_window=True):
        if self.closed:return
        if self.display and self.context and self.bound:
            self.gl.glXMakeCurrent(self.display,self.window,self.context);self.release_buffers();self.relationships.close()
        self.scene=None;self.keys.clear();super().close(destroy_window)
