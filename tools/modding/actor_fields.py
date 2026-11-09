"""Serialized actor-property subsets copied from verified reader contracts.

Fields named fieldXXX/raw words deliberately do not acquire guessed semantics.
No constructors, callbacks, portal refresh or runtime defaults are executed.
"""
from __future__ import annotations
import math
import struct

ACTOR_NAMES = {3:"Human",4:"Car",6:"Door",8:"Ladder",9:"SupplyBox",10:"SpawnPoint",
               13:"Switch",21:"StaticWeapon",22:"ExplosivePlace",25:"Animation",28:"Pike",
               29:"FowlGenerator",30:"Dog",33:"NullActor"}
FRAME_NAMES = {1:"Visual",2:"Light",3:"Camera",4:"Sound",5:"Sector",6:"Dummy",7:"Target",
               8:"User",9:"Model",10:"Joint",11:"Volume",12:"Occluder",13:"Scene",14:"Area",
               15:"Shadow",16:"Landscape"}
ADDRESSES = {3:"0059E800",6:"00601AC0",8:"005EBBD0",9:"005FC200",10:"006119B0",
             13:"00613E10",21:"00604CE0",22:"0060CA30",25:"005B5AF0",28:"005B9C80",
             29:"005BB7E0",30:"005D6630",33:"005E2BC0"}


def decode_properties(cursor, actor_type):
    evidence="recovered/game/actor_properties.h:"+ADDRESSES.get(actor_type,"unsupported")
    values={"actor_type":actor_type,"actor_label":ACTOR_NAMES.get(actor_type,"Unknown"),"evidence":evidence}
    start=cursor.position
    if cursor.position==cursor.end:
        values.update(decode_status="empty_property_payload",consumed_bytes=0,unconsumed_bytes=0,
                      evidence="recovered/game/mission_scene_reader.h:006CC760/006CD940 AE24",
                      note="Original AE24 dispatch skips an empty property payload; no defaults inferred")
        return values
    if actor_type not in ADDRESSES:
        values["decode_status"]="unsupported_property_decoder" if actor_type in ACTOR_NAMES else "unsupported_actor_type"
        return values
    fields=[]
    def read(label,fmt):
        offset=cursor.position
        value=cursor.unpack(fmt)
        if not all(not isinstance(v,float) or math.isfinite(v) for v in value):raise ValueError("Nonfinite actor property")
        fields.append({"offset":offset,"size":struct.calcsize(fmt),"field":label,"value":value[0] if len(value)==1 else list(value)})
        return value[0] if len(value)==1 else value
    def version(low,high):
        v=read("serialized_version","<b")
        if not low<=v<=high:raise NotImplementedError(f"Unsupported {ACTOR_NAMES[actor_type]} version {v}")
        return v
    def chunks(schema,required_version=None):
        if required_version is not None:
            off,size,kind,payload=cursor.chunk()
            if kind!=0 or payload.unpack("<I")[0]!=required_version:raise NotImplementedError("Unsupported actor chunk version")
            fields.append({"offset":off,"size":size,"field":"version_chunk","value":required_version})
        while cursor.position<cursor.end:
            off,size,kind,payload=cursor.chunk()
            if kind not in schema:
                fields.append({"offset":off,"size":size,"field":"unknown_chunk","id":kind})
                continue
            label,fmt=schema[kind]
            value=payload.text() if fmt=="text" else payload.unpack(fmt)
            if isinstance(value,tuple):
                if not all(not isinstance(v,float) or math.isfinite(v) for v in value):raise ValueError("Nonfinite actor property")
                value=value[0] if len(value)==1 else list(value)
            fields.append({"offset":off,"size":size,"field":label,"id":kind,"value":value,"unconsumed_bytes":payload.end-payload.position})
    try:
        if actor_type==3:
            v=version(11,15)
            read("field19c","<I");read("field1a0","<I");read("stats_ec_raw_words","<12I");read("field170","<I")
            for threshold,label in [(11,"flag1a4"),(12,"flag1a5"),(13,"flag1a6")]:
                if v>threshold:read(label,"<B")
            if v>14:read("field1a8","<I")
        elif actor_type==8:
            v=version(1,3);read("settings170","<3I")
            if v>1:read("setting17c","<I")
            if v>2:read("flag180","<B")
        elif actor_type==9:
            read("ignored_version","<B")
            for label in ("field11c","field120","field128","field12c","field134"):read(label,"<I")
            read("item_ids_list0_including_sentinels","<10I");read("item_ids_list1_including_sentinels","<10I");read("field210","<I")
        elif actor_type==28:
            for label in ("field184","field18c","field190"):read(label,"<I")
            read("vector90_raw_words","<3I");read("fields194","<3I")
        elif actor_type==30:
            read("ignored_leading_word","<I");read("fields4a0","<4I")
        elif actor_type in (21,33):
            values["note"]="Original property receiver is a no-op; no payload semantics inferred"
        elif actor_type==10:
            chunks({1:("field90","<I"),2:("field94","<I"),3:("fielda4","<I"),4:("field98","<I"),5:("fielda0","<I")},3)
        elif actor_type==29:
            chunks({1:("fielda0","<I"),2:("fielda8","<I"),3:("fieldac","<I"),4:("fieldb0","<I"),5:("name","text")},1)
        elif actor_type==25:
            if cursor.position==cursor.end:raise ValueError("Empty Animation properties")
            chunks({0:("name0","text"),1:("fieldd0_serialized","<f"),2:("flaga2","<B"),3:("fielda8","<I"),4:("fieldac","<I"),5:("name5","text"),6:("fieldb0","<I"),7:("flaga4","<B")})
        elif actor_type==22:
            chunks({100:("field9c_flag98","<IB"),0x65:("fielda0","<I"),0x66:("fielda4","<I"),0x67:("fieldsa8","<4I"),0x68:("flagb8","<B"),0x69:("flagb9","<B")})
        elif actor_type==13:
            chunks({0:("fieldd4_serialized_state","<I"),1:("fieldd8","<I")})
        elif actor_type==6:
            read("fields220","<4I");read("vector210_raw_words","<3I");read("field230","<I")
            read("flag234","<B");read("ignored_historical_byte","<B");read("ignored_historical_word","<I");read("flag23c","<B")
            chunks({0x65:("fields240","<9I"),0x66:("flaga0","<B"),0x67:("name120","text"),0x68:("flag98","<B"),0x69:("flag99","<B"),0x6a:("vector200_raw_words","<3I"),0x6b:("flag190","<B"),0x6c:("flag235","<B"),0x6d:("field114_flag118","<IB")})
        values["fields"]=fields
        values["decode_status"]="partial_unknown_chunks" if any(f["field"]=="unknown_chunk" for f in fields) else "decoded_serialized_subset"
        values["consumed_bytes"]=cursor.position-start
        values["unconsumed_bytes"]=cursor.end-cursor.position
    except NotImplementedError as exc:
        values.update(decode_status="unsupported_version",diagnostic=str(exc))
    except (ValueError,struct.error) as exc:
        # Never publish partial interpreted fields from a malformed known record.
        values.update(decode_status="malformed",diagnostic=str(exc))
    return values
