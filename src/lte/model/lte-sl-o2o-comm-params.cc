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

#include "lte-sl-o2o-comm-params.h"

#include <ns3/log.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>
#include <ns3/simulator.h>
#include <ns3/nstime.h>
#include "ns3/timer.h"
#include <ns3/uinteger.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteSlO2oCommParams");

NS_OBJECT_ENSURE_REGISTERED (LteSlO2oCommParams);

LteSlO2oCommParams::LteSlO2oCommParams ()
{
  NS_LOG_FUNCTION (this);

  m_relay_dT4111 = 0;
  m_relay_dT4103 = 0;
  m_relay_dT4108 = 0;
  m_relay_dTRUIR = 0;

  m_relay_T4111 = new Timer ();
  m_relay_T4103 = new Timer ();
  m_relay_T4108 = new Timer ();
  m_relay_TRUIR = new Timer ();

  m_remote_dT4100 = 0;
  m_remote_dT4101 = 0;
  m_remote_dT4102 = 0;
  m_remote_dT4103 = 0;

  m_remote_T4100 = new Timer ();
  m_remote_T4101 = new Timer ();
  m_remote_T4102 = new Timer ();
  m_remote_T4103 = new Timer ();

  m_contextId.contextId = 0;
  m_contextId.peerL2Id = 0;

  m_relay_DCR_count = 0;
  m_relay_RUIR_count = 0;
  m_remote_DCR_count = 0;
  m_remote_DCRq_count = 0;
  m_remote_DCK_count = 0;
  m_remote_ka_count = 0;

  m_stateString.push_back ("RELAY_IDLE");
  m_stateString.push_back ("RELAY_SETUP_REQUEST");
  m_stateString.push_back ("RELAY_SECURE_ESTABLISHED");
  //m_stateString.push_back ("RELAY_INIT_INFO");
  m_stateString.push_back ("RELAY_INIT_RELEASE");
  m_stateString.push_back ("REMOTE_IDLE");
  m_stateString.push_back ("REMOTE_INIT_SETUP");
  //m_stateString.push_back ("REMOTE_SECURE_LINK");
  m_stateString.push_back ("REMOTE_SECURE_ESTABLISHED");
  //m_stateString.push_back ("REMOTE_CHECK_LINK");
  m_stateString.push_back ("REMOTE_INIT_RELEASE");

  m_security_mode_id = 0;
}

LteSlO2oCommParams::~LteSlO2oCommParams ()
{
  NS_LOG_FUNCTION (this);
}

void
LteSlO2oCommParams::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

