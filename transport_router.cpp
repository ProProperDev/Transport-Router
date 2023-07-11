#include "transport_router.h"

void TransportRouter::SetSettings(RouterSettings settings) {
    settings_ = std::move(settings);
}

TransportRouter::RouterSettings TransportRouter::GetSettings() const {
    return settings_;
}

graph::VertexId TransportRouter::GetWaitVertexByStop(const domain::Stop* stop_ptr) const {
    return stop_to_vertex_pair_.at(stop_ptr).first;
}

graph::VertexId TransportRouter::GetBusVertexByStop(const domain::Stop* stop_ptr) const {
    return stop_to_vertex_pair_.at(stop_ptr).second;
}

void TransportRouter::AddEdgeToItem(graph::VertexId start_vertex, graph::VertexId stop_vertex, Item item) {
    graph::EdgeId edge = route_graph_->AddEdge({start_vertex, stop_vertex, item.time});
    edge_to_item_[edge] = item;
}

void TransportRouter::AddBusEdge(const domain::Stop* start_stop, const domain::Stop* finish_stop, std::string_view bus_name, int span, double distance) {
    Item item;
    item.type = ItemType::BUS;
    item.name = bus_name;
    item.time = distance /(settings_.velocity *  1000 / 60);
    item.span_count = span;
    AddEdgeToItem(GetBusVertexByStop(start_stop),
                  GetWaitVertexByStop(finish_stop),
                  item);
}

void TransportRouter::AddRouteToGraph(std::string_view bus_name, const domain::Bus* bus_ptr) {
//Если маршрут кольцевой учитаваем только forward_distance, тк автобус едет от остановки к остановке только в одном направлении
    if (bus_ptr->is_roundtrip) {
        for (int stop_index = 0; stop_index < bus_ptr->bus_route.size()-1; ++stop_index) {
            double forward_distance = 0.0;
            for (int current_stop_index = stop_index; current_stop_index < bus_ptr->bus_route.size()-1 ; ++current_stop_index) {
                int next_stop_index = current_stop_index+1;
                forward_distance += catalogue_.GetStopToStopDistance(bus_ptr->bus_route[current_stop_index], 
                                                                     bus_ptr->bus_route[next_stop_index]);
                AddBusEdge(bus_ptr->bus_route[stop_index], bus_ptr->bus_route[next_stop_index],
                           bus_name, next_stop_index - stop_index, forward_distance);
            }
        }
    }
//Для некольцевого маршрута учитываем разные направления движения между остановками
    if(!bus_ptr->is_roundtrip) {
        for (int stop_index = 0; stop_index < bus_ptr->bus_route.size()/2; ++stop_index) {
            double forward_distance = 0.0;
            double backward_distance = 0.0;
            for (int current_stop_index = stop_index; current_stop_index < bus_ptr->bus_route.size()/2; ++current_stop_index) {
                int next_stop_index = current_stop_index+1;
                forward_distance += catalogue_.GetStopToStopDistance(bus_ptr->bus_route[current_stop_index],
                                                                     bus_ptr->bus_route[next_stop_index]);
                AddBusEdge(bus_ptr->bus_route[stop_index], bus_ptr->bus_route[next_stop_index],
                           bus_name, next_stop_index - stop_index, forward_distance);

                backward_distance += catalogue_.GetStopToStopDistance(bus_ptr->bus_route[next_stop_index],
                                                                      bus_ptr->bus_route[current_stop_index]);
                AddBusEdge(bus_ptr->bus_route[next_stop_index], bus_ptr->bus_route[stop_index],
                           bus_name, next_stop_index - stop_index, backward_distance);
            }
        }
    }
}

void TransportRouter::CreateVertexFromStopWithEdge() {
    graph::VertexId vertex_id = 0;
    for (const auto& [stop_name, stop_ptr] : catalogue_.GetAllStops()) {
        stop_to_vertex_pair_[stop_ptr] = {vertex_id, vertex_id + 1};
        graph::EdgeId edge_ = route_graph_->AddEdge({vertex_id, vertex_id + 1, static_cast<double>(settings_.time)});
        edge_to_item_[edge_] = {ItemType::WAIT, stop_name, static_cast<double>(settings_.time), 1};
        vertex_id += 2;
    }
}

void TransportRouter::FillGpaphWithRoutes() {
 for(const auto [busname, bus_ptr] : catalogue_.GetAllBusnameToBusPtr()) {
        AddRouteToGraph(busname, bus_ptr);
    }
}

void TransportRouter::BuildAllRoutes() {
    route_graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(catalogue_.GetAllStops().size() *2 );
    CreateVertexFromStopWithEdge();
    FillGpaphWithRoutes();

    router_ = std::make_unique<graph::Router<double>>(*route_graph_);
}

std::optional<TransportRouter::RouteItems> TransportRouter::BuildRouteByStops(const domain::Stop* start_stop, const domain::Stop* finish_stop) const {
    RouteItems items_info;
    std::optional<graph::Router<double>::RouteInfo> router_info = router_->BuildRoute(GetWaitVertexByStop(start_stop), GetWaitVertexByStop(finish_stop));
    if (router_info) {
        items_info.total_time = router_info.value().weight;
        for (const auto& edge : router_info.value().edges) {
            items_info.items.push_back(edge_to_item_.at(edge));
        }
        return items_info;
    } else {
        return std::nullopt;
    }
}