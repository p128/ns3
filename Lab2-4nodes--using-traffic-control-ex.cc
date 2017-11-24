#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"


// Network topology
//
//       10.0.1.0       10.0.2.0
// n0 -------------- n2..........n3
//		     .
//	 10.0.3.0    .
//n1.................
//    point-to-point
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("4-node-Example");


int
main (int argc, char *argv[])
{
  double simulationTime = 10; //seconds
  std::string transportProt = "Udp";
  std::string socketType= "ns3::UdpSocketFactory";

  CommandLine cmd;
  cmd.Parse (argc, argv);

//Create nodes
  NodeContainer nodes;
  nodes.Create (4);

//gouping the nodes into containers according to links between them
	NodeContainer node0_1 = NodeContainer (nodes.Get (0), nodes.Get (1));
	NodeContainer node1_2 = NodeContainer (nodes.Get (1), nodes.Get (2));
	NodeContainer node3_1 = NodeContainer (nodes.Get (3), nodes.Get (1));

//Configiring p2p model
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  pointToPoint.SetQueue ("ns3::DropTailQueue", "Mode", StringValue ("QUEUE_MODE_PACKETS"), "MaxPackets", UintegerValue (1));

//Installing p2p model on the nodes
	NetDeviceContainer device0_1 = pointToPoint.Install (node0_1);
	NetDeviceContainer device1_2 = pointToPoint.Install (node1_2);
	NetDeviceContainer device3_1 = pointToPoint.Install (node3_1);

//Installing IP stack
  InternetStackHelper stack;
  stack.Install (nodes);

 //Now that we have set up devices and interfaces, its time to assign IP address to each of the interfaces.
	NS_LOG_INFO ("Assign IP Addresses.");
	Ipv4AddressHelper ip;
	ip.SetBase ("10.0.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interface0_1 = ip.Assign (device0_1);

	ip.SetBase ("10.0.2.0", "255.255.255.0");
	Ipv4InterfaceContainer interface1_2 = ip.Assign (device1_2);

	ip.SetBase ("10.0.3.0", "255.255.255.0");
	Ipv4InterfaceContainer interface3_1 = ip.Assign (device3_1);

//I. Configuring application (UDP) on nodes
  uint32_t payloadSize = 1448;
  OnOffHelper onoff (socketType, Ipv4Address::GetAny ());
  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  onoff.SetAttribute ("DataRate", StringValue ("50Mbps")); //bit/s
  
//Flow n0....n1
  uint16_t port = 7;

   //1. Install receiver (for packetsink) on node 1
  Address localAddress1 (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper1 (socketType, localAddress1);
  ApplicationContainer sinkApp1 = packetSinkHelper1.Install (nodes.Get (1));
  sinkApp1.Start (Seconds (0.0));
  sinkApp1.Stop (Seconds (simulationTime + 0.1));

  //2. Install sender app on node 0
  ApplicationContainer apps;
  AddressValue remoteAddress (InetSocketAddress (interface0_1.GetAddress (1), port));
  onoff.SetAttribute ("Remote", remoteAddress);
  apps.Add (onoff.Install (nodes.Get (0)));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (simulationTime + 0.1));


//Flow n1....n2
  //1. Install receiver (for packetsink) on node 2
  Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper (socketType, localAddress);
  ApplicationContainer sinkApp = packetSinkHelper.Install (nodes.Get (2));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (simulationTime + 0.1));

  //2. Install sender app on node 1
  ApplicationContainer apps1;
  AddressValue remoteAddress1 (InetSocketAddress (interface1_2.GetAddress (1), port));
  onoff.SetAttribute ("Remote", remoteAddress1);
  apps1.Add (onoff.Install (nodes.Get (1)));
  apps1.Start (Seconds (1.0));
  apps1.Stop (Seconds (simulationTime + 0.1));

//II. Configuring application (TCP) on nodes
  transportProt = "Tcp";
  socketType= "ns3::TcpSocketFactory";;
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));

  OnOffHelper onoff1 (socketType, Ipv4Address::GetAny ());
  onoff1.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff1.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  onoff1.SetAttribute ("DataRate", StringValue ("50Mbps")); //bit/s
  
//Flow n3....n1
  //1. Install receiver (for packetsink) on node 1
  Address localAddressT (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelperT (socketType, localAddressT);
  ApplicationContainer sinkAppT = packetSinkHelperT.Install (nodes.Get (1));
  sinkAppT.Start (Seconds (0.0));
  sinkAppT.Stop (Seconds (simulationTime + 0.1));

  //2. Install sender app on node 3
  ApplicationContainer appsT;
  AddressValue remoteAddressT (InetSocketAddress (interface3_1.GetAddress (1), port));
  onoff1.SetAttribute ("Remote", remoteAddressT);
  appsT.Add (onoff1.Install (nodes.Get (3)));
  appsT.Start (Seconds (1.0));
  appsT.Stop (Seconds (simulationTime + 0.1));


//Flow n1....n2
  //1. Install receiver (for packetsink) on node 2
  Address localAddressT1 (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelperT1 (socketType, localAddressT1);
  ApplicationContainer sinkAppT1 = packetSinkHelperT1.Install (nodes.Get (2));
  sinkAppT1.Start (Seconds (0.0));
  sinkAppT1.Stop (Seconds (simulationTime + 0.1));

  //2. Install sender app on node 1
  ApplicationContainer appsT1;
  AddressValue remoteAddressT1 (InetSocketAddress (interface1_2.GetAddress (1), port));
  onoff1.SetAttribute ("Remote", remoteAddressT1);
  appsT1.Add (onoff1.Install (nodes.Get (1)));
  appsT1.Start (Seconds (1.0));
  appsT1.Stop (Seconds (simulationTime + 0.1));

//Enable Tracing using flowmonitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

	AsciiTraceHelper ascii;
	Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("la2-4n.tr");
	pointToPoint.EnableAsciiAll (stream);

//Set when to stop simulator
  Simulator::Stop (Seconds (simulationTime + 5));

//Add visualization using Netanim
  AnimationInterface anim ("lab1-4n.xml"); 
  AnimationInterface::SetConstantPosition(nodes.Get(0), 1.0, 2.0);
  AnimationInterface::SetConstantPosition(nodes.Get(1), 2.0, 2.0);
  AnimationInterface::SetConstantPosition(nodes.Get(2), 3.0, 3.0);
  AnimationInterface::SetConstantPosition(nodes.Get(3), 1.0, 3.0);

//Run the simulator
  Simulator::Run ();

 // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
       NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
      NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
      NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
      NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps");
    }

   Simulator::Destroy ();

  
  return 0;
}
