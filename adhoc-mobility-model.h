// 

#include "ns3/node-container.h"

namespace ns3 {

class setupMobilityModel
{
public:
    void installModel (NodeContainer nodeContainer, NodeContainer gatewayContainer);
    void installModel (NodeContainer nodeContainer, NodeContainer gatewayContainer, unsigned int seed, unsigned int run);
    void installModel (NodeContainer nodeContainer);


};
}