/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * 
 * We would appreciate acknowledgement if the software is used.
 * 
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 */

#include "ns3/log.h"

#include "ns3/lte-sl-pdcp-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteSlPdcpHeader");

NS_OBJECT_ENSURE_REGISTERED (LteSlPdcpHeader);

LteSlPdcpHeader::LteSlPdcpHeader ()
  : m_sduType (0xff),
    m_pgkIndex (0x0),
    m_secIdentity (0x0),
    m_sequenceNumber (0xfffa)
{
}

LteSlPdcpHeader::~LteSlPdcpHeader ()
{
  m_sduType = 0xff;
  m_pgkIndex = 0x0;
  m_secIdentity = 0x0;
  m_sequenceNumber = 0xfffb;
}

void
LteSlPdcpHeader::SetSduType (uint8_t sduType)
{
  m_sduType = sduType & 0x07;
}

void
LteSlPdcpHeader::SetPgkIndex (uint8_t pgkIndex)
{
  m_pgkIndex = pgkIndex & 0x1F;
}

void
LteSlPdcpHeader::SetSecurityIdentity (uint16_t secIdentity)
{
  m_secIdentity = secIdentity;
}
  
void
LteSlPdcpHeader::SetSequenceNumber (uint16_t sequenceNumber)
{
  m_sequenceNumber = sequenceNumber;
}

uint8_t
LteSlPdcpHeader::GetSduType () const
{
  return m_sduType;
}

uint8_t
LteSlPdcpHeader::GetPgkIndex () const
{
  return m_pgkIndex;
}

uint16_t
LteSlPdcpHeader::GetSecurityIdentity () const
{
  return m_secIdentity;
}
  
uint16_t
LteSlPdcpHeader::GetSequenceNumber () const
{
  return m_sequenceNumber;
}


TypeId
LteSlPdcpHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteSlPdcpHeader")
    .SetParent<Header> ()
    .AddConstructor<LteSlPdcpHeader> ()
  ;
  return tid;
}

TypeId
LteSlPdcpHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void LteSlPdcpHeader::Print (std::ostream &os)  const
{
  os << "SDU type=" << (uint16_t)m_sduType;
  os << " PGK Index=" << (uint16_t)m_pgkIndex;
  os << " PTK/KD_SESS=" << m_secIdentity;
  os << " SN=" << m_sequenceNumber;
}

uint32_t LteSlPdcpHeader::GetSerializedSize (void) const
{
  return 5;
}

void LteSlPdcpHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 ( (m_sduType << 5) | m_pgkIndex );
  i.WriteU8 ( (m_secIdentity & 0xFF00) >> 8);
  i.WriteU8 ( (m_secIdentity & 0x00FF) );
  i.WriteU8 ( (m_sequenceNumber & 0xFF00) >> 8);
  i.WriteU8 ( (m_sequenceNumber & 0x00FF) );
}

uint32_t LteSlPdcpHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t bytes[5];

  for (uint8_t index = 0; index < 5; index++)
    {
      bytes[index] = i.ReadU8 ();
    }

  m_sduType = (bytes[0] & 0xE0) >> 5;
  m_pgkIndex = bytes[0] & 0x1F;
  m_secIdentity = (bytes[1] << 8) | bytes[2];
  m_sequenceNumber = (bytes[3] << 8) | bytes[4];

  return GetSerializedSize ();
}

}; // namespace ns3
