#include <fstream>
#include <iostream>
#include <string>
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
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/pointer.h"
#include "ns3/olsr-helper.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/flow-monitor-helper.h"
#include <ctime>
#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("mainp6");


int GTC = 0;
int MaxChildren = 0;
int Run_number;
uint32_t numNodes;
uint32_t numPacketChildren;
std::vector<Ptr<Socket>> VectorSource;
float min_packetinterval = 0.5;
int max_packetinterval = 2;



void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet!");
    }
}

double Randomnummer(){
  
  double min_random_interval = 0.0;
  // the -1 is due numNodes-1 is the maount of nodes that may send. The *10 is in order to get the resolution used for intervals up to 1 decimal i.e 1.1, 1.2, 1.3, and so on
  double max_random_interval = static_cast<double>((numNodes-1)*10);    
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (min_random_interval));
  x->SetAttribute ("Max", DoubleValue (max_random_interval));
  double randgen = x->GetValue ();
  return randgen;
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
      std::string info = "We still goin stronk  " + std::to_string(pktCount);
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
  LogComponentEnable("mainp6", LOG_LEVEL_INFO);
  std::string phyMode ("DsssRate1Mbps");
  std::string protocol = "olsr";
  uint32_t packetSize = 1000; // bytes
  uint32_t numPackets = 20;
  numPacketChildren = numPackets;
  uint32_t sinkNode = 0;
  uint32_t sourceNode = 1;
  numNodes = 30;
  double interval = 4.0; // seconds mellem hver pakke sendes
  uint32_t numGW = 1;
  bool verbose = false;
  bool tracing = true;
  int nodeSpeed = 20;
  int nodePause = 0;
  uint32_t step =100;
  unsigned int seed = 1234;
  unsigned int runNumber = 1;
  

  CommandLine cmd;
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("protocol", "Pick routing protocol", protocol);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("numNodes", "number of nodes", numNodes);
  cmd.AddValue ("sinkNode", "Receiver node number", sinkNode);
  cmd.AddValue ("sourceNode", "Sender node number", sourceNode);
  cmd.AddValue ("runNumber", "Set the run number", runNumber);
  cmd.Parse (argc, argv);

  RngSeedManager::SetSeed(seed);
  RngSeedManager::SetRun(runNumber);
  
  double min = 0.0;
  double max = 10.0;
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (min));
  x->SetAttribute ("Max", DoubleValue (max));
  
  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  NS_LOG_INFO ("Seed: " << seed );
  NS_LOG_INFO ("Run: " << runNumber );
  
  NodeContainer c;
  c.Create (numNodes);
  NodeContainer GW;
  GW.Create (numGW);
  NodeContainer cGW = NodeContainer (GW.Get(0));
  cGW.Add(c);

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
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, cGW);

  // Note that with FixedRssLossModel, the positions below are not
  // used for received signal strength.
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
  mobility.Install (c);
  streamIndex += mobility.AssignStreams (c, streamIndex);
  NS_UNUSED (streamIndex);
  
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
  
  
  OlsrHelper olsr;
  AodvHelper aodv;
  DsdvHelper dsdv;
  DsrHelper dsr; // drs routing modul does not support flow monitor (require Ipv4 or Ipv6)
  DsrMainHelper dsrMain;
  Ipv4ListRoutingHelper list;
  InternetStackHelper internet;
  
  if (protocol == "dsr")
  {
    internet.Install (cGW);
    dsrMain.Install (dsr, cGW);
  }
  else if (protocol == "aodv")
  {
    list.Add (aodv, 10);
    internet.SetRoutingHelper (list);
    internet.Install (cGW);
    aodv.AssignStreams(cGW, 5);
  }
  else if (protocol == "olsr")
  {
    list.Add (olsr, 10);
    internet.SetRoutingHelper (list);
    internet.Install (cGW);
    olsr.AssignStreams(cGW, 5);
  }
  else if (protocol == "dsdv")
  {
    list.Add (dsdv, 10);
    internet.SetRoutingHelper (list);
    internet.Install (cGW);
    //dsdv.AssignStreams(cGW, 5);
  }
  else
  {
    NS_FATAL_ERROR ("No such protocol:" << protocol);
  }

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer Ipv4Int = ipv4.Assign (devices);


  /************* USE SIMULATOR TO SCHEDULE A EVENT ***************************/

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  for (uint32_t v = 1; v < numNodes; v++){
    Ptr<Socket> SamletSource = Socket::CreateSocket (c.Get (v), tid);
    InetSocketAddress remote = InetSocketAddress (Ipv4Int.GetAddress (0, 0), 80);
    SamletSource->Connect (remote);

    VectorSource.push_back(SamletSource);
  }

    Ptr<Socket> source = VectorSource[sourceNode];
    Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                  Seconds (5.0), &GenerateTraffic,
                                  source, packetSize, numPackets, interPacketInterval);


    if (tracing == true)
    {
      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("wifi-simple-adhoc-grid.tr"));
      wifiPhy.EnablePcap ("wifi-simple-adhoc-grid", devices);
    }

  

  // Output what we are doing
  NS_LOG_INFO ("Testing " << numPackets  ); 

  AnimationInterface anim ("adhocTest.xml"); // where "animation.xml" is any arbitrary
  
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  Simulator::Stop (Seconds (30.0));

  Simulator::Run ();
  flowMonitor->SerializeToXmlFile("flowmonitor.xml", true, true);
  Simulator::Destroy ();

  return 0;
}
