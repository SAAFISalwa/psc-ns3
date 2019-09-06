/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * NIST-developed software is provided by NIST as a public
 * service. You may use, copy and distribute copies of the software in
 * any medium, provided that you keep intact this entire notice. You
 * may improve, modify and create derivative works of the software or
 * any portion of the software, and you may copy and distribute such
 * modifications or works. Modified works should carry a notice
 * stating that you changed the software and should note the date and
 * nature of any such change. Please explicitly acknowledge the
 * National Institute of Standards and Technology as the source of the
 * software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES
 * NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY
 * OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
 * WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED
 * OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT
 * WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 * SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 * CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of
 * using and distributing the software and you assume all risks
 * associated with its use, including but not limited to the risks and
 * costs of program errors, compliance with applicable laws, damage to
 * or loss of data, programs or equipment, and the unavailability or
 * interruption of operation. This software is not intended to be used
 * in any situation where a failure could cause risk of injury or
 * damage to property. The software developed by NIST employees is not
 * subject to copyright protection within the United States.
 */


#include "ns3/lte-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store.h"
#include <cfloat>
#include <sstream>
#include <math.h>
#include "ns3/gnuplot.h"
#include "ns3/udp-echo-client.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("lte-sl-relay-cluster");

//Trace sink function for logging when a packet is transmitted or received
void
UePacketTrace (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p, const Address &srcAddrs, const Address &dstAddrs)
{
  std::ostringstream oss;
  stream->GetStream ()->precision (6);

  *stream->GetStream () << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t"
                        << context << "\t"
                        << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 () << ":"
                        << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t"
                        << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 () << ":"
                        << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << "\t"
                        << p->GetSize () << "\t"
                        << std::endl;

}

