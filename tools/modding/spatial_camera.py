"""Positive-Z camera convention, separate from original object transforms.

LS3DF10009B30 perspective stores clip W=+view Z (10009CC0). OpenGL
expects negative view Z; convert at the display view matrix, never in assets.
"""
from dataclasses import dataclass,field
import math


def add(a,b,k=1.):return [a[i]+b[i]*k for i in range(3)]
def dot(a,b):return sum(a[i]*b[i] for i in range(3))


@dataclass
class Camera:
    mode:str='orbit'
    position:list=field(default_factory=lambda:[0.,0.,10.])
    target:list=field(default_factory=lambda:[0.,0.,0.])
    yaw:float=145.  # Same initial world eye/direction as the former RH preset.
    pitch:float=-22.
    distance:float=10.
    speed:float=20.
    far:float=1000.
    def basis(self):
        y,p=map(math.radians,(self.yaw,self.pitch));c=math.cos(p)
        forward=(math.sin(y)*c,math.sin(p),math.cos(y)*c)
        right=(math.cos(y),0.,-math.sin(y))
        up=(-math.sin(y)*math.sin(p),c,-math.cos(y)*math.sin(p))
        return forward,right,up
    def eye(self):return add(self.target,self.basis()[0],-self.distance) if self.mode=='orbit' else list(self.position)
    def view_matrix(self):
        """OpenGL column-major view with engine right/up/+forward convention.

        Camera-space depth alone changes sign. Culling/movement/pan use the
        same basis; drawing, origins, bounds and ID picking share this matrix.
        Its reflection changes winding; the preview still disables face culling.
        """
        forward,right,up=self.basis();eye=self.eye()
        return (right[0],up[0],-forward[0],0.,
                right[1],up[1],-forward[1],0.,
                right[2],up[2],-forward[2],0.,
                -dot(right,eye),-dot(up,eye),dot(forward,eye),1.)
    def switch(self,mode):
        eye=self.eye()
        if mode=='fly':self.position=eye
        elif self.mode=='fly':self.target=add(eye,self.basis()[0],self.distance)
        self.mode=mode
    def focus(self,box=None,point=None):
        self.target=[(box[0][i]+box[1][i])*.5 for i in range(3)] if box else list(point or (0.,0.,0.))
        radius=max(math.dist(*box)*.5,.25) if box else 2.
        self.distance=radius*3.;self.mode='orbit'
        self.far=max(self.far,self.distance+radius*4)
    def look(self,dx,dy):
        self.yaw=(self.yaw+dx*.25)%360;self.pitch=max(-89.,min(89.,self.pitch-dy*.25))
    def move(self,keys,dt):
        if self.mode!='fly' or not keys:return False
        forward,right,_=self.basis();delta=[0.,0.,0.]
        for name,vector,sign in [('w',forward,1),('s',forward,-1),('a',right,-1),('d',right,1),('e',(0,1,0),1),('q',(0,1,0),-1)]:
            if name in keys:delta=add(delta,vector,sign)
        length=math.sqrt(dot(delta,delta))
        if not length:return False
        self.position=add(self.position,delta,min(dt,.1)*self.speed*(4 if 'shift' in keys else 1)/length);return True
    def in_view(self,box,point,aspect):
        center=[(box[0][i]+box[1][i])*.5 for i in range(3)] if box else point
        if center is None:return False
        radius=math.dist(*box)*.5 if box else .5
        vector=add(center,self.eye(),-1);forward,right,up=self.basis();z=dot(vector,forward)
        if z+radius<.03 or z-radius>self.far:return False
        # Conservative bounding-sphere planes; false positives are acceptable.
        slope=math.tan(math.radians(60)*.5)
        return abs(dot(vector,right))<=max(z,0)*slope*aspect+radius*math.sqrt(1+(slope*aspect)**2) and abs(dot(vector,up))<=max(z,0)*slope+radius*math.sqrt(1+slope*slope)
