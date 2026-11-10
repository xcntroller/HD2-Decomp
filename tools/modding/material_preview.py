"""Established material fields/alpha sources, separated from preview approximations."""
from dataclasses import dataclass,asdict
import math


@dataclass(frozen=True)
class MaterialPreview:
    flags:int
    status:str
    color:tuple|None
    opacity:float|None
    color_blocks:tuple
    color_key:bool
    intrinsic_alpha:bool
    opacity_map:bool
    evidence:str='1002BBE0 maps material20000000→texture8,40000000→4,01000000→2;10075140..10075542'
    approximation:str='Untextured block14 RGB preview, white textured modulation, fixed preview lighting, RGBA8 color-key target and alpha threshold0.5; not original lighting/device quantization or blend ordering'

    def metadata(self):return asdict(self)


def describe(material):
    values=material.parameter_floats;flags=material.flags
    blocks=tuple(tuple(values[i:i+4]) for i in range(0,16,4))
    valid=len(values)>=18 and all(v is not None and math.isfinite(v) for v in values[:18])
    #1002B140..1002B1D0 establish four RGB blocks and two scalar setters.
    #Block14 as diffuse-like preview is a labelled interpretation, not a claim
    #that we have reconstructed every lighting/material field consumer.
    color=tuple(values[:3]) if valid and all(0<=v<=1 for v in values[:3]) else None
    opacity=values[17] if valid and 0<=values[17]<=1 else None #Native material+58;1000181C/1000EA7C.
    return MaterialPreview(flags,'recognized_partial' if color is not None and opacity is not None else 'unsupported_numeric_fields',
        color,opacity,blocks,bool(flags&0x20000000),bool(flags&0x01000000 and flags&0x60000000),
        bool(material.secondary_texture and flags&0x40000000 and not flags&0xa1000000))


def rgba_preview(image,policy,secondary=None):
    """Returns bytes only: caller owns cache/admission. No pixels persisted to disk.

    Native32-bit-target indexed branch compares packed palette RGB to zero.
    Native24/32-bit nonintrinsic branch compares the first source byte (blue)
    to key0. This odd rule is retained, not "corrected" into luminance.
    Opacity bitmap8 uses source indices, not palette colors (10075184/10075435).
    """
    pixels=image.width*image.height
    if policy.intrinsic_alpha:
        if image.native_alpha is None:raise ValueError('Material requests intrinsic alpha but supported source has no32-bit alpha bytes')
        alpha=image.native_alpha;mode='serialized32bit_alpha'
    elif secondary is not None:
        if (image.width,image.height)!=(secondary.width,secondary.height):raise ValueError('Opacity image dimensions differ; original resampling is unsupported')
        if secondary.indices is None:raise ValueError('Only established8-bit opacity-index source is supported')
        alpha=secondary.indices;mode='serialized_opacity_indices'
    elif policy.color_key:
        if image.metadata['bits_per_pixel']==8:
            alpha=bytes(0 if image.pixels[i:i+3]==b'\0\0\0' else 255 for i in range(0,len(image.pixels),3))
            mode='palette_RGB_black_key_RGBA8_target'
        else:
            alpha=bytes(0 if image.pixels[i]==0 else 255 for i in range(2,len(image.pixels),3));mode='native_blue_byte_zero_key'
    else:return image.pixels,3,'opaque_RGB'
    if len(alpha)!=pixels:raise ValueError('Alpha source size mismatch')
    result=bytearray(pixels*4)
    for channel in range(3):result[channel::4]=image.pixels[channel::3]
    result[3::4]=alpha
    return bytes(result),4,mode
