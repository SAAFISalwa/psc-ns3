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

#include "lte-sl-basic-ue-controller.h"
#include "ns3/epc-ue-nas.h"
#include "ns3/abort.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteSlBasicUeController");
NS_OBJECT_ENSURE_REGISTERED (LteSlBasicUeController);

LteSlBasicUeController::LteSlBasicUeController ()
  : m_connectingRelayUeId (std::numeric_limits<uint32_t>::max ())
{
  NS_LOG_FUNCTION (this);

  m_relayDiscProbRndVar = CreateObject<UniformRandomVariable> ();
  m_relayDiscProbRndVar->SetAttribute ("Min", DoubleValue (0.0));
  m_relayDiscProbRndVar->SetAttribute ("Max", DoubleValue (1.0));
}

LteSlBasicUeController::~LteSlBasicUeController ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
LteSlBasicUeController::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::LteSlBasicUeController")
    .SetParent<LteSlUeController> ()
    .SetGroupName ("Lte")
    .AddConstructor<LteSlBasicUeController> ()
    .AddAttribute ("NetDevice",
                   "The Lte NetDevice",
                   PointerValue (),
                   MakePointerAccessor (&LteSlBasicUeController::m_netDevice),
                   MakePointerChecker <LteUeNetDevice> ())
    .AddAttribute ("ProseHelper",
                   "The Lte Sidelink Helper",
                   PointerValue (),
                   MakePointerAccessor (&LteSlBasicUeController::m_lteSidelinkHelper),
                   MakePointerChecker <LteSidelinkHelper> ())
    .AddAttribute ("RelayDiscProb",
                   "The Selection Probability of a Relay for Communication once it is discovered",
                   DoubleValue (100),
                   MakeDoubleAccessor (&LteSlBasicUeController::m_relayDiscProb),
                   MakeDoubleChecker <double> ());
  return tid;
}

void
LteSlBasicUeController::DoDispose ()
{
}

void LteSlBasicUeController::RecvRelayServiceDiscovery (uint32_t serviceCode, uint64_t announcerInfo, uint32_t proseRelayUeId, uint8_t statusIndicator)
{
  NS_LOG_FUNCTION (this << serviceCode << announcerInfo << proseRelayUeId << statusIndicator);

  //Check if we are not connecting to a relay or already connected to one
  if (m_connectingRelayUeId == std::numeric_limits<uint32_t>::max ()
      && m_lteSlUeNetDeviceMap.empty ())
    {
      double randomValue = m_relayDiscProbRndVar->GetValue ();

      if (randomValue <= m_relayDiscProb / 100.0)
        {
          m_connectingRelayUeId = proseRelayUeId;
          m_slUeRrc->StartRelayDirectCommunication (serviceCode, proseRelayUeId);
        }
    }
}

