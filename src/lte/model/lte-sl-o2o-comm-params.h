/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef LTE_SL_O2O_COMM_PARAMS_H
#define LTE_SL_O2O_COMM_PARAMS_H

#include <ns3/object.h>
#include <ns3/nstime.h>
#include "ns3/timer.h"
#include <ns3/lte-sl-pc5-signalling-header.h>

namespace ns3 {

class LteSlO2oCommParams : public Object
{

public:
  /**
   * Structure to identify and link context
   * We use the sequence number of the connection request to identify each new request
   * That number is then used in all subsequent packets to "simulate" the fact that
   * packets are secured once the link is established.
   */
  struct LteSlPc5ContextId
  {
    uint32_t peerL2Id; ///< The L2 ID of the peer node
    uint32_t contextId; ///< The identifier for a given link with peer node

    /**
     * Comparison operator
     *
     * \param lhs first context
     * \param rhs second context
     * \returns true if the first context has lower value than the second
     */
    friend bool operator < (const LteSlPc5ContextId& lhs, const LteSlPc5ContextId& rhs)
    {
      return lhs.peerL2Id < rhs.peerL2Id || ( lhs.peerL2Id == rhs.peerL2Id && lhs.contextId < rhs.contextId);
    }
  };

  /**
   * Constructor
   */
  LteSlO2oCommParams ();

  /**
   * Destructor
   */
  virtual ~LteSlO2oCommParams ();

/**
 * Set the context ID
 * @param contextId The context ID
 */
  void SetContextId (LteSlPc5ContextId contextId);

/**
 * Get the context ID
 * \return The context ID
 */
  LteSlPc5ContextId GetContextId ();

/**
 * Clears all pending timers
 */
  void ClearTimers ();

// inherited from Object
private:
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

public:
/**
 * The Relay or Remote UE states
 */
  enum UeO2OCommState
  {
    RELAY_IDLE = 0,
    RELAY_SETUP_REQUEST,
    RELAY_SECURE_ESTABLISHED,
    //RELAY_INIT_INFO,
    RELAY_INIT_RELEASE,
    REMOTE_IDLE,
    REMOTE_INIT_SETUP,
    //REMOTE_SECURE_LINK,
    REMOTE_SECURE_ESTABLISHED,
    //REMOTE_CHECK_LINK,
    REMOTE_INIT_RELEASE
  };

/**
 * The possible reasons for rejecting a request from a remote UE
 */
  enum UeO2ORejectReason
  {
    COMM_NOT_ALLOWED = 1,
    AUTH_FAILURE,
    CONFLICT_L2_ID,
    LACK_OF_RESOURCES,
    IP_MISMATCH,
    OTHER_ERRORS
  };

/**
 * The state of the direct security mode procedure
 */
  enum DirectSecurityModeState
  {
    EMPTY = 0,
    COMMANDED,
    COMPLETED,
  };

/**
 * The possible reasons for releasing a link
 */
  enum UeO2OReleaseReason
  {
    COMM_NO_LONGER_NEEDED = 1,
    COMM_NO_LONGER_ALLOWED,
    COMM_NO_LONGER_AVAILABLE
  };

/**
 * The current state of the remote or relay UE
 */
  UeO2OCommState state;

/**
 * String representation of the current state
 */
  std::vector <std::string> stateString;

/**
 * The duration value (in ms) for T4111 timer
 */
  uint16_t relay_dT4111;

/**
 * The duration value (in ms) for T4103 timer
 */
  uint16_t relay_dT4103;

/**
 * The duration value (in ms) for T4108 timer
 */
  uint16_t relay_dT4108;

/**
 * The duration value (in ms) for TRUIR timer
 */
  uint16_t relay_dTRUIR;

/**
 * T4111 timer
 */
  Timer relay_T4111;

/**
 * T4103 timer
 */
  Timer relay_T4103;

/**
 * T4108 timer
 */
  Timer relay_T4108;

/**
 * TRUIR timer
 */
  Timer relay_TRUIR;

/**
 * The duration value (in ms) for T4100 timer
 */
  uint16_t remote_dT4100;

/**
 * The duration value (in ms) for T4101 timer
 */
  uint16_t remote_dT4101;

/**
 * The duration value (in ms) for T4102 timer
 */
  uint16_t remote_dT4102;

/**
 * The duration value (in ms) for T4103 timer
 */
  uint16_t remote_dT4103;

/**
 * T4100 timer
 */
  Timer remote_T4100;

/**
 * T4101 timer
 */
  Timer remote_T4101;

/**
 * T4102 timer
 */
  Timer remote_T4102;

/**
 * T4103 timer
 */
  Timer remote_T4103;

/**
 * The maximum values for DCR counter
 */
  uint16_t relay_DCR_maximum;

/**
 * The maximum values for RUIR counter
 */
  uint16_t relay_RUIR_maximum;

/**
 * The maximum values for DCR counter
 */
  uint16_t remote_DCR_maximum;

/**
 * The maximum values for DCRq counter
 */
  uint16_t remote_DCRq_maximum;

/**
 * The maximum values for DCK counter
 */
  uint16_t remote_DCK_maximum;

/**
 * The DCR counter for relay UE
 */
  uint16_t relay_DCR_count;

/**
 * The RUIR counter
 */
  uint16_t relay_RUIR_count;

/**
 * The DCR counter for remote UE
 */
  uint16_t remote_DCR_count;

/**
 * The DCRq counter
 */
  uint16_t remote_DCRq_count;

/**
 * The DCK counter
 */
  uint16_t remote_DCK_count;

/**
 * The KA counter
 */
  uint16_t remote_ka_count;

/**
 * Contains the copy of DirectCommunicationRequest for further retransmissions
 */
  DirectCommunicationRequest dcrq_retrans;

/**
 * Contains the copy of DirectCommunicationRelease for further retransmissions
 */
  DirectCommunicationRelease dcr_retrans;

/**
 * Contains the copy of DirectCommunicationKeepalive for further retransmissions
 */
  DirectCommunicationKeepalive dck_retrans;

/**
 * Contains the copy of RemoteUeInfoRequest for further retransmissions
 */
  RemoteUeInfoRequest ruirq_retrans;

/**
 * Contains the copy of DirectSecurityModeCommand for further retransmissions
 */
  DirectSecurityModeCommand dsmcm_retrans;

 /**
  * Flag to identify if the direct security mode control state of this context
  */
  DirectSecurityModeState security_mode_state;

 /**
  * ID to identify the security mode control related to a request TODO: Elaborate!
  */
  uint8_t security_mode_id;

/**
 * \brief Get the type ID.
 * \return the object TypeId
 */
  static TypeId GetTypeId (void);

/**
 * The context identifier
 */
  LteSlPc5ContextId m_contextId;

};

} // namespace ns3

#endif // LTE_SL_O2O_COMM_PARAMS_H