void
LteSlO2oCommParams::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId
LteSlO2oCommParams::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteSlO2oCommParams")
    .SetParent<Object> ()
    .AddConstructor<LteSlO2oCommParams> ()
    .AddAttribute ("relay_dT4111",
                   "Duration of Timer 4111 for Relay UE in milliseconds",
                   UintegerValue (200),
                   MakeUintegerAccessor (&LteSlO2oCommParams::SetRelayT4111Duration),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("relay_dT4103",
                   "Duration of Timer 4103 for Relay UE in milliseconds",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LteSlO2oCommParams::SetRelayT4103Duration),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("relay_dT4108",
                   "Duration of Timer 4108 (Wait for Keep Alive) for Relay UE in milliseconds",
                   UintegerValue (2000),
                   MakeUintegerAccessor (&LteSlO2oCommParams::SetRelayT4108Duration),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("relay_dTRUIR",
                   "Duration of Remote UE Info Request (RUIR) Timer for Relay UE in milliseconds",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LteSlO2oCommParams::SetRelayTRUIRDuration),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("relay_DCR_maximum",
                   "Maximum Value of Count of Direct Communication Release for Relay UE",
                   UintegerValue (3),
                   MakeUintegerAccessor (&LteSlO2oCommParams::m_relay_DCR_maximum),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("relay_RUIR_maximum",
                   "Maximum Value of Count of Remote UE Info Request for Relay UE",
                   UintegerValue (3),
                   MakeUintegerAccessor (&LteSlO2oCommParams::m_relay_RUIR_maximum),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("remote_dT4100",
                   "Duration of Timer 4100 for Remote UE in milliseconds",
                   UintegerValue (400),
                   MakeUintegerAccessor (&LteSlO2oCommParams::SetRemoteT4100Duration),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("remote_dT4101",
                   "Duration of Timer 4101 (Keep Alive Retx) for Remote UE in milliseconds",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LteSlO2oCommParams::SetRemoteT4101Duration),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("remote_dT4102",
                   "Duration of Timer 4102 (Keep Alive Tx) for Remote UE in milliseconds",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&LteSlO2oCommParams::SetRemoteT4102Duration),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("remote_dT4103",
                   "Duration of Timer 4103 for Remote UE in milliseconds",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LteSlO2oCommParams::SetRemoteT4103Duration),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("remote_DCR_maximum",
                   "Maximum Value of Count of Direct Communication Release for Remote UE",
                   UintegerValue (3),
                   MakeUintegerAccessor (&LteSlO2oCommParams::m_remote_DCR_maximum),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("remote_DCRq_maximum",
                   "Maximum Value of Count of Direct Communication Request for Remote UE",
                   UintegerValue (3),
                   MakeUintegerAccessor (&LteSlO2oCommParams::m_remote_DCRq_maximum),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("remote_DCK_maximum",
                   "Maximum Value of Count of Direct Communication Keepalive for Remote UE",
                   UintegerValue (3),
                   MakeUintegerAccessor (&LteSlO2oCommParams::m_remote_DCK_maximum),
                   MakeUintegerChecker<uint32_t>())
  ;
  return tid;
}

void
LteSlO2oCommParams::SetContextId (LteSlPc5ContextId contextId)
{
  NS_LOG_FUNCTION (this << contextId.peerL2Id << contextId.peerL2Id);
  m_contextId = contextId;
}

LteSlO2oCommParams::LteSlPc5ContextId
LteSlO2oCommParams::GetContextId ()
{
  NS_LOG_FUNCTION (this);
  return m_contextId;
}

void
LteSlO2oCommParams::ClearTimers ()
{
  NS_LOG_FUNCTION (this);

  if (GetRemoteT4100 ()->IsRunning ())
    {
      GetRemoteT4100 ()->Cancel ();
    }
  if (GetRemoteT4101 ()->IsRunning ())
    {
      GetRemoteT4101 ()->Cancel ();
    }
  if (GetRemoteT4102 ()->IsRunning ())
    {
      GetRemoteT4102 ()->Cancel ();
    }
  if (GetRemoteT4103 ()->IsRunning ())
    {
      GetRemoteT4103 ()->Cancel ();
    }
  if (GetRelayT4111 ()->IsRunning ())
    {
      GetRelayT4111 ()->Cancel ();
    }
  if (GetRelayT4103 ()->IsRunning ())
    {
      GetRelayT4103 ()->Cancel ();
    }
  if (GetRelayT4108 ()->IsRunning ())
    {
      GetRelayT4108 ()->Cancel ();
    }
  if (GetRelayTRUIR ()->IsRunning ())
    {
      GetRelayTRUIR ()->Cancel ();
    }
}

void
LteSlO2oCommParams::SetRelayT4111Duration (uint32_t duration)
{
  NS_LOG_FUNCTION (this << duration);

  m_relay_dT4111 = duration;
  m_relay_T4111->SetDelay (MilliSeconds (m_relay_dT4111));
}

void
LteSlO2oCommParams::SetRelayT4103Duration (uint32_t duration)
{
  NS_LOG_FUNCTION (this << duration);

  m_relay_dT4103 = duration;
  m_relay_T4103->SetDelay (MilliSeconds (m_relay_dT4103));
}

void
LteSlO2oCommParams::SetRelayT4108Duration (uint32_t duration)
{
  NS_LOG_FUNCTION (this << duration);

  m_relay_dT4108 = duration;
  m_relay_T4108->SetDelay (MilliSeconds (m_relay_dT4108));
}

void
LteSlO2oCommParams::SetRelayTRUIRDuration (uint32_t duration)
{
  NS_LOG_FUNCTION (this << duration);

  m_relay_dTRUIR = duration;
  m_relay_TRUIR->SetDelay (MilliSeconds (m_relay_dTRUIR));
}

Timer*
LteSlO2oCommParams::GetRelayT4111 ()
{
  return m_relay_T4111;
}

Timer*
LteSlO2oCommParams::GetRelayT4103 ()
{
  return m_relay_T4103;
}

Timer*
LteSlO2oCommParams::GetRelayT4108 ()
{
  return m_relay_T4108;
}

Timer*
LteSlO2oCommParams::GetRelayTRUIR ()
{
  return m_relay_TRUIR;
}

void
LteSlO2oCommParams::SetRemoteT4100Duration (uint32_t duration)
{
  NS_LOG_FUNCTION (this << duration);

  m_remote_dT4100 = duration;
  m_remote_T4100->SetDelay (MilliSeconds (m_remote_dT4100));
}

void
LteSlO2oCommParams::SetRemoteT4101Duration (uint32_t duration)
{
  NS_LOG_FUNCTION (this << duration);

  m_remote_dT4101 = duration;
  m_remote_T4101->SetDelay (MilliSeconds (m_remote_dT4101));
}

void
LteSlO2oCommParams::SetRemoteT4102Duration (uint32_t duration)
{
  NS_LOG_FUNCTION (this << duration);

  m_remote_dT4102 = duration;
  m_remote_T4102->SetDelay (MilliSeconds (m_remote_dT4102));

}

void
LteSlO2oCommParams::SetRemoteT4103Duration (uint32_t duration)
{
  NS_LOG_FUNCTION (this << duration);

  m_remote_dT4103 = duration;
  m_remote_T4103->SetDelay (MilliSeconds (m_remote_dT4103));
}

Timer*
LteSlO2oCommParams::GetRemoteT4100 ()
{
  return m_remote_T4100;
}

Timer*
LteSlO2oCommParams::GetRemoteT4101 ()
{
  return m_remote_T4101;
}

Timer*
LteSlO2oCommParams::GetRemoteT4102 ()
{
  return m_remote_T4102;
}

Timer*
LteSlO2oCommParams::GetRemoteT4103 ()
{
  return m_remote_T4103;
}

uint32_t
LteSlO2oCommParams::GetRelayDcrMax () const
{
  return m_relay_DCR_maximum;
}

uint32_t
LteSlO2oCommParams::GetRelayRuirMax () const
{
  return m_relay_RUIR_maximum;
}

uint32_t
LteSlO2oCommParams::GetRemoteDcrMax () const
{
  return m_remote_DCR_maximum;
}

uint32_t
LteSlO2oCommParams::GetRemoteDcrqMax () const
{
  return m_remote_DCRq_maximum;
}

uint32_t
LteSlO2oCommParams::GetRemoteDckMax () const
{
  return m_remote_DCK_maximum;
}

uint16_t
LteSlO2oCommParams::GetRelayDcrCount () const
{
  return m_relay_DCR_count;
}

void
LteSlO2oCommParams::SetRelayDcrCount (uint16_t value)
{
  m_relay_DCR_count = value;
}

uint16_t
LteSlO2oCommParams::GetRelayRuirCount () const
{
  return m_relay_RUIR_count;
}

void
LteSlO2oCommParams::SetRelayRuirCount (uint16_t value)
{
  m_relay_RUIR_count = value;
}

uint16_t
LteSlO2oCommParams::GetRemoteDcrCount () const
{
  return m_remote_DCR_count;
}

void
LteSlO2oCommParams::SetRemoteDcrCount (uint16_t value)
{
  m_remote_DCR_count = value;
}

uint16_t
LteSlO2oCommParams::GetRemoteDcrqCount () const
{
  return m_remote_DCRq_count;
}

void
LteSlO2oCommParams::SetRemoteDcrqCount (uint16_t value)
{
  m_remote_DCRq_count = value;
}

uint16_t
LteSlO2oCommParams::GetRemoteDckCount () const
{
  return m_remote_DCK_count;
}

void
LteSlO2oCommParams::SetRemoteDckCount (uint16_t value)
{
  m_remote_DCK_count = value;
}

uint16_t
LteSlO2oCommParams::GetRemoteKaCount () const
{
  return m_remote_ka_count;
}

void
LteSlO2oCommParams::SetRemoteKaCount (uint16_t value)
{
  m_remote_ka_count = value;
}

DirectCommunicationRequest
LteSlO2oCommParams::GetDcrqRetrans () const
{
  return m_dcrq_retrans;
}

void
LteSlO2oCommParams::SetDcrqRetrans (DirectCommunicationRequest value)
{
  m_dcrq_retrans = value;
}

DirectCommunicationRelease
LteSlO2oCommParams::GetDcrRetrans () const
{
  return m_dcr_retrans;
}

void
LteSlO2oCommParams::SetDcrRetrans (DirectCommunicationRelease value)
{
  m_dcr_retrans = value;
}

DirectCommunicationKeepalive
LteSlO2oCommParams::GetDckRetrans () const
{
  return m_dck_retrans;
}

void
LteSlO2oCommParams::SetDckRetrans (DirectCommunicationKeepalive value)
{
  m_dck_retrans = value;
}

RemoteUeInfoRequest
LteSlO2oCommParams::GetRuirqRetrans () const
{
  return m_ruirq_retrans;
}

void
LteSlO2oCommParams::SetRuirqRetrans (RemoteUeInfoRequest value)
{
  m_ruirq_retrans = value;
}

DirectSecurityModeCommand
LteSlO2oCommParams::GetDsmcmRetrans () const
{
  return m_dsmcm_retrans;
}

void
LteSlO2oCommParams::SetDsmcmRetrans (DirectSecurityModeCommand value)
{
  m_dsmcm_retrans = value;
}

void
LteSlO2oCommParams::SetDsmcmRetransLsb (uint8_t lsb)
{
  m_dsmcm_retrans.SetLsb (lsb);
}

LteSlO2oCommParams::DirectSecurityModeState
LteSlO2oCommParams::GetSecurityModeState () const
{
  return m_security_mode_state;
}

void
LteSlO2oCommParams::SetSecurityModeState (LteSlO2oCommParams::DirectSecurityModeState value)
{
  m_security_mode_state = value;
}

uint8_t
LteSlO2oCommParams::GetSecurityModeId () const
{
  return m_security_mode_id;
}

void
LteSlO2oCommParams::SetSecurityModeId (uint8_t value)
{
  m_security_mode_id = value;
}

LteSlO2oCommParams::UeO2OCommState
LteSlO2oCommParams::GetState () const
{
  return m_state;
}

void
LteSlO2oCommParams::SetState (LteSlO2oCommParams::UeO2OCommState value)
{
  m_state = value;
}

std::string
LteSlO2oCommParams::GetStateString (LteSlO2oCommParams::UeO2OCommState state)
{
  return m_stateString[state];
}




} // namespace ns3