void LteSlBasicUeController::Pc5SecuredEstablished (uint32_t peerUeId, uint32_t selfUeId, LteSlUeRrc::RelayRole role)
{
  NS_LOG_FUNCTION (this << peerUeId << selfUeId << role);

  //Make sure we don't already have a connection
  std::map < uint32_t, Ptr<LteSlUeNetDevice> >::iterator it = m_lteSlUeNetDeviceMap.find (peerUeId);
  if (m_connectingRelayUeId != std::numeric_limits<uint32_t>::max ())
    {
      NS_ASSERT_MSG (it == m_lteSlUeNetDeviceMap.end (), "PC5 connection between " << selfUeId << " and " << peerUeId << " already established");
      NS_ASSERT_MSG (m_connectingRelayUeId == peerUeId, "PC5 connection established to a relay it was not trying to connect");
      m_connectingRelayUeId = std::numeric_limits<uint32_t>::max ();
    }

  // If the UE is a Relay UE and has already a connection with that Remote UE,
  // tear it down and proceed to start a new one
  // This happens e.g., when the DirectCommunicationAccept is lost: the Relay established the connection
  // but not the Remote, and the Remote try to connect again before the local release (in the relay) of the previous connection

  // TS 24.334 10.4.2.6.2 Abnormal cases at the target UE
  // "... if the target UE already has an existing link established to the UE known to use this Layer 2 ID
  // and the new request contains an identical User Info as the known user, the UE shall process the new request. However, the target UE shall only delete
  // the existing link context after the new link setup procedure succeeds,
  // or the link keepalive procedure as described in subclause 10.4.3 fails.
  if (it != m_lteSlUeNetDeviceMap.end () && role == LteSlUeRrc::RelayUE)
    {
      //Terminate the current connection
      Pc5ConnectionTerminated (peerUeId, selfUeId, role);

      //Proceed to start the new one
      it = m_lteSlUeNetDeviceMap.end ();
    }

  if (it == m_lteSlUeNetDeviceMap.end ())
    {
      //Create a new UeNetDevice for sidelink communication
      Ptr<LteSlUeNetDevice> slNetDev = CreateObject<LteSlUeNetDevice> ();
      slNetDev->SetNas (m_netDevice->GetNas ());
      Ptr<Node> node = m_netDevice->GetNode ();
      slNetDev->SetNode (node);
      Mac64Address slNetDevAddress = Mac64Address::Allocate ();
      NS_LOG_DEBUG ("LteNetDevice address " << m_netDevice->GetAddress () << " Ipv6: " << node->GetObject<Ipv6L3Protocol> ()->GetAddress (1,1).GetAddress ());
      NS_LOG_DEBUG ("Created new LteSlNetDevice with address " << slNetDevAddress);
      slNetDev->SetAddress (slNetDevAddress);
      slNetDev->Initialize ();
      node->AddDevice (slNetDev);
      m_lteSlUeNetDeviceMap.insert (std::pair<uint32_t, Ptr<LteSlUeNetDevice> > (peerUeId, slNetDev));

      //Configure IP addresses as well as ingress and egress TFTs
      Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();

      if (role == LteSlUeRrc::RemoteUE)
        {
          Ipv6InterfaceContainer ifUp = m_lteSidelinkHelper->AssignIpv6AddressForRelayCommunication (slNetDev, peerUeId, selfUeId, role);
          uint32_t ipInterfaceIndex = ipv6->GetInterfaceForDevice (slNetDev);
          NS_LOG_DEBUG ("New LteSlNetDevice in remote node has interface index " << ipInterfaceIndex);

          //add route
          Ipv6Address ipv6a_relay = m_lteSidelinkHelper->GetRelayIpv6AddressFromMap (peerUeId, selfUeId, role);
          Ipv6StaticRoutingHelper ipv6RoutingHelper;
          Ptr<Ipv6StaticRouting> ue2ueStaticRouting = ipv6RoutingHelper.GetStaticRouting (ipv6);
          ue2ueStaticRouting->SetDefaultRoute (ipv6a_relay, ipInterfaceIndex);
          //ue2ueStaticRouting->AddNetworkRouteTo (ifUp.GetAddress (0,1), Ipv6Prefix(64), ipInterfaceIndex, 2);

          //add ingress filter so the NAS can send packets up via the right netdevice
          //accepts packets where the destination is his local address
          Ptr<LteSlTft> tft = Create<LteSlTft> (LteSlTft::RECEIVE, LteSlTft::LOCAL, ifUp.GetAddress (0,1), peerUeId);
          m_netDevice->GetNas ()->SetForwardUpCallback (tft, MakeCallback (&LteSlUeNetDevice::Receive, slNetDev));

          //configure TFT to be able to send packets through the sidelink radio bearer
          tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, LteSlTft::LOCAL, ifUp.GetAddress (0,1), peerUeId);
          m_netDevice->GetNas ()->ActivateSidelinkBearer (tft);
        }

      if (role == LteSlUeRrc::RelayUE)
        {
          Ipv6InterfaceContainer ifUp = m_lteSidelinkHelper->AssignIpv6AddressForRelayCommunication (slNetDev, selfUeId, peerUeId, role);
          uint32_t ipInterfaceIndex = ipv6->GetInterfaceForDevice (slNetDev);
          NS_LOG_DEBUG ("New LteSlNetDevice in relay node has interface index " << ipInterfaceIndex);
          //Since it is a relay, it needs to forward packets between sidelink and uplink/downlink netdevices
          ipv6->SetForwarding (ipInterfaceIndex, true);

          //Also the interface connecting to the network need to forward packets
          uint32_t ipInterfaceNetDevIndex = ipv6->GetInterfaceForDevice (m_netDevice);
          NS_LOG_DEBUG ("NetDevice in relay node has interface index " << ipInterfaceNetDevIndex);
          ipv6->SetForwarding (ipInterfaceNetDevIndex, true);


          //Ipv6StaticRoutingHelper ipv6RoutingHelper;
          //Ptr<Ipv6StaticRouting> ue2ueStaticRouting = ipv6RoutingHelper.GetStaticRouting (node->GetObject<Ipv6> ());
          //ue2ueStaticRouting->AddNetworkRouteTo (ifUp.GetAddress (0,1), Ipv6Prefix(64), "::",2, 0);

          //add ingress filter so the NAS can send packets up via the right netdevice
          //accepts packets coming from any address within the prefix used for relay
          Ptr<LteSlTft> tft = Create<LteSlTft> (LteSlTft::RECEIVE, LteSlTft::REMOTE, ifUp.GetAddress (0,1), Ipv6Prefix (64), peerUeId);
          m_netDevice->GetNas ()->SetForwardUpCallback (tft, MakeCallback (&LteSlUeNetDevice::Receive, slNetDev));

          //configure TFT to be able to send packets through the sidelink radio bearer
          tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, LteSlTft::REMOTE, ifUp.GetAddress (0,1), Ipv6Prefix (64), peerUeId);
          m_netDevice->GetNas ()->ActivateSidelinkBearer (tft);
        }
    }
}

