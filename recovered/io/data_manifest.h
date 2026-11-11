#ifndef HD2_IO_DATA_MANIFEST_H
#define HD2_IO_DATA_MANIFEST_H
#include "io/resource_observations.h"
namespace hd2::io {
inline std::string data_manifest_text(const std::string& value){
    std::string out;const char* digits="0123456789abcdef";
    for(unsigned char c:value){if(c<32||c==127||c=='%'){out+='%';out+=digits[c>>4];out+=digits[c&15];}else out+=static_cast<char>(c);}return out;
}
// Percent-escaped TSV, owning values only. Empty digest means unobserved full
// payload, never an empty file. Resolver absence alone has no requiredness label.
inline std::string resource_observation_manifest(const ResourceObservations& store){
    std::string out="resource\tflags\topen\tclosed\tio-fault\tshort-read\tdiagnostic-lost\tbytes\tsha256\torigin\n";
    for(const auto& r:store.records())out+=data_manifest_text(r.name)+"\t"+std::to_string(r.flags)+"\t"+(r.opened?"opened":"absent")+"\t"+(r.closed?"yes":"no")+"\t"+(r.io_fault?"yes":"no")+"\t"+(r.short_read?"yes":"no")+"\t"+(r.diagnostic_lost?"yes":"no")+"\t"+(r.size_known?std::to_string(r.size):"unknown")+"\t"+(r.sha256.empty()?"unknown":r.sha256)+"\t"+data_manifest_text(r.origin)+"\n";
    return out;
}
inline std::string data_observation_manifest(const ResourceObservations& store){
    std::string out="level\toperation\tdetail\tvalue\n";
    for(const auto& e:store.events())out+=data_manifest_text(e.level)+"\t"+data_manifest_text(e.operation)+"\t"+data_manifest_text(e.detail)+"\t"+std::to_string(e.value)+"\n";
    out+="diagnostic\tdropped\tcapacity-or-allocation-failure\t"+std::to_string(store.dropped())+"\n";return out;
}
template<class Output>void write_data_observations(const ResourceObservations& store,Output& output){
    const auto resources=resource_observation_manifest(store),events=data_observation_manifest(store);
    output.write("resource-provenance.tsv",resources.data(),resources.size());
    output.write("observations.tsv",events.data(),events.size());
    const auto completion=std::string("diagnostic-write-complete\tyes\nresource-records\t")+std::to_string(store.records().size())+
        "\nevents\t"+std::to_string(store.events().size())+"\ndropped\t"+std::to_string(store.dropped())+
        "\nengine-or-desktop-acceptance\tnot established by this marker\n";
    output.write("data-capture.tsv",completion.data(),completion.size());
}

}
#endif
