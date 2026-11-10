"""Viewer camera math, deliberately separate from original object transforms."""
from dataclasses import dataclass,field
import math


def add(a,b,k=1.):return [a[i]+b[i]*k for i in range(3)]
def dot(a,b):return sum(a[i]*b[i] for i in range(3))


@dataclass
class Camera:
    mode:str='orbit'
    position:list=field(default_factory=lambda:[0.,0.,10.])
    target:list=field(default_factory=lambda:[0.,0.,0.])
    yaw:float=35.
    pitch:float=-22.
    distance:float=10.
    speed:float=20.
    far:float=1000.
    def basis(self):
        y,p=map(math.radians,(self.yaw,self.pitch));c=math.cos(p)
        forward=(math.sin(y)*c,math.sin(p),-math.cos(y)*c)
        right=(math.cos(y),0.,math.sin(y))
        up=(-math.sin(y)*math.sin(p),c,math.cos(y)*math.sin(p))
        return forward,right,up
    def eye(self):return add(self.target,self.basis()[0],-self.distance) if self.mode=='orbit' else list(self.position)
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
