/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Nicola Baldo <nbaldo@cttc.es>
 * Modified by:
 *          Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *          Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 */

#ifndef LTE_SL_UE_NET_DEVICE_H
#define LTE_SL_UE_NET_DEVICE_H

#include "ns3/lte-net-device.h"
#include "ns3/event-id.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/lte-phy.h"
#include "ns3/eps-bearer.h"
#include "ns3/component-carrier-ue.h"
#include <vector>
#include <map>

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
class LtePhy;
class LteUePhy;
class LteEnbNetDevice;
class LteUeMac;
class LteUeRrc;
class EpcUeNas;
class EpcTft;
class LteUeComponentCarrierManager;

/**
 * \ingroup lte
 * The LteSlUeNetDevice class implements the UE net device
 */
class LteSlUeNetDevice : public LteNetDevice
{

public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  LteSlUeNetDevice (void);
  virtual ~LteSlUeNetDevice (void);
  virtual void DoDispose ();

  // inherited from NetDevice
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

  /**
   * \brief Set the NAS.
   * \param nas the LTE UE NAS
   */
  void SetNas (const Ptr<EpcUeNas> nas);

  /**
   * \brief Get the NAS.
   * \return the LTE UE NAS
   */
  Ptr<EpcUeNas> GetNas (void) const;


protected:
  // inherited from Object
  virtual void DoInitialize (void);


private:
  Ptr<EpcUeNas> m_nas; ///< the NAS

}; // end of class LteSlUeNetDevice

} // namespace ns3

#endif /* LTE_SL_UE_NET_DEVICE_H */
