#pragma once

#include <string>

#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

namespace transport_catalogue {

class Serializer {
public:
    Serializer(TransportCatalogue& db, MapRenderer& renderer, TransportRouter& router)
        : db_(db), renderer_(renderer), router_(router) {          
        }
    
    void Serialize();
    void Deserialize();
    void SetSettings(std::string file_name);
private:
    //Serialize методы
    serialized_data::Stop CreateSerializeStop(const domain::Stop* stop_ptr);
    serialized_data::Bus CreateSerializeBus(const domain::Bus* bus_ptr);
    serialized_data::RouterSettings CreateSerializeRouterSettings();
    serialized_data::StopDistance CreateSerializeDistance(const domain::Stop* src_stop_ptr, const domain::Stop* dst_stop_ptr,
                                                      int64_t distance);
    serialized_data::Color CreateSerializeColor(svg::Color color);
    serialized_data::RenderSettings CreateSerializeRenderSettings();
    
    //Deserialize методы
    svg::Color ConvertSerializeColorToColor(serialized_data::Color color);
    domain::Stop ConvertSerilizeStopToStop(const serialized_data::Stop& stop);
    domain::Bus ConvertSerilizeBusToBus(serialized_data::TransportCatalogue& serialize_catalog,
                                        const serialized_data::Bus& serialize_bus);
    render_settings ConvertFromSerializeRenderSettings(const serialized_data::RenderSettings& settings);
    TransportRouter::RouterSettings ConvertFromSerializeRouterSettings(const serialized_data::RouterSettings& serialize_settings);
    domain::Stop* GetStopPtrFromId(serialized_data::TransportCatalogue& serialize_catalog,
                                   uint64_t stops_id);

    TransportCatalogue& db_;
    MapRenderer& renderer_;
    TransportRouter& router_;
    std::string file_name_;
};
}//namespace transport_catalogue