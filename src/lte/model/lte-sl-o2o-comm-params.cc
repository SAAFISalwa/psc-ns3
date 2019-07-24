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

#include "lte-sl-o2o-comm-params.h"

#include <ns3/log.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>
#include <ns3/simulator.h>
#include <ns3/nstime.h>
#include "ns3/timer.h"
#include <ns3/double.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteSlO2oCommParams");

NS_OBJECT_ENSURE_REGISTERED (LteSlO2oCommParams);

LteSlO2oCommParams::LteSlO2oCommParams ()
{
  NS_LOG_FUNCTION (this);

  m_contextId.contextId = 0;
  m_contextId.peerL2Id = 0;

  relay_DCR_count = 0;
  relay_RUIR_count = 0;
  remote_DCR_count = 0;
  remote_DCRq_count = 0;
  remote_DCK_count = 0;
  remote_ka_count = 0;

  stateString.push_back ("RELAY_IDLE");
  stateString.push_back ("RELAY_SETUP_REQUEST");
  stateString.push_back ("RELAY_SECURE_ESTABLISHED");
  //stateString.push_back ("RELAY_INIT_INFO");
  stateString.push_back ("RELAY_INIT_RELEASE");
  stateString.push_back ("REMOTE_IDLE");
  stateString.push_back ("REMOTE_INIT_SETUP");
  //stateString.push_back ("REMOTE_SECURE_LINK");
  stateString.push_back ("REMOTE_SECURE_ESTABLISHED");
  //stateString.push_back ("REMOTE_CHECK_LINK");
  stateString.push_back ("REMOTE_INIT_RELEASE");

  security_mode_id = 0;
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
                   DoubleValue (200),
                   MakeDoubleAccessor (&LteSlO2oCommParams::relay_dT4111),
                   MakeDoubleChecker<double>())
    .AddAttribute ("relay_dT4103",
                   "Duration of Timer 4103 for Relay UE in milliseconds",
                   DoubleValue (100),
                   MakeDoubleAccessor (&LteSlO2oCommParams::relay_dT4103),
                   MakeDoubleChecker<double>())
    .AddAttribute ("relay_dT4108",
                   "Duration of Timer 4108 (Wait for Keep Alive) for Relay UE in milliseconds",
                   DoubleValue (2000),
                   MakeDoubleAccessor (&LteSlO2oCommParams::relay_dT4108),
                   MakeDoubleChecker<double>())
    .AddAttribute ("relay_dTRUIR",
                   "Duration of Remote UE Info Request (RUIR) Timer for Relay UE in milliseconds",
                   DoubleValue (100),
                   MakeDoubleAccessor (&LteSlO2oCommParams::relay_dTRUIR),
                   MakeDoubleChecker<double>())
    .AddAttribute ("relay_DCR_maximum",
                   "Maximum Value of Count of Direct Communication Release for Relay UE",
                   DoubleValue (3),
                   MakeDoubleAccessor (&LteSlO2oCommParams::relay_DCR_maximum),
                   MakeDoubleChecker<double>())
    .AddAttribute ("relay_RUIR_maximum",
                   "Maximum Value of Count of Remote UE Info Request for Relay UE",
                   DoubleValue (3),
                   MakeDoubleAccessor (&LteSlO2oCommParams::relay_RUIR_maximum),
                   MakeDoubleChecker<double>())
    .AddAttribute ("remote_dT4100",
                   "Duration of Timer 4100 for Remote UE in milliseconds",
                   DoubleValue (400),
                   MakeDoubleAccessor (&LteSlO2oCommParams::remote_dT4100),
                   MakeDoubleChecker<double>())
    .AddAttribute ("remote_dT4101",
                   "Duration of Timer 4101 (Keep Alive Retx) for Remote UE in milliseconds",
                   DoubleValue (100),
                   MakeDoubleAccessor (&LteSlO2oCommParams::remote_dT4101),
                   MakeDoubleChecker<double>())
    .AddAttribute ("remote_dT4102",
                   "Duration of Timer 4102 (Keep Alive Tx) for Remote UE in milliseconds",
                   DoubleValue (1000),
                   MakeDoubleAccessor (&LteSlO2oCommParams::remote_dT4102),
                   MakeDoubleChecker<double>())
    .AddAttribute ("remote_dT4103",
                   "Duration of Timer 4103 for Remote UE in milliseconds",
                   DoubleValue (100),
                   MakeDoubleAccessor (&LteSlO2oCommParams::remote_dT4103),
                   MakeDoubleChecker<double>())
    .AddAttribute ("remote_DCR_maximum",
                   "Maximum Value of Count of Direct Communication Release for Remote UE",
                   DoubleValue (3),
                   MakeDoubleAccessor (&LteSlO2oCommParams::remote_DCR_maximum),
                   MakeDoubleChecker<double>())
    .AddAttribute ("remote_DCRq_maximum",
                   "Maximum Value of Count of Direct Communication Request for Remote UE",
                   DoubleValue (3),
                   MakeDoubleAccessor (&LteSlO2oCommParams::remote_DCRq_maximum),
                   MakeDoubleChecker<double>())
    .AddAttribute ("remote_DCK_maximum",
                   "Maximum Value of Count of Direct Communication Keepalive for Remote UE",
                   DoubleValue (3),
                   MakeDoubleAccessor (&LteSlO2oCommParams::remote_DCK_maximum),
                   MakeDoubleChecker<double>())
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

  if (remote_T4100.IsRunning ())
    {
      remote_T4100.Cancel ();
    }

  if (remote_T4101.IsRunning ())
    {
      remote_T4101.Cancel ();
    }

  if (remote_T4102.IsRunning ())
    {
      remote_T4102.Cancel ();
    }

  if (remote_T4103.IsRunning ())
    {
      remote_T4103.Cancel ();
    }

  if (relay_T4111.IsRunning ())
    {
      relay_T4111.Cancel ();
    }

  if (relay_T4103.IsRunning ())
    {
      relay_T4103.Cancel ();
    }
  if (relay_T4108.IsRunning ())
    {
      relay_T4108.Cancel ();
    }
  if (relay_TRUIR.IsRunning ())
    {
      relay_TRUIR.Cancel ();
    }
}

} // namespace ns3
