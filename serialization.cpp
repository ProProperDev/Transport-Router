#include "serialization.h"
#include "transport_catalogue.pb.h"


#include <fstream>

namespace transport_catalogue {

void Serializer::SetSettings(std::string file_name) {
    file_name_ = std::move(file_name);
}

serialized_data::Stop  Serializer::CreateSerializeStop(const domain::Stop* stop_ptr) {
    serialized_data::Stop stop;
    stop.set_name(stop_ptr->name);
    stop.mutable_coordinates()->set_lng(stop_ptr->stop_geo_pos.lng);
    stop.mutable_coordinates()->set_lat(stop_ptr->stop_geo_pos.lat);
    return stop;
}

serialized_data::Bus Serializer::CreateSerializeBus(const domain::Bus* bus_ptr) {
    serialized_data::Bus bus;
    bus.set_name(bus_ptr->name);
    bus.set_is_roundtrip(bus_ptr->is_roundtrip);
    for(const domain::Stop* stop_ptr : bus_ptr->bus_route) {
        bus.add_stops(reinterpret_cast<uint64_t>(stop_ptr));
    }
    return bus;
}

serialized_data::RouterSettings Serializer::CreateSerializeRouterSettings() {
    TransportRouter::RouterSettings settings = router_.GetSettings();
    serialized_data::RouterSettings serialized_settings;
    serialized_settings.set_time(settings.time);
    serialized_settings.set_velocity(settings.velocity);
    return serialized_settings;
}
    
domain::Stop Serializer::ConvertSerilizeStopToStop(const serialized_data::Stop& stop) {
    domain::Stop converted_stop{stop.name(), {stop.coordinates().lat(), stop.coordinates().lng()}};
    return converted_stop;
}

domain::Bus Serializer::ConvertSerilizeBusToBus(serialized_data::TransportCatalogue& serialize_catalog,
                                    const serialized_data::Bus& serialize_bus) {
    domain::Bus converted_bus;
    converted_bus.name = serialize_bus.name();
    converted_bus.is_roundtrip = serialize_bus.is_roundtrip();
    
    for (const auto stops_id : serialize_bus.stops()) {
        converted_bus.bus_route.push_back(db_.FindStop((*serialize_catalog.mutable_stops())[stops_id].name()));
    }

    return converted_bus;
}
//Из map[stop_id, stop_name] из serialize_catalog находим имя остановки по её id и по имени ищем в каталоге
domain::Stop* Serializer::GetStopPtrFromId(serialized_data::TransportCatalogue& serialize_catalog,
                               uint64_t stops_id) {
    return db_.FindStop((*serialize_catalog.mutable_stops())[stops_id].name());
}
    
serialized_data::StopDistance Serializer::CreateSerializeDistance(const domain::Stop* src_stop_ptr, const domain::Stop* dst_stop_ptr,
                                                      int64_t distance) {
    serialized_data::StopDistance stop_dist;
    //Заполняем поля src и dst, конвертировав адреса остановок в уникальные id
    stop_dist.set_src(reinterpret_cast<uint64_t>(src_stop_ptr));
    stop_dist.set_dst(reinterpret_cast<uint64_t>(dst_stop_ptr));
    stop_dist.set_distance(distance);
    return stop_dist;
}

serialized_data::Color Serializer::CreateSerializeColor(svg::Color color) {
    serialized_data::Color serialized_color;
    if (std::holds_alternative<std::string>(color)) {
        serialized_color.set_string_value(std::get<std::string>(color));
    } else if (std::holds_alternative<svg::Rgb>(color)) {
        svg::Rgb rgb_color = std::get<svg::Rgb>(color);
        serialized_color.mutable_rgb_value()->set_r(rgb_color.red);
        serialized_color.mutable_rgb_value()->set_g(rgb_color.green);
        serialized_color.mutable_rgb_value()->set_b(rgb_color.blue);
    } else if (std::holds_alternative<svg::Rgba>(color)) {
        svg::Rgba rgb_color = std::get<svg::Rgba>(color);
        serialized_color.mutable_rgba_value()->set_r(rgb_color.red);
        serialized_color.mutable_rgba_value()->set_g(rgb_color.green);
        serialized_color.mutable_rgba_value()->set_b(rgb_color.blue);
        serialized_color.mutable_rgba_value()->set_opacity(rgb_color.opacity);
    }
    return serialized_color;
}
    
serialized_data::RenderSettings Serializer::CreateSerializeRenderSettings() {
    render_settings settings = renderer_.GetSettings();
    serialized_data::RenderSettings serialized_settings;
    serialized_settings.set_width(settings.width);
    serialized_settings.set_height(settings.height);
    serialized_settings.set_padding(settings.padding);
    serialized_settings.set_line_width(settings.line_width);
    serialized_settings.set_stop_radius(settings.stop_radius);
    serialized_settings.set_bus_label_font_size(settings.bus_label_font_size);
    serialized_settings.mutable_bus_label_offset()->set_x(settings.bus_label_offset.x);
    serialized_settings.mutable_bus_label_offset()->set_y(settings.bus_label_offset.y);
    serialized_settings.set_stop_label_font_size(settings.stop_label_font_size);
    serialized_settings.mutable_stop_label_offset()->set_x(settings.stop_label_offset.x);
    serialized_settings.mutable_stop_label_offset()->set_y(settings.stop_label_offset.y);
    (*serialized_settings.mutable_underlayer_color()) = CreateSerializeColor(settings.underlayer_color);
    serialized_settings.set_underlayer_width(settings.underlayer_width);

    for (const auto& color : settings.color_palette) {
        *serialized_settings.add_color_palette() = CreateSerializeColor(color);
    }
    return serialized_settings;
}

svg::Color Serializer::ConvertSerializeColorToColor(serialized_data::Color color) {
    if (color.has_rgb_value()) {
        svg::Rgb rgb_color;
        rgb_color.red = color.rgb_value().r();
        rgb_color.green = color.rgb_value().g();
        rgb_color.blue = color.rgb_value().b();
        return rgb_color;
    } else if (color.has_rgba_value()) {
        svg::Rgba rgba_color;
        rgba_color.red = color.rgba_value().r();
        rgba_color.green = color.rgba_value().g();
        rgba_color.blue = color.rgba_value().b();
        rgba_color.opacity = color.rgba_value().opacity();
        return rgba_color;
    } else {
        std::string string_color = color.string_value();
        if (string_color.size() != 0) {
            return string_color;
        } else {
            return {};
        }
    }
}

render_settings Serializer::ConvertFromSerializeRenderSettings(const serialized_data::RenderSettings& settings) {
    render_settings unserialised_settings;
    unserialised_settings.width = settings.width();
    unserialised_settings.height = settings.height();
    unserialised_settings.padding = settings.padding();
    unserialised_settings.line_width = settings.line_width();
    unserialised_settings.stop_radius = settings.stop_radius();
    unserialised_settings.bus_label_font_size = settings.bus_label_font_size();
    unserialised_settings.bus_label_offset = {settings.bus_label_offset().x(),settings.bus_label_offset().y()};
    unserialised_settings.stop_label_font_size = settings.stop_label_font_size();
    unserialised_settings.stop_label_offset = {settings.stop_label_offset().x(),settings.stop_label_offset().y()};
    unserialised_settings.underlayer_color = ConvertSerializeColorToColor(settings.underlayer_color());
    unserialised_settings.underlayer_width = settings.underlayer_width();
    for(const auto& color : settings.color_palette()) {
        unserialised_settings.color_palette.push_back(ConvertSerializeColorToColor(color));
    }
    return unserialised_settings;
}

TransportRouter::RouterSettings Serializer::ConvertFromSerializeRouterSettings(const serialized_data::RouterSettings& serialize_settings) {
    return {serialize_settings.time(), serialize_settings.velocity()};
}
    
void Serializer::Serialize() {
    std::ofstream output(file_name_, std::ios::binary);
    serialized_data::TransportCatalogue serialized_catalogue;

    for (const auto& [name, stop_ptr] : db_.GetAllStops()) {
        (*serialized_catalogue.mutable_stops())[reinterpret_cast<uint64_t>(stop_ptr)] = CreateSerializeStop(stop_ptr);
    }

    for (const auto& [name, bus_ptr] : db_.GetAllBusnameToBusPtr()) {
        *serialized_catalogue.add_buses() = CreateSerializeBus(bus_ptr);
    }
    
    for(const auto& [stop_ptrs, distance] : db_.GetDistancesMap()) {
        *serialized_catalogue.add_distances() = CreateSerializeDistance(stop_ptrs.first, stop_ptrs.second, distance);
    }

    *(serialized_catalogue.mutable_render_settings()) = CreateSerializeRenderSettings();
    *(serialized_catalogue.mutable_router_settings()) = CreateSerializeRouterSettings();
    serialized_catalogue.SerializeToOstream(&output);
}

void Serializer::Deserialize() {
    std::ifstream input(file_name_, std::ios::binary);
    serialized_data::TransportCatalogue serialized_catalogue;
    serialized_catalogue.ParseFromIstream(&input);
//Заполняем базу остановками
    for(const auto& [stop_ptr, stop] : serialized_catalogue.stops()) {
        db_.AddStop(ConvertSerilizeStopToStop(stop));
    }
//Записываем расстояния между остановками
    for (const auto& distance_info : serialized_catalogue.distances()) {
        db_.SetStopToStopDistance(GetStopPtrFromId(serialized_catalogue, distance_info.src())->name,
                                  GetStopPtrFromId(serialized_catalogue, distance_info.dst())->name,
                                  distance_info.distance());
    }
//Заполняем базу автобусами
    for (const auto& bus : serialized_catalogue.buses()) {
        db_.AddBus(ConvertSerilizeBusToBus(serialized_catalogue, bus));
    }
//Записываем считанные установки в роутер и рендер
    renderer_.SetSettings(ConvertFromSerializeRenderSettings(serialized_catalogue.render_settings()));
    router_.SetSettings(ConvertFromSerializeRouterSettings(serialized_catalogue.router_settings()));
}

}//transport_catalogue