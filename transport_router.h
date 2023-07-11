#pragma once

#include "transport_catalogue.h"
#include "router.h"

#include <memory>
#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <vector>

class TransportRouter {
public:
     enum class ItemType {
        BUS,
        WAIT
    };
    
    struct Item {
        ItemType type;
        std::string_view name;
        double time;
        int span_count;
    };
    
    struct RouteItems {
        double total_time;
        std::vector<Item> items;
    };
    
    struct RouterSettings {
        int time;
        double velocity;
    };
    
    TransportRouter(const transport_catalogue::TransportCatalogue& catalogue) : catalogue_(catalogue) { route_graph_.reset();
    }
   
    void SetSettings(RouterSettings settings);
    RouterSettings GetSettings() const; 
    void BuildAllRoutes();
    std::optional<RouteItems> BuildRouteByStops(const domain::Stop* start_stop, const domain::Stop* finish_stop) const;

private:
    void AddRouteToGraph(std::string_view bus_name, const domain::Bus* bus_ptr);
    void CreateVertexFromStopWithEdge();
    graph::VertexId GetWaitVertexByStop(const domain::Stop* stop_ptr) const;
    graph::VertexId GetBusVertexByStop(const domain::Stop* stop_ptr) const;
    void AddEdgeToItem(graph::VertexId start_vertex, graph::VertexId stop_vertex, Item item);
    void AddBusEdge(const domain::Stop* start_stop, const domain::Stop* finish_stop,
                    std::string_view bus_name, int span, double distance);
    void FillGpaphWithRoutes();

    RouterSettings settings_;
    const transport_catalogue::TransportCatalogue& catalogue_;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> route_graph_;
    std::unique_ptr<graph::Router<double>> router_;
    std::map<const domain::Stop*, std::pair<int, int>> stop_to_vertex_pair_;
    std::map<graph::EdgeId, Item> edge_to_item_;
};