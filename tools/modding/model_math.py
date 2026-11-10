"""4DS frame matrices: LS3DF 1002D450,1001E390,1002D830.

Row-vector matrices, translation in row 3. Kept separate from the display camera.
No axis conversion is applied to serialized coordinates.
"""
import math

IDENTITY=(1.,0.,0.,0., 0.,1.,0.,0., 0.,0.,1.,0., 0.,0.,0.,1.)


def multiply(a,b):
    return tuple(sum(a[row*4+k]*b[k*4+col] for k in range(4)) for row in range(4) for col in range(4))


def local_matrix(position,quaternion,scale):
    x,y,z,w=quaternion
    # The loader reads serialized XYZW directly; it does not call Normalize.
    # Match SetRot3's special identity branch as well as its component products.
    if abs(w)>=1.:rotation=(1.,0.,0.,0.,1.,0.,0.,0.,1.)
    else:rotation=(1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w),
                   2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w),
                   2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y))
    result=[]
    for row in range(3):result.extend([rotation[row*3+k]*scale[row] for k in range(3)]+[0.])
    return tuple(result)+tuple(position)+(1.,)


def transform(matrix,point):
    return tuple(sum(point[k]*matrix[k*4+c] for k in range(3))+matrix[12+c] for c in range(3))


def bounds(points):
    low=[math.inf]*3;high=[-math.inf]*3;count=0
    for point in points:
        if not all(math.isfinite(c) and abs(c)<=1e20 for c in point):raise ValueError('Nonfinite/unrenderable transformed coordinate')
        for k,c in enumerate(point):low[k]=min(low[k],c);high[k]=max(high[k],c)
        count+=1
    return (tuple(low),tuple(high)) if count else None
