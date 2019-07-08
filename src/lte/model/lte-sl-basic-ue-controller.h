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

#ifndef LTE_SL_BASIC_UE_CONTROLLER_H
#define LTE_SL_BASIC_UE_CONTROLLER_H

#include <ns3/lte-sl-ue-controller.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/lte-sl-ue-net-device.h>
#include <ns3/lte-sidelink-helper.h>
#include "ns3/internet-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \brief The abstract base class of a UE sidelink controller.
 *
 */
class LteSlBasicUeController : public LteSlUeController
{

public:
  LteSlBasicUeController ();
  virtual ~LteSlBasicUeController ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  // inherited from LteSlUeController
  virtual void RecvRelayServiceDiscovery (uint32_t serviceCode, uint64_t announcerInfo, uint32_t proseRelayUeId, uint8_t statusIndicator);
  virtual void Pc5SecuredEstablished (uint32_t remoteUeId, uint32_t selfUeId, LteSlUeRrc::RelayRole role);
  virtual void Pc5ConnectionTerminated (uint32_t remoteUeId, uint32_t selfUeId, LteSlUeRrc::RelayRole role);
  virtual void Pc5ConnectionAborted (uint32_t peerUeId, uint32_t selfUeId, LteSlUeRrc::RelayRole role, LteSlO2oCommParams::UeO2ORejectReason reason);
  virtual void RecvRemoteUeReport (uint64_t localImsi, uint32_t peerUeId, uint64_t remoteImsi);

protected:
  // inherited from Object
  virtual void DoDispose ();

  /** The UE netdevice to access different layers of the UE stack */
  Ptr <LteUeNetDevice> m_netDevice;

  /** The helper to assist with address configuration */
  Ptr <LteSidelinkHelper> m_lteSidelinkHelper;

  /** Map the relay UE layer 2 ID and the NetDevice used to communicate */
  std::map < uint32_t, Ptr<LteSlUeNetDevice> > m_lteSlUeNetDeviceMap;

  /** Probability (in %) to connect to a newly discovery relay if not already connected */
  uint16_t m_relayDiscProb;

  /** Random variable to decide initial relay connection */
  Ptr<UniformRandomVariable> m_relayDiscProbRndVar;

  /** Indicate the relay UE L2 ID the node is trying to connect */
  uint32_t m_connectingRelayUeId;

}; // end of class LteSlUeController

} // namespace ns3

#endif /* LTE_SL_BASIC_UE_CONTROLLER_H */