void
GenerateTopologyPlotFile (NodeContainer enbNode, NodeContainer relayUeNodes, NodeContainer remoteUeNodes,
                          double relayRadius, double remoteRadius )
{
  std::string fileNameWithNoExtension = "topology";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string gnuplotFileName            = fileNameWithNoExtension + ".plt";
  std::string plotTitle               = "Topology (Labels = Node IDs)";

  Gnuplot plot (graphicsFileName);
  plot.SetTitle (plotTitle);
  plot.SetTerminal ("png size 1280,1024");
  plot.SetLegend ("X", "Y"); //These are the axis, not the legend
  std::ostringstream plotExtras;
  plotExtras << "set xrange [-" << 1.1 * (relayRadius + remoteRadius) << ":+" << 1.1 * (relayRadius + remoteRadius) << "]" << std::endl;
  plotExtras << "set yrange [-" << 1.1 * (relayRadius + remoteRadius) << ":+" << 1.1 * (relayRadius + remoteRadius) << "]" << std::endl;
  plotExtras << "set linetype 1 pt 3 ps 2 " << std::endl;
  plotExtras << "set linetype 2 lc rgb \"green\" pt 2 ps 2" << std::endl;
  plotExtras << "set linetype 3 pt 1 ps 2" << std::endl;
  plot.AppendExtra (plotExtras.str ());

  //eNB
  Gnuplot2dDataset datasetEnodeB;
  datasetEnodeB.SetTitle ("eNodeB");
  datasetEnodeB.SetStyle (Gnuplot2dDataset::POINTS);

  double x = enbNode.Get (0)->GetObject<MobilityModel> ()->GetPosition ().x;
  double y = enbNode.Get (0)->GetObject<MobilityModel> ()->GetPosition ().y;
  std::ostringstream strForLabel;
  strForLabel << "set label \"" << enbNode.Get (0)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"grey\" center front offset 0,1";
  plot.AppendExtra (strForLabel.str ());
  datasetEnodeB.Add (x, y);
  plot.AddDataset (datasetEnodeB);

  //Relay UEs
  Gnuplot2dDataset datasetRelays;
  datasetRelays.SetTitle ("Relay UEs");
  datasetRelays.SetStyle (Gnuplot2dDataset::POINTS);
  for (uint32_t ry = 0; ry < relayUeNodes.GetN (); ry++)
    {
      double x = relayUeNodes.Get (ry)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = relayUeNodes.Get (ry)->GetObject<MobilityModel> ()->GetPosition ().y;
      std::ostringstream strForLabel;
      strForLabel << "set label \"" << relayUeNodes.Get (ry)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"grey\" center front offset 0,1";
      plot.AppendExtra (strForLabel.str ());
      datasetRelays.Add (x, y);
    }
  plot.AddDataset (datasetRelays);

  //Remote UEs
  Gnuplot2dDataset datasetRemotes;
  datasetRemotes.SetTitle ("Remote UEs");
  datasetRemotes.SetStyle (Gnuplot2dDataset::POINTS);
  for (uint32_t rm = 0; rm < remoteUeNodes.GetN (); rm++)
    {
      double x = remoteUeNodes.Get (rm)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = remoteUeNodes.Get (rm)->GetObject<MobilityModel> ()->GetPosition ().y;
      std::ostringstream strForLabel;
      strForLabel << "set label \"" << remoteUeNodes.Get (rm)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"grey\" center front offset 0,1";
      plot.AppendExtra (strForLabel.str ());
      datasetRemotes.Add (x, y);
    }
  plot.AddDataset (datasetRemotes);

  std::ofstream plotFile (gnuplotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();
}

void
TraceSinkPC5SignalingPacketTrace (Ptr<OutputStreamWrapper> stream, uint32_t srcL2Id, uint32_t dstL2Id, Ptr<Packet> p)
{
  LteSlPc5SignallingMessageType lpc5smt;
  p->PeekHeader (lpc5smt);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << srcL2Id << "\t" << dstL2Id << "\t" << (uint32_t)lpc5smt.GetMessageType () << std::endl;
}

void
ChangeUdpEchoClientRemote (Ptr<Node> newRemoteNode, Ptr<UdpEchoClient> app, uint16_t port )
{

  Ptr<Ipv6> ipv6 = newRemoteNode->GetObject<Ipv6> ();
  //Get the interface used for SL ("7777:f00e::", Ipv6Prefix (48) is the base prefix for LteSlUeNetDevices )
  int32_t ipInterfaceIndex = ipv6->GetInterfaceForPrefix ("7777:f00e::", Ipv6Prefix (48));

  Ipv6Address remoteNodeSlIpAddress = newRemoteNode->GetObject<Ipv6L3Protocol> ()->GetAddress (ipInterfaceIndex,1).GetAddress ();
  NS_LOG_INFO (" Node id = [" << app->GetNode ()->GetId ()
                              << "] changed the UdpEchoClient Remote Ip Address to " << remoteNodeSlIpAddress);

  app->SetRemote (remoteNodeSlIpAddress, port);
}

/*
 * Scenario with configurable number of Relay UEs and Remote UEs.
 *
 * Topology:
 * The 'nRelayUes' Relay UEs are deployed around the eNB uniformly on a
 * circle of radius 'relayRadius' meters.
 * Each Relay UE has a cluster of 'nRemoteUesPerRelay' Remote UEs deployed
 * around itself uniformly on a circle of radius 'remoteRadius' meters.
 * --> The generated output file 'topology.plt' is a gnuplot script to plot
 * --> the topology of the scenario. To generate the plot run:
 * --> $ gnuplot topology.plt
 * --> and it will generate the image 'topology.png' with the nodes position.
 * The parameter 'remoteUesOoc' set to true indicates the Remote UEs are
 * out-of-coverage (not attached to the eNodeB and use SL preconfiguration).
 *
 * One-to-one connection:
 * The cluster of Remote UEs around a given Relay UE are interested only in its
 * Relay Service Code and thus will connect only to that Relay UE.
 * The UEs start their relay service sequentially in time. First the Relay UE,
 * then the cluster of Remote UEs associated to that Relay UE (sequentially as
 * well), then the next Relay UE, and so on.
 *
 * Traffic:
 * Each Remote UE sends packets to a given node in the network, which echoes
 * back each packet to the Remote UE, showcasing both upward and downward
 * traffic through the Relay UE.
 * The parameter 'echoServerNode' determines the node towards which the Remote
 * UEs send their traffic: either a Remote Host in the internet (when set to
 * 'RemoteHost') or the first Remote UE connected to the first Relay UE (when
 * set to 'RemoteUE').
 * Each transmitting Remote UE starts sending traffic 1.00 s after the start
 * of the one-to-one connection procedure with its Relay UE and remain active
 * during 10.0 s. The simulation time is calculated so that the last Remote UE
 * can have its 10.0s of traffic activity.
 */
int main (int argc, char *argv[])
{

  double simTime = 1.0; //s //Simulation time (in seconds) updated automatically based on number of nodes
  double relayRadius = 300.0; //m
  double remoteRadius = 50.0; //m
  uint32_t nRelayUes = 2;
  uint32_t nRemoteUesPerRelay = 1;
  bool remoteUesOoc = true;
  std::string echoServerNode ("RemoteHost");

  CommandLine cmd;

  cmd.AddValue ("relayRadius", "The radius of the circle (with center on the eNB) where the Relay UEs are positioned", relayRadius);
  cmd.AddValue ("remoteRadius", "The radius of the circle (with center on the Relay UE) where the Remote UEs are positioned", remoteRadius);
  cmd.AddValue ("nRelayUes", "Number of Relay UEs in the scenario", nRelayUes);
  cmd.AddValue ("nRemoteUesPerRelay", "Number of remote UEs per deployed Relay UE", nRemoteUesPerRelay);
  cmd.AddValue ("remoteUesOoc", "The Remote UEs are out-of-coverage", remoteUesOoc);
  cmd.AddValue ("echoServerNode", "The node towards which the Remote UE traffic is directed to (RemoteHost|RemoteUE)", echoServerNode);

  cmd.Parse (argc, argv);

  if (echoServerNode.compare ("RemoteHost") != 0 && echoServerNode.compare ("RemoteUE") != 0)
    {
      std::cout << "Wrong echoServerNode!. Options are (RemoteHost|RemoteUE)." << std::endl;
      return 1;
    }
  if (echoServerNode.compare ("RemoteUE") == 0 && nRelayUes * nRemoteUesPerRelay < 2)
    {
      std::cout << "At least 2 Remote UEs are needed when echoServerNode is a RemoteUE !" << std::endl;
      return 1;
    }


  //Calculate the start time of the relay service for Relay UEs and Remote UEs
  //Do it sequentially for easy of tractability
  double startTimeRelay[nRelayUes];
  double startTimeRemote[nRelayUes * nRemoteUesPerRelay];
  // The time between Relay UE's start of service
  // 1.0 s of baseline
  // 0.320 s to ensure sending the 1st discovery message
  // plus the time needed to all Remote UEs to connect to it:
  // (2+2*nRemoteUesPerRelay)*0.04 is the time in the worst case for all connection messages to go through between a Remote UE and a Relay UE:
  // 2 msgs from the Remote UE, each of them in 1 single SL period (0.04 s) and
  // 2 msgs from the Relay UE, which given the SL period RR scheduling, it can take in the worst case up to
  // nRemoteUesPerRelay SL periods to be sent
  double timeBetweenRelayStarts = 1.0 + nRemoteUesPerRelay * ((2 + 2 * nRemoteUesPerRelay) * 0.04); //s

  //The time between Remote UE's start of service
  //One discovery period (0.32 s) to avoid some of the congestion for the connection messages
  double timeBetweenRemoteStarts = 0.32; //s

  for (uint32_t ryIdx = 0; ryIdx < nRelayUes; ryIdx++)
    {
      startTimeRelay[ryIdx] = 2.0 + 0.320 + timeBetweenRelayStarts * ryIdx;

      NS_LOG_INFO ("Relay UE Idx " << ryIdx << " start time " << startTimeRelay[ryIdx] << "s");

      for (uint32_t rm = 0; rm < nRemoteUesPerRelay; ++rm)
        {
          uint32_t rmIdx = ryIdx * nRemoteUesPerRelay + rm;
          startTimeRemote[rmIdx] = startTimeRelay[ryIdx] + timeBetweenRemoteStarts * (rm + 1);
          NS_LOG_INFO ("Remote UE Idx " << rmIdx << " start time " << startTimeRemote[rmIdx] << "s");
        }
    }

  //Calculate simTime based on relay service starts and give 10 s of traffic for the last one
  simTime = startTimeRemote[(nRelayUes * nRemoteUesPerRelay - 1)] + 1.0 + 10.0; //s
  NS_LOG_INFO ("Simulation time = " << simTime << " s");

  NS_LOG_INFO ("Configuring default parameters...");

  //Configure the UE for UE_SELECTED scenario
  Config::SetDefault ("ns3::LteUeMac::SlGrantMcs", UintegerValue (16));
  Config::SetDefault ("ns3::LteUeMac::SlGrantSize", UintegerValue (6)); //The number of RBs allocated per UE for Sidelink
  Config::SetDefault ("ns3::LteUeMac::Ktrp", UintegerValue (1));
  Config::SetDefault ("ns3::LteUeMac::UseSetTrp", BooleanValue (false));
  Config::SetDefault ("ns3::LteUeMac::SlScheduler", StringValue ("Random")); //Values include Fixed, Random, MinPrb, MaxCoverage

  //Set the frequency
  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (5330));
  Config::SetDefault ("ns3::LteUeNetDevice::DlEarfcn", UintegerValue (5330));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", UintegerValue (23330));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (50));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (50));

  // Set error models
  Config::SetDefault ("ns3::LteSpectrumPhy::SlCtrlErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::SlDataErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlFullDuplexEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::DropRbOnCollisionEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteUePhy::DownlinkCqiPeriodicity", TimeValue (MilliSeconds (79)));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));

  //Set the UEs power in dBm
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (23.0));
  //Set the eNBs power in dBm
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (46.0));

  //Sidelink bearers activation time
  Time slBearersActivationTime = Seconds (2.0);

  //Create and set the helpers
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  Ptr<LteSidelinkHelper> proseHelper = CreateObject<LteSidelinkHelper> ();
  proseHelper->SetLteHelper (lteHelper);
  Config::SetDefault ("ns3::LteSlBasicUeController::ProseHelper", PointerValue (proseHelper));

  //Configure Timers
  //High keep-alive timers to maintain connection during simulation time
  Config::SetDefault ("ns3::LteSlO2oCommParams::relay_dT4108", UintegerValue (simTime * 1000));
  Config::SetDefault ("ns3::LteSlO2oCommParams::remote_dT4101", UintegerValue (simTime * 1000));
  Config::SetDefault ("ns3::LteSlO2oCommParams::remote_dT4102", UintegerValue (simTime * 1000));
  Config::SetDefault ("ns3::LteSlO2oCommParams::relay_dT4103", UintegerValue (simTime * 1000));
  Config::SetDefault ("ns3::LteSlO2oCommParams::relay_dTRUIR", UintegerValue (simTime * 1000));
  Config::SetDefault ("ns3::LteSlO2oCommParams::remote_dT4103", UintegerValue (simTime * 1000));
  Config::SetDefault ("ns3::LteSlO2oCommParams::remote_dT4100", UintegerValue (simTime * 1000));
  Config::SetDefault ("ns3::LteSlO2oCommParams::relay_dT4111", UintegerValue (simTime * 1000));

  //Set pathloss model
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));

  //Enable Sidelink
  lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

  //Create nodes (eNb + UEs)
  NodeContainer enbNode;
  enbNode.Create (1);
  NS_LOG_INFO ("eNb node id = [" << enbNode.Get (0)->GetId () << "]");

  NodeContainer relayUeNodes;
  relayUeNodes.Create (nRelayUes);

  NodeContainer remoteUeNodes;
  remoteUeNodes.Create (nRelayUes * nRemoteUesPerRelay);

  for (uint32_t ry = 0; ry < relayUeNodes.GetN (); ry++)
    {
      NS_LOG_INFO ("Relay UE " << ry + 1 << " node id = [" << relayUeNodes.Get (ry)->GetId () << "]");
    }
  for (uint32_t rm = 0; rm < remoteUeNodes.GetN (); rm++)
    {
      NS_LOG_INFO ("Remote UE " << rm + 1 << " node id = [" << remoteUeNodes.Get (rm)->GetId () << "]");
    }
  NodeContainer allUeNodes = NodeContainer (relayUeNodes,remoteUeNodes);

  //Position of the nodes
  //eNodeB
  Ptr<ListPositionAllocator> positionAllocEnb = CreateObject<ListPositionAllocator> ();
  positionAllocEnb->Add (Vector (0.0, 0.0, 30.0));

  //UEs
  Ptr<ListPositionAllocator> positionAllocRelays = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> positionAllocRemotes = CreateObject<ListPositionAllocator> ();
  for (uint32_t ry = 0; ry < relayUeNodes.GetN (); ++ry)
    {
      //Relay UE
      double ry_angle = ry * (360.0 / relayUeNodes.GetN ()); //degrees
      double ry_pos_x = std::floor (relayRadius * std::cos (ry_angle * M_PI / 180.0) );
      double ry_pos_y = std::floor (relayRadius * std::sin (ry_angle * M_PI / 180.0) );

      positionAllocRelays->Add (Vector (ry_pos_x, ry_pos_y, 1.5));

      NS_LOG_INFO ("Relay UE " << ry + 1 << " node id = [" << relayUeNodes.Get (ry)->GetId () << "]"
                   " x " << ry_pos_x << " y " << ry_pos_y);

      //Remote UEs
      for (uint32_t rm = 0; rm < nRemoteUesPerRelay; ++rm)
        {
          double rm_angle = rm * (360.0 / nRemoteUesPerRelay); //degrees
          double rm_pos_x = std::floor (ry_pos_x + remoteRadius * std::cos (rm_angle * M_PI / 180.0));
          double rm_pos_y = std::floor (ry_pos_y + remoteRadius * std::sin (rm_angle * M_PI / 180.0));

          positionAllocRemotes->Add (Vector (rm_pos_x, rm_pos_y, 1.5));

          uint32_t remoteIdx = ry * nRemoteUesPerRelay + rm;
          NS_LOG_INFO ("Remote UE " << remoteIdx << " node id = [" << remoteUeNodes.Get (remoteIdx)->GetId () << "]"
                       " x " << rm_pos_x << " y " << rm_pos_y);
        }
    }

  //Install mobility
  //eNodeB
  MobilityHelper mobilityeNodeB;
  mobilityeNodeB.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityeNodeB.SetPositionAllocator (positionAllocEnb);
  mobilityeNodeB.Install (enbNode);

  //Relay UEs
  MobilityHelper mobilityRelays;
  mobilityRelays.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityRelays.SetPositionAllocator (positionAllocRelays);
  mobilityRelays.Install (relayUeNodes);

  //Remote UE
  MobilityHelper mobilityRemotes;
  mobilityRemotes.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityRemotes.SetPositionAllocator (positionAllocRemotes);
  mobilityRemotes.Install (remoteUeNodes);

  //Generate gnuplot file with the script to generate the topology plot
  GenerateTopologyPlotFile (enbNode, relayUeNodes, remoteUeNodes, relayRadius, remoteRadius);

  //Install LTE devices to the nodes
  NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice (enbNode);
  NetDeviceContainer relayUeDevs = lteHelper->InstallUeDevice (relayUeNodes);
  NetDeviceContainer remoteUeDevs = lteHelper->InstallUeDevice (remoteUeNodes);
  NetDeviceContainer allUeDevs = NetDeviceContainer (relayUeDevs, remoteUeDevs);

  //Configure Sidelink
  Ptr<LteSlEnbRrc> enbSidelinkConfiguration = CreateObject<LteSlEnbRrc> ();
  enbSidelinkConfiguration->SetSlEnabled (true);

  //Configure communication pool
  LteRrcSap::SlCommTxResourcesSetup pool;

  pool.setup = LteRrcSap::SlCommTxResourcesSetup::UE_SELECTED;
  pool.ueSelected.havePoolToRelease = false;
  pool.ueSelected.havePoolToAdd = true;
  pool.ueSelected.poolToAddModList.nbPools = 1;
  pool.ueSelected.poolToAddModList.pools[0].poolIdentity = 1;

  LteSlResourcePoolFactory pfactory;
  //Control
  pfactory.SetControlPeriod ("sf40");
  pfactory.SetControlBitmap (0x00000000FF); //8 subframes for PSCCH
  pfactory.SetControlOffset (0);
  pfactory.SetControlPrbNum (22);
  pfactory.SetControlPrbStart (0);
  pfactory.SetControlPrbEnd (49);
  //Data
  pfactory.SetDataBitmap (0xFFFFFFFFFF);
  pfactory.SetDataOffset (8); //After 8 subframes of PSCCH
  pfactory.SetDataPrbNum (25);
  pfactory.SetDataPrbStart (0);
  pfactory.SetDataPrbEnd (49);

  pool.ueSelected.poolToAddModList.pools[0].pool =  pfactory.CreatePool ();

  //Add the pool as a default pool
  enbSidelinkConfiguration->SetDefaultPool (pool);

  //Configure discovery pool
  enbSidelinkConfiguration->SetDiscEnabled (true);

  LteRrcSap::SlDiscTxResourcesSetup discPool;
  discPool.setup =  LteRrcSap::SlDiscTxResourcesSetup::UE_SELECTED;
  discPool.ueSelected.havePoolToRelease = false;
  discPool.ueSelected.havePoolToAdd = true;
  discPool.ueSelected.poolToAddModList.nbPools = 1;
  discPool.ueSelected.poolToAddModList.pools[0].poolIdentity = 1;

  LteSlDiscResourcePoolFactory pDiscFactory;
  pDiscFactory.SetDiscCpLen ("NORMAL");
  pDiscFactory.SetDiscPeriod ("rf32");
  pDiscFactory.SetNumRetx (0);
  pDiscFactory.SetNumRepetition (1);
  pDiscFactory.SetDiscPrbNum (10);
  pDiscFactory.SetDiscPrbStart (10);
  pDiscFactory.SetDiscPrbEnd (40);
  pDiscFactory.SetDiscOffset (0);
  pDiscFactory.SetDiscBitmap (0x11111);
  pDiscFactory.SetDiscTxProbability ("p100");

  discPool.ueSelected.poolToAddModList.pools[0].pool =  pDiscFactory.CreatePool ();

  enbSidelinkConfiguration->AddDiscPool (discPool);

  //Install Sidelink configuration for eNBs
  lteHelper->InstallSidelinkConfiguration (enbDevs, enbSidelinkConfiguration);

  //Configure Sidelink Preconfiguration for the UEs
  Ptr<LteSlUeRrc> ueSidelinkConfiguration = CreateObject<LteSlUeRrc> ();
  ueSidelinkConfiguration->SetSlEnabled (true);

  LteRrcSap::SlPreconfiguration preconfiguration;
  ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);

  LteRrcSap::SlPreconfiguration preconfigurationRemote;
  LteRrcSap::SlPreconfiguration preconfigurationRelay;

  if (remoteUesOoc)
    {
      //General
      preconfigurationRemote.preconfigGeneral.carrierFreq = 23330;
      preconfigurationRemote.preconfigGeneral.slBandwidth = 50;

      //Discovery
      preconfigurationRemote.preconfigDisc.nbPools = 1;
      LteSlDiscPreconfigPoolFactory preconfDiscPoolFactory;
      preconfDiscPoolFactory.SetDiscCpLen ("NORMAL");
      preconfDiscPoolFactory.SetDiscPeriod ("rf32");
      preconfDiscPoolFactory.SetNumRetx (0);
      preconfDiscPoolFactory.SetNumRepetition (1);
      preconfDiscPoolFactory.SetDiscPrbNum (10);
      preconfDiscPoolFactory.SetDiscPrbStart (10);
      preconfDiscPoolFactory.SetDiscPrbEnd (40);
      preconfDiscPoolFactory.SetDiscOffset (0);
      preconfDiscPoolFactory.SetDiscBitmap (0x11111);
      preconfDiscPoolFactory.SetDiscTxProbability ("p100");

      preconfigurationRemote.preconfigDisc.pools[0] = preconfDiscPoolFactory.CreatePool ();

      //Communication
      preconfigurationRemote.preconfigComm.nbPools = 1;
      LteSlPreconfigPoolFactory preconfCommPoolFactory;
      //-Control
      preconfCommPoolFactory.SetControlPeriod ("sf40");
      preconfCommPoolFactory.SetControlBitmap (0x00000000FF); //8 subframes for PSCCH
      preconfCommPoolFactory.SetControlOffset (0);
      preconfCommPoolFactory.SetControlPrbNum (22);
      preconfCommPoolFactory.SetControlPrbStart (0);
      preconfCommPoolFactory.SetControlPrbEnd (49);
      //-Data
      preconfCommPoolFactory.SetDataBitmap (0xFFFFFFFFFF);
      preconfCommPoolFactory.SetDataOffset (8); //After 8 subframes of PSCCH
      preconfCommPoolFactory.SetDataPrbNum (25);
      preconfCommPoolFactory.SetDataPrbStart (0);
      preconfCommPoolFactory.SetDataPrbEnd (49);

      preconfigurationRemote.preconfigComm.pools[0] = preconfCommPoolFactory.CreatePool ();
    }

  ueSidelinkConfiguration->SetDiscEnabled (true);
  uint8_t nb = 3;
  ueSidelinkConfiguration->SetDiscTxResources (nb);
  ueSidelinkConfiguration->SetDiscInterFreq (enbDevs.Get (0)->GetObject<LteEnbNetDevice> ()->GetUlEarfcn ());

  ueSidelinkConfiguration->SetSlPreconfiguration (preconfigurationRelay);
  lteHelper->InstallSidelinkConfiguration (relayUeDevs, ueSidelinkConfiguration);

  ueSidelinkConfiguration->SetSlPreconfiguration (preconfigurationRemote);
  lteHelper->InstallSidelinkConfiguration (remoteUeDevs, ueSidelinkConfiguration);

  //Install the IP stack on the UEs and assign IP address
  internet.Install (relayUeNodes);
  internet.Install (remoteUeNodes);

  Ipv6InterfaceContainer ueIpIfaceRelays;
  Ipv6InterfaceContainer ueIpIfaceRemotes;
  ueIpIfaceRelays = epcHelper->AssignUeIpv6Address (relayUeDevs);
  ueIpIfaceRemotes = epcHelper->AssignUeIpv6Address (remoteUeDevs);

  //Define and set routing
  Ipv6StaticRoutingHelper Ipv6RoutingHelper;
  for (uint32_t u = 0; u <  allUeNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = allUeNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv6StaticRouting> ueStaticRouting = Ipv6RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv6> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress6 (), 1);
    }

  Ipv6AddressHelper ipv6h;
  ipv6h.SetBase (Ipv6Address ("6001:db80::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer internetIpIfaces = ipv6h.Assign (internetDevices);

  internetIpIfaces.SetForwarding (0, true);
  internetIpIfaces.SetDefaultRouteInAllNodes (0);

  Ipv6StaticRoutingHelper ipv6RoutingHelper;
  Ptr<Ipv6StaticRouting> remoteHostStaticRouting = ipv6RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv6> ());
  remoteHostStaticRouting->AddNetworkRouteTo ("7777:f000::", Ipv6Prefix (60), internetIpIfaces.GetAddress (0, 1), 1, 0);

  //Routing downward traffic for the Remote UEs
  Ptr<Ipv6StaticRouting> pgwStaticRouting = ipv6RoutingHelper.GetStaticRouting (pgw->GetObject<Ipv6> ());
  pgwStaticRouting->AddNetworkRouteTo ("7777:f00e::", Ipv6Prefix (48), Ipv6Address ("::"), 1, 0);

  //Attach Relay UEs to the eNB
  lteHelper->Attach (relayUeDevs);
  //If the Remote UEs are not OOC attach them to the eNodeB as well
  if (!remoteUesOoc)
    {
      lteHelper->Attach (remoteUeDevs);
    }

  ///*** Configure applications ***///
  //For each Remote UE, we have a pair (UpdEchoClient, UdpEchoServer)
  //Each Remote UE has an assigned port
  //UdpEchoClient installed in the Remote UE, sending to the echoServerAddr
  //and to the corresponding Remote UE port
  //UdpEchoServer installed in the echoServerNode, listening to the
  //corresponding Remote UE port

  Ipv6Address echoServerAddr;

  if (echoServerNode.compare ("RemoteHost") == 0)
    {
      echoServerAddr = internetIpIfaces.GetAddress (1, 1);
    }
  else if (echoServerNode.compare ("RemoteUE") == 0)
    {
      // We use a dummy IP address for initial configuration as we don't know the
      // IP address of the 'Remote UE (0)' before it connects to the Relay UE
      echoServerAddr = Ipv6Address::GetOnes ();
    }
  uint16_t echoPortBase = 50000;
  ApplicationContainer serverApps;
  ApplicationContainer clientApps;
  AsciiTraceHelper ascii;

  std::ostringstream oss;
  Ptr<OutputStreamWrapper> packetOutputStream = ascii.CreateFileStream ("AppPacketTrace.txt");
  *packetOutputStream->GetStream () << "time(sec)\ttx/rx\tC/S\tNodeID\tIP[src]\tIP[dst]\tPktSize(bytes)" << std::endl;

  for (uint16_t remUeIdx = 0; remUeIdx < remoteUeNodes.GetN (); remUeIdx++)
    {
      if (echoServerNode.compare ("RemoteUE") == 0 && remUeIdx == 0)
        {
          //No own traffic applications for Remote UE (0) as it is the echoServerNode
          continue;
        }

      uint16_t remUePort = echoPortBase + remUeIdx;
      uint32_t echoServerNodeId = 0;
      //UdpEchoServer listening in the Remote UE port
      UdpEchoServerHelper echoServerHelper (remUePort);
      ApplicationContainer singleServerApp;
      if (echoServerNode.compare ("RemoteHost") == 0)
        {
          singleServerApp.Add (echoServerHelper.Install (remoteHost));
          echoServerNodeId = remoteHost->GetId ();
        }
      else if (echoServerNode.compare ("RemoteUE") == 0)
        {
          singleServerApp.Add (echoServerHelper.Install (remoteUeNodes.Get (0)));
          echoServerNodeId = remoteUeNodes.Get (0)->GetId ();

        }
      singleServerApp.Start (Seconds (1.0));
      singleServerApp.Stop (Seconds (simTime));

      //Tracing packets on the UdpEchoServer (S)
      oss << "rx\tS\t" << echoServerNodeId;
      singleServerApp.Get (0)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, packetOutputStream));
      oss.str ("");
      oss << "tx\tS\t" << echoServerNodeId;
      singleServerApp.Get (0)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, packetOutputStream));
      oss.str ("");

      serverApps.Add (singleServerApp);

      //UdpEchoClient in the Remote UE
      UdpEchoClientHelper echoClientHelper (echoServerAddr);
      echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (20));
      echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
      echoClientHelper.SetAttribute ("PacketSize", UintegerValue (150));
      echoClientHelper.SetAttribute ("RemotePort", UintegerValue (remUePort));

      ApplicationContainer singleClientApp = echoClientHelper.Install (remoteUeNodes.Get (remUeIdx));
      //Start the application 1.0 s after the remote UE started the relay service
      //normally it should be enough time to connect
      singleClientApp.Start (Seconds (1.0 + startTimeRemote[remUeIdx]) );
      //Stop the application after 10.0 s
      singleClientApp.Stop (Seconds (1.0 + startTimeRemote[remUeIdx] + 10.0));

      //Tracing packets on the UdpEchoClient (C)
      oss << "tx\tC\t" << remoteUeNodes.Get (remUeIdx)->GetId ();
      singleClientApp.Get (0)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, packetOutputStream));
      oss.str ("");
      oss << "rx\tC\t" << remoteUeNodes.Get (remUeIdx)->GetId ();
      singleClientApp.Get (0)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, packetOutputStream));
      oss.str ("");

      clientApps.Add (singleClientApp);

      if (echoServerNode.compare ("RemoteUE") == 0 && remUeIdx != 0)
        {
          //Schedule the change of the RemoteAddress to 100 ms before the start of the application
          //normally 'Remote UE (0)' should be already connected to its Relay UE so we can
          //assign its address as RemoteAddress
          Simulator::Schedule (Seconds (1.0 + startTimeRemote[remUeIdx] - 0.100), &ChangeUdpEchoClientRemote, remoteUeNodes.Get (0),
                               singleClientApp.Get (0)->GetObject<UdpEchoClient> (), remUePort );
        }
    }

  ///*** End of application configuration ***///

  ///*** Configure Relaying ***///
  Ptr<OutputStreamWrapper> PC5SignalingPacketTraceStream = ascii.CreateFileStream ("PC5SignalingPacketTrace.txt");
  *PC5SignalingPacketTraceStream->GetStream () << "time(s)\ttxId\tRxId\tmsgType" << std::endl;

  proseHelper->SetIpv6BaseForRelayCommunication ("7777:f00e::", Ipv6Prefix (48));

  Ptr<EpcTft> tft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpf;
  dlpf.localIpv6Address.Set ("7777:f00e::");
  dlpf.localIpv6Prefix = Ipv6Prefix (32);
  tft->Add (dlpf);
  EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
  lteHelper->ActivateDedicatedEpsBearer (relayUeDevs, bearer, tft);

  for (uint32_t ryDevIdx = 0; ryDevIdx < relayUeDevs.GetN (); ryDevIdx++)
    {
      uint32_t serviceCode = relayUeDevs.Get (ryDevIdx)->GetObject<LteUeNetDevice> ()->GetImsi ();

      Simulator::Schedule (Seconds (startTimeRelay[ryDevIdx]), &LteSidelinkHelper::StartRelayService, proseHelper, relayUeDevs.Get (ryDevIdx), serviceCode, LteSlUeRrc::ModelA, LteSlUeRrc::RelayUE);
      NS_LOG_INFO ("Relay UE " << ryDevIdx << " node id = [" << relayUeNodes.Get (ryDevIdx)->GetId () << "] provides Service Code " << serviceCode << " and start service at " << startTimeRelay[ryDevIdx] << " s");
      //Remote UEs
      for (uint32_t rm = 0; rm < nRemoteUesPerRelay; ++rm)
        {
          uint32_t rmDevIdx = ryDevIdx * nRemoteUesPerRelay + rm;
          Simulator::Schedule ((Seconds (startTimeRemote[rmDevIdx])), &LteSidelinkHelper::StartRelayService, proseHelper, remoteUeDevs.Get (rmDevIdx), serviceCode, LteSlUeRrc::ModelA, LteSlUeRrc::RemoteUE);
          NS_LOG_INFO ("Remote UE " << rmDevIdx << " node id = [" << remoteUeNodes.Get (rmDevIdx)->GetId () << "] interested in Service Code " << serviceCode << " and start service at " << startTimeRemote[rmDevIdx] << " s");
        }
    }

  //Tracing relay connection messages
  for (uint32_t ueDevIdx = 0; ueDevIdx < relayUeDevs.GetN (); ueDevIdx++)
    {
      Ptr<LteUeRrc> rrc = relayUeDevs.Get (ueDevIdx)->GetObject<LteUeNetDevice> ()->GetRrc ();
      PointerValue ptrOne;
      rrc->GetAttribute ("SidelinkConfiguration", ptrOne);
      Ptr<LteSlUeRrc> slUeRrc = ptrOne.Get<LteSlUeRrc> ();
      slUeRrc->TraceConnectWithoutContext ("PC5SignalingPacketTrace",
                                           MakeBoundCallback (&TraceSinkPC5SignalingPacketTrace,
                                                              PC5SignalingPacketTraceStream));
    }
  for (uint32_t ueDevIdx = 0; ueDevIdx < remoteUeDevs.GetN (); ueDevIdx++)
    {
      Ptr<LteUeRrc> rrc = remoteUeDevs.Get (ueDevIdx)->GetObject<LteUeNetDevice> ()->GetRrc ();
      PointerValue ptrOne;
      rrc->GetAttribute ("SidelinkConfiguration", ptrOne);
      Ptr<LteSlUeRrc> slUeRrc = ptrOne.Get<LteSlUeRrc> ();
      slUeRrc->TraceConnectWithoutContext ("PC5SignalingPacketTrace",
                                           MakeBoundCallback (&TraceSinkPC5SignalingPacketTrace,
                                                              PC5SignalingPacketTraceStream));
    }
  ///*** END Configure Relaying ***///

  lteHelper->EnablePdcpTraces ();
  lteHelper->EnableSlRxPhyTraces ();

  NS_LOG_INFO ("Simulation time " << simTime << " s");
  NS_LOG_INFO ("Starting simulation...");

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;

}
