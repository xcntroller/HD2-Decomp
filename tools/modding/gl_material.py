"""Preview state only; interpretation/provenance lives in material_preview.py."""

def blended(state):
    return state.get('opacity') is not None and state['opacity']<1 or state.get('rendering_status')=='RGBA_preview' and (state.get('opacity_map') or state.get('intrinsic_alpha'))


def apply(gl,state,texture,selected=False,picking=False):
    rgba=bool(texture and state.get('rendering_status')=='RGBA_preview')
    alpha=state.get('opacity')
    if alpha is None:alpha=1.
    if texture:gl.glEnable(0x0de1);gl.glBindTexture(0x0de1,texture)
    else:gl.glDisable(0x0de1)
    if rgba:
        gl.glEnable(0x0bc0);gl.glAlphaFunc(0x0204,.001 if blended(state) else .5)
    else:gl.glDisable(0x0bc0)
    if picking:
        gl.glDisable(0x0be2);gl.glDepthMask(1)
        #Keep the exact picking ID in RGB while texture alpha cuts holes.
        if rgba:
            for pname,value in ((0x2200,0x8570),(0x8571,0x1e01),(0x8580,0x8577),(0x8572,0x1e01),(0x8588,0x1702)):
                gl.glTexEnvi(0x2300,pname,value)
        else:gl.glDisable(0x0de1)
        return
    gl.glTexEnvi(0x2300,0x2200,0x2100)
    if blended(state):gl.glEnable(0x0be2);gl.glBlendFunc(0x0302,0x0303);gl.glDepthMask(0)
    else:gl.glDisable(0x0be2);gl.glDepthMask(1)
    #Original textured-lighting color consumers are not reconstructed. Do not
    #multiply arbitrary editor color blocks into a successfully decoded image.
    color=(1,1,1) if texture else state.get('color') if state.get('rendering_status')=='color_only_preview' else (.65,.70,.74)
    if selected:color=(1.,.65,.15)
    gl.glColor4f(*color,alpha)


def reset(gl):
    gl.glDisable(0x0bc0);gl.glDisable(0x0be2);gl.glDepthMask(1)
    gl.glTexEnvi(0x2300,0x2200,0x2100)