void
LteSlBasicUeController::Pc5ConnectionTerminated (uint32_t peerUeId, uint32_t selfUeId, LteSlUeRrc::RelayRole role)
{
  NS_LOG_FUNCTION (this << peerUeId << selfUeId << role);

  std::map < uint32_t, Ptr<LteSlUeNetDevice> >::iterator it = m_lteSlUeNetDeviceMap.find (peerUeId);
  NS_ASSERT_MSG (it != m_lteSlUeNetDeviceMap.end (), "Could not find an associated interface");

  Ptr<LteSlUeNetDevice> slNetDev = it->second;

  Ptr<Node> node = m_netDevice->GetNode ();
  Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
  uint32_t ipInterfaceIndex = ipv6->GetInterfaceForDevice (slNetDev);
  ipv6->SetDown (ipInterfaceIndex);

  if (role == LteSlUeRrc::RemoteUE)
    {
      Ipv6Address ipv6a = m_lteSidelinkHelper->GetSelfIpv6AddressFromMap (peerUeId, selfUeId, role);

      Ptr<LteSlTft> tft = Create<LteSlTft> (LteSlTft::RECEIVE, LteSlTft::LOCAL, ipv6a, peerUeId);
      m_netDevice->GetNas ()->RemoveForwardUpCallback (tft);

      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, LteSlTft::REMOTE, ipv6a, Ipv6Prefix (64), peerUeId);
      m_netDevice->GetNas ()->DeactivateSidelinkBearer (tft);

      ipv6->RemoveAddress (ipInterfaceIndex, ipv6a);
    }

  if (role == LteSlUeRrc::RelayUE)
    {
      Ipv6Address ipv6a = m_lteSidelinkHelper->GetSelfIpv6AddressFromMap (selfUeId, peerUeId, role);

      Ptr<LteSlTft> tft = Create<LteSlTft> (LteSlTft::RECEIVE, LteSlTft::REMOTE, ipv6a, Ipv6Prefix (64), peerUeId);
      m_netDevice->GetNas ()->RemoveForwardUpCallback (tft);

      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, LteSlTft::REMOTE, ipv6a, Ipv6Prefix (64), peerUeId);
      m_netDevice->GetNas ()->DeactivateSidelinkBearer (tft);

      ipv6->RemoveAddress (ipInterfaceIndex, ipv6a);
    }
  node->RemoveDevice (slNetDev);
  m_lteSlUeNetDeviceMap.erase (peerUeId);

}

void
LteSlBasicUeController::Pc5ConnectionAborted (uint32_t peerUeId, uint32_t selfUeId, LteSlUeRrc::RelayRole role, LteSlO2oCommParams::UeO2ORejectReason reason)
{
  NS_LOG_FUNCTION (this << peerUeId << selfUeId << role << reason);

  NS_ASSERT (role == LteSlUeRrc::RemoteUE);
  if (m_connectingRelayUeId == peerUeId)
    {
      //reset connecting UE to enable reconnection attempt
      m_connectingRelayUeId = std::numeric_limits<uint32_t>::max ();
    }
}

void
LteSlBasicUeController::RecvRemoteUeReport (uint64_t localImsi, uint32_t peerUeId, uint64_t remoteImsi)
{
  NS_LOG_FUNCTION (this << localImsi << peerUeId << remoteImsi);

  //for now we let the core know about the new UE. Normally this is done by the relay node
  //by transmitting a Remote UE report (24.301 6.6.3)
  std::map < uint32_t, Ptr<LteSlUeNetDevice> >::iterator it = m_lteSlUeNetDeviceMap.find (peerUeId);
  NS_ASSERT_MSG (it != m_lteSlUeNetDeviceMap.end (), "Unknown remote UE id " << peerUeId);
  Ptr<Ipv6> ipv6 = m_netDevice->GetNode ()->GetObject<Ipv6> ();
  uint32_t ipInterfaceIndex = ipv6->GetInterfaceForDevice ((*it).second);

  //Now we can report the /64 IPv6 Prefix of the remote UE
  uint8_t relayAddr[16];
  ipv6->GetAddress (ipInterfaceIndex, 1).GetAddress ().GetBytes (relayAddr);
  uint8_t remotePrefix[8];
  std::memmove (remotePrefix, relayAddr, 8);

  m_lteSidelinkHelper->RemoteUeContextConnected (localImsi, remoteImsi, remotePrefix);

}

} // namespace ns3
