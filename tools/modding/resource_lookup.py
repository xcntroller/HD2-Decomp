"""Bounded explicit cache-directory profiles; archive selection stays ResourceStore's."""
from dataclasses import asdict
from resource_store import virtual_name


class ResourceLookup:
    def __init__(self,store,model_directories=('models',),texture_directories=('maps','maps_c','maps_u')):
        self.store=store
        self.models=tuple(virtual_name(p) for p in model_directories)
        self.textures=tuple(virtual_name(p) for p in texture_directories)
        if not self.models or max(len(self.models),len(self.textures))>256:raise ValueError('Directory profile exceeds tool bounds')
        self.last_texture_directory=None

    def model(self,key):
        key=virtual_name(key)
        #Client model_resource_key removes the last dot suffix; the engine
        #worker independently removes a last filename suffix before appending
        #.4ds. Keep both steps for unusual multi-dot names.
        key=key.rsplit('.',1)[0] if '.' in key else key
        paths=[]
        for root in self.models:
            path=root+'/'+key;leaf=path.rsplit('/',1)[-1]
            if '.' in leaf:path=path.rsplit('.',1)[0]
            paths.append(virtual_name(path+'.4ds'))
        result=self.select(paths,'ordered_model_directory_profile',
            'model_resource_key/006D4EC0/1005D260; registered order, missing candidates only')
        result['cache_key']=key
        return result

    def texture(self,name):
        name=virtual_name(name);paths=[]
        if '/' in name:paths.append(name) #1006FBD0: direct path probe first.
        roots=([self.last_texture_directory] if self.last_texture_directory else [])+list(reversed(self.textures))
        for root in roots:
            candidate=root+'/'+name
            if candidate not in paths:paths.append(candidate)
        result=self.select(paths,'ordered_texture_directory_profile',
            '10069F70/1006FBD0; direct path, last successful directory, reverse registered order')
        if result['path']:
            path=result['path']
            #A direct path hit does not change the native sticky-directory state.
            if path!=name:
                self.last_texture_directory=path[:-len(name)-1]
        return result

    def select(self,paths,certainty,evidence):
        attempts=[]
        for path in paths:
            resolution=self.store.resolution(path)
            attempts.append({'path':path,'status':resolution.status})
            if resolution.status!='missing':
                #Defensive barrier: don't turn inspection/codec/CRC failures into
                #native "not found" and silently choose another directory/member.
                return {'path':path,'status':resolution.status,'resolution':asdict(resolution),
                    'attempts':attempts,'certainty':certainty,'evidence':evidence}
        return {'path':None,'status':'missing','attempts':attempts,'certainty':certainty,'evidence':evidence,
            'message':'No resource in supplied registered-root profile; unknown directory enumeration/cache state is not guessed'}

    def metadata(self):
        return {'models':self.models,'textures':self.textures,'scope':'Explicit ordered roots; Windows descendant enumeration and warm engine cache are not reconstructed',
            'texture_search':'direct / last successful directory / reverse registered roots',
            'invalid_selection_policy':'stop; never substitute another directory or shadowed archive'}
