#include "map_renderer.h"

#include <set>
#include <unordered_set>

namespace transport_catalogue {

using namespace std::literals;

bool IsZero(double value) {
    return std::abs(value) < 1e-6;
}

render_settings MapRenderer::GetSettings() {
    return render_settings_;
}
    
svg::Document MapRenderer::RenderMap(TransportCatalogue& catalogue) {
    return RenderBusRoutes(catalogue.GetAllBuses());
}
// Прорисовка кривой маршрута
svg::Polyline MapRenderer::RenderBusRoute(const domain::Bus* bus_ptr,  SphereProjector& projector, int color_number) const {
    svg::Polyline route;
    for (const auto& stop_ptr : bus_ptr->bus_route) {
        route.AddPoint(projector(stop_ptr->stop_geo_pos));
    }

    route.SetFillColor("none");
    route.SetStrokeColor(render_settings_.color_palette[color_number % render_settings_.color_palette.size()]);
    route.SetStrokeWidth(render_settings_.line_width);
    route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return route;
}
// Прорисовка символа остановки
svg::Circle MapRenderer::RenderStopSymbol(svg::Point position) const {
    svg::Circle stop_symbol;
    stop_symbol.SetCenter(position);
    stop_symbol.SetRadius(render_settings_.stop_radius);
    stop_symbol.SetFillColor("white"s);
    return stop_symbol;
} 

svg::Document MapRenderer::RenderBusRoutes( std::deque<domain::Bus> buses) const {//исправить константную ссылку и нормальную сортировку
    svg::Document svg_doc;
// Сортируем маршруты в алфавитном порядке по их названию 
    std::sort(begin(buses), end(buses),[](auto a, auto b) { return a.name < b.name;});
    
    std::vector<domain::Stop*> stop_ptr_arr; // Вектор остановок для дальнейшей их прорисовки 
    
    for (const auto& bus : buses) {
        for(auto stopname : bus.bus_route) {
            stop_ptr_arr.push_back(stopname);
        }
    }
    
    std::sort(stop_ptr_arr.begin(), stop_ptr_arr.end(), [](auto& a, auto& b) {return a->name < b->name;});
    auto it = std::unique(stop_ptr_arr.begin(), stop_ptr_arr.end());
    stop_ptr_arr.erase(it, stop_ptr_arr.end()); // Удаляем дубликаты остановок, чтобы не рисовать их по несколько раз
    
    SphereProjector projector(stop_ptr_arr.begin(),
                              stop_ptr_arr.end(),
                              render_settings_.width,
                              render_settings_.height,
                              render_settings_.padding);
    
    int route_count = 0;
// Рисуем кривые всех маршрутов
    for (const auto& bus : buses) {
        if (bus.bus_route.empty()) {
            continue;
        }
        svg_doc.Add(RenderBusRoute(&bus, projector, route_count));
        ++route_count;
    }
    
    route_count = 0;
// Рисуем названия всех маршрутов    
    for (const auto& bus : buses) {
      if (bus.bus_route.empty()) {
            break;
        }

        svg_doc.Add(RenderBusNameBase(&bus, projector(bus.bus_route[0]->stop_geo_pos)));
        svg_doc.Add(RenderBusName(&bus, projector(bus.bus_route[0]->stop_geo_pos), route_count));
// Проверяем: Если маршрут некольцевой, то на остановке разворота дублируем название маршрута
        if (std::equal(bus.bus_route.begin(),bus.bus_route.end(), bus.bus_route.rbegin(),bus.bus_route.rend())
            && bus.bus_route[0] != bus.bus_route[bus.bus_route.size()/2] && bus.bus_route.size() > 1 && !bus.is_roundtrip) {
            svg_doc.Add(RenderBusNameBase(&bus,
                                          projector(bus.bus_route[bus.bus_route.size()/2]->stop_geo_pos)));
            svg_doc.Add(RenderBusName(&bus,
                                      projector(bus.bus_route[bus.bus_route.size()/2]->stop_geo_pos), route_count));
        }
         
        ++route_count;
    }
// Рисуем символы всех остановок
    for (const domain::Stop* stop_ptr : stop_ptr_arr) { 
        svg_doc.Add(RenderStopSymbol(projector(stop_ptr->stop_geo_pos)));
    }
// Рисуем названия всех остановок    
    for (const domain::Stop* stop_ptr : stop_ptr_arr) {
        svg_doc.Add(RenderStopNameBase(stop_ptr,  projector(stop_ptr->stop_geo_pos)));
        svg_doc.Add(RenderStopName(stop_ptr,  projector(stop_ptr->stop_geo_pos)));
        
    }
    
    return svg_doc;
}
    
svg::Text MapRenderer::RenderBusName(const domain::Bus* bus_ptr,  svg::Point position, int color_number) const {
    svg::Text route_name;
    
    route_name.SetPosition(position);
    route_name.SetOffset(render_settings_.bus_label_offset);
    route_name.SetFontSize(render_settings_.bus_label_font_size);
    route_name.SetFontFamily("Verdana"s);
    route_name.SetFontWeight("bold"s);
    route_name.SetData(bus_ptr->name);
    
    route_name.SetFillColor(render_settings_.color_palette[color_number % render_settings_.color_palette.size()]);
    
    return route_name;
}

    
svg::Text MapRenderer::RenderBusNameBase(const domain::Bus* bus_ptr,  svg::Point position) const {
    svg::Text route_name_base;
    
    route_name_base.SetPosition(position);
    route_name_base.SetOffset(render_settings_.bus_label_offset);
    route_name_base.SetFontSize(render_settings_.bus_label_font_size);
    route_name_base.SetFontFamily("Verdana"s);
    route_name_base.SetFontWeight("bold"s);
    route_name_base.SetData(bus_ptr->name);
    
    route_name_base.SetFillColor(render_settings_.underlayer_color);
    route_name_base.SetStrokeColor(render_settings_.underlayer_color);
    route_name_base.SetStrokeWidth(render_settings_.underlayer_width);
    route_name_base.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    route_name_base.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
    return route_name_base;
}
    
svg::Text MapRenderer::RenderStopNameBase(const domain::Stop* stop_ptr, svg::Point position) const {
    svg::Text stop_name_base;
    
    stop_name_base.SetPosition(position);
    stop_name_base.SetOffset(render_settings_.stop_label_offset);
    stop_name_base.SetFontSize(render_settings_.stop_label_font_size);
    stop_name_base.SetFontFamily("Verdana"s);
    stop_name_base.SetData(stop_ptr->name);
    
    stop_name_base.SetFillColor(render_settings_.underlayer_color);
    stop_name_base.SetStrokeColor(render_settings_.underlayer_color);
    stop_name_base.SetStrokeWidth(render_settings_.underlayer_width);
    stop_name_base.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    stop_name_base.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
    return stop_name_base;
}
    
svg::Text MapRenderer::RenderStopName(const domain::Stop* stop_ptr, svg::Point position) const {
    svg::Text stop_name;

    stop_name.SetPosition(position);
    stop_name.SetOffset(render_settings_.stop_label_offset);
    stop_name.SetFontSize(render_settings_.stop_label_font_size);
    stop_name.SetFontFamily("Verdana"s);
    stop_name.SetData(stop_ptr->name);
    
    
    stop_name.SetFillColor("black"s);
    
    return stop_name;
}

}