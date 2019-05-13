#include <chrono>
#include <fstream>
#include <iostream>
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/network-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/pointer.h"
#include "ns3/olsr-helper.h"
#include "ns3/aodv-helper.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/random-variable-stream.h"
auto T1=0;
auto T2=0;
auto now = 0;
using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhoc");
using namespace std;
int GTC = 0;
int MaxChildren = 0;
int Run_number;
uint32_t numNodes;
uint32_t numPacketChildren;
vector<Ptr<Socket>> VectorSource;
double min_random_interval = 0.0;
double max_random_interval = 10.0;
float min_packetinterval = 0.5;
int max_packetinterval = 2;
  

  // Call this function everytime a random number is desired
  double Randomnummer(){
  // the -1 is due numNodes-1 is the maount of nodes that may send. The *10 is in order to get the resolution used for intervals up to 1 decimal i.e 1.1, 1.2, 1.3, and so on
  max_random_interval = static_cast<double>((numNodes-1)*10);    
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (min_random_interval));
  x->SetAttribute ("Max", DoubleValue (max_random_interval));
  double randgen = x->GetValue ();
  return randgen;
}



void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet!");
      
    }
}


 static void GenerateTrafficChild (Ptr<Socket> socket, uint32_t pktSize,
                             int pktCount, Time pktInterval)
{
    if(pktCount > 0){
      //Due to the increased resolution of the randomnummer we use modulus the size of vector to ensure no out of bounds
      int v = static_cast<int>(Randomnummer()) % static_cast<int>(VectorSource.size()) ;
    
      //NS_LOG_UNCOND(socket->GetNode()->GetId());
      socket = VectorSource[v];
      double Tid = (static_cast<int>(round(Randomnummer())) % (max_packetinterval*10))/10.0 + min_packetinterval;
      Time interPacketInterval = Seconds (Tid);
    
      pktInterval = interPacketInterval;
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTrafficChild,
                           socket, pktSize,pktCount - 1, pktInterval);
      
      NS_LOG_UNCOND ("Sending one packet!");
    }
  else
    { 
      socket->Close ();
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             int pktCount, Time pktInterval)
{

    if(pktCount > 0){
      //Due to the increased resolution of the randomnummer we use modulus the size of vector to ensure no out of bounds
      int v = static_cast<int>(Randomnummer()) % static_cast<int>(VectorSource.size()) ;
      socket = VectorSource[v];
      double Tid = (static_cast<int>(round(Randomnummer())) %(max_packetinterval*10))/10.0 + min_packetinterval;
      NS_LOG_UNCOND(Tid);
      Time interPacketInterval = Seconds (Tid);
      socket->Send (Create<Packet> (pktSize));
      string info = "We still goin stronk  " + to_string(pktCount);
      NS_LOG_UNCOND ("Sending one packet!");
      //double Tid =(rand() % 15 + 5)/10;
      
      pktInterval = interPacketInterval;
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);
      if (GTC < MaxChildren){
          GTC ++;
Simulator::Schedule (pktInterval, &GenerateTrafficChild,
                           socket, pktSize, numPacketChildren, pktInterval);
      NS_LOG_UNCOND ("Spawner et nyt monster");
      }
    }
  else
    {
      socket->Close ();
    }
}

int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  //double rss = -90;  // -dBm
  uint32_t packetSize = 1000; // bytes
  uint32_t numPackets = 20;
  numPacketChildren = numPackets;
  uint32_t sinkNode = 0; // Node der modtager (Gateway)
  uint32_t sourceNode = 1;
  numNodes = 30;
  double interval = 4.0; // seconds mellem hver pakke sendes
  //double interval_lower = 0.5;
  //double interval_higher = 2 - interval_lower;
  bool verbose = false;
  bool tracing = false;
  int nodeSpeed = 20;
  int nodePause = 0;
  int Run_number = 1;
  string File_name = "Results/OLSR/OLSR1";
  uint32_t step =100;
  unsigned int seed = 1234;
  uint32_t numGW = 1;
  
  /*Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Mode", StringValue ("Time"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Time", StringValue ("2s"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Bounds", StringValue ("0|200|0|200"));
*/
  
  CommandLine cmd;
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  //cmd.AddValue ("rss", "received signal strength", rss);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("numNodes", "number of nodes", numNodes);
  cmd.AddValue ("sinkNode", "Receiver node number", sinkNode);
  cmd.AddValue ("sourceNode", "Sender node number", sourceNode);
  cmd.AddValue ("File_name","Filens navn", File_name);
  cmd.AddValue ("Run_number", "Hvilket run vi er ved", Run_number);
  cmd.AddValue ("MaxChildren", "Hvor mange der sender", MaxChildren);
  cmd.AddValue ("min_random_interval"," Lowest number the randomizer will use", min_random_interval);
  cmd.AddValue ("max_random_interval"," Highest number the randomizer will use", max_random_interval);
  cmd.AddValue ("min_packetinterval"," Lowest number the randomizer will use", min_packetinterval);
  cmd.AddValue ("max_packetinterval"," Highest number the randomizer will use", max_packetinterval);
  cmd.Parse (argc, argv);
  

    RngSeedManager::SetSeed(seed);
    RngSeedManager::SetRun(Run_number);
  

  // The values returned by a uniformly distributed random
  // variable should always be within the range
  //
  //     [min, max)  .
  //

  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  
  
  NS_LOG_UNCOND ("Seed: " << seed );
  NS_LOG_UNCOND ("Run: " << Run_number );

  
  
  NodeContainer nodes;
  nodes.Create (numNodes-1);
  
  NodeContainer GW;
  GW.Create(numGW);
  
  NodeContainer c;
  c.Add(GW);
  c.Add(nodes);

  
  
  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-10) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  //wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);


  MobilityHelper mobility;
  int64_t streamIndex = 0;
  
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
  
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str ()),
                                  "Pause", StringValue (ssPause.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc));
  mobility.SetPositionAllocator (taPositionAlloc);
  mobility.Install (nodes);
  streamIndex += mobility.AssignStreams (nodes, streamIndex);
  NS_UNUSED (streamIndex);
  
  OlsrHelper aodv;
  Ipv4StaticRoutingHelper StaticRouting;
  
  Ipv4ListRoutingHelper list;
  list.Add (StaticRouting, 0);
  list.Add (aodv,10);
  aodv::RoutingProtocol testing;
  
  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (c);
  
  MobilityHelper GWmobility;
  GWmobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (200.0),
                                 "MinY", DoubleValue (200.0),
                                 "DeltaX", DoubleValue (step),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (numGW),
                                 "LayoutType", StringValue ("RowFirst"));
  GWmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  GWmobility.Install (GW);
  
  
  
  

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  
  
  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (sinkNode), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
  
  
  
  for (uint32_t v = 1; v < numNodes; v++){
  Ptr<Socket> SamletSource = Socket::CreateSocket (c.Get (v), tid);
  InetSocketAddress remote = InetSocketAddress (i.GetAddress (sinkNode, 0), 80);
  //InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
  //source->SetAllowBroadcast (true);
  SamletSource->Connect (remote);
  
  VectorSource.push_back(SamletSource);
  }
  
  Ptr<Socket> source = VectorSource[sourceNode]; 
    Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                 Seconds (5.0), &GenerateTraffic,
                                 source, packetSize, numPackets, interPacketInterval); 
    
//NS_LOG_UNCOND(book);
    
  if (tracing == true)
    {
      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("wifi-simple-adhoc-grid.tr"));
      //wifiPhy.EnablePcap ("wifi-simple-adhoc-grid", devices);
      // Trace routing tables
      //Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("wifi-simple-adhoc-grid.routes", std::ios::out);
      //aodv.PrintRoutingTableAllEvery (Seconds (2), routingStream);
      //Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper> ("wifi-simple-adhoc-grid.neighbors", std::ios::out);
      //aodv.PrintNeighborCacheAllEvery (Seconds (2), neighborStream);

      // To do-- enable an IP-level trace that shows forwarding events only
    }

  // Tracing
  string pcap = File_name + "/wifi-simple-adhoc";
  wifiPhy.EnablePcap (pcap, devices);
  
  string xml = File_name  + "/OLSR.xml";  
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();
  
  uint32_t Maxtid = (numPackets * (max_packetinterval+min_packetinterval)) + numPackets;
  NS_LOG_UNCOND(Maxtid);
  Simulator::Stop (Seconds (Maxtid));
  Simulator::Run ();
  flowMonitor->SerializeToXmlFile(xml, true, true);
  
  Simulator::Destroy ();

  return 0;
}



