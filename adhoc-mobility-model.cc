
#include "adhoc-mobility-model.h"
#include "ns3/mobility-module.h"
#include "ns3/node-container.h"
#include "ns3/routes-mobility-helper.h"

namespace ns3 {

void
setupMobilityModel::installModel (NodeContainer nodeContainer, NodeContainer gatewayContainer)
{
    double searchLat = 41.171770;
    double searchLng = -8.611038;
    double searchRadius = 500;

    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
    //Install mobility helper on the nodes
    mobility.Install (nodeContainer);

    MobilityHelper gw_mobility;
    gw_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (gatewayContainer);

    RoutesMobilityHelper routes (searchLat, searchLng, 0);

    routes.ChooseRoute (nodeContainer, searchLat, searchLng, searchRadius);
}

void
setupMobilityModel::installModel (NodeContainer nodeContainer, NodeContainer gatewayContainer, unsigned int seed, unsigned int run)
{
    double searchLat = 41.171770;
    double searchLng = -8.611038;
    double searchRadius = 500;

    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
    //Install mobility helper on the nodes
    mobility.Install (nodeContainer);

    MobilityHelper gw_mobility;
    gw_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (gatewayContainer);

    RoutesMobilityHelper routes (searchLat, searchLng, 0);

    routes.ChooseRoute (nodeContainer, searchLat, searchLng, searchRadius, seed, run);
}

void
setupMobilityModel::installModel (NodeContainer nodeContainer)
{
    double searchLat = 41.171770;
    double searchLng = -8.611038;
    double searchRadius = 500;

    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
    //Install mobility helper on the nodes
    mobility.Install (nodeContainer);
    

    RoutesMobilityHelper routes (searchLat, searchLng, 0);

    routes.ChooseRoute (nodeContainer, searchLat, searchLng, searchRadius);
}

}