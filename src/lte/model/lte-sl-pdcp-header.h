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

#ifndef LTE_SL_PDCP_HEADER_H
#define LTE_SL_PDCP_HEADER_H

#include "ns3/header.h"

#include <list>

namespace ns3 {

/**
 * \ingroup lte
 * \brief The packet header for the Packet Data Convergence Protocol (PDCP) packets
 *
 * This class has fields corresponding to those in an PDCP header used for Sidelink
 * Radio Bearers (SLRB) as well as
 * methods for serialization to and deserialization from a byte buffer.
 * It follows 3GPP TS 36.323 Packet Data Convergence Protocol (PDCP) specification
 * release 14.
 * Implementation notes:
 * The security field can be either PTK Identity (for one to many communication) or 
 * K_D-Sess ID (for one to one communication). 
 * Current version does not implement the MAC-I elements that would be used if the 
 * SLRB needed integrity protection.
 */
class LteSlPdcpHeader : public Header
{
public:

  /**
   * \brief Constructor
   *
   * Creates a null header
   */
  LteSlPdcpHeader ();
  ~LteSlPdcpHeader ();

  /**
   * \brief Set the SDU type (3-bit value)
   * 
   * \param sduType The SDU type
   */
  void SetSduType (uint8_t sduType); 
  
  /**
   * \brief Set the PGK index (5-bit value)
   * 
   * \param pgkIndex The PGK index
   */
  void SetPgkIndex (uint8_t pgkIndex);
  
  /**
   * \brief Set the security identity (16-bit value)
   * 
   * \param secIdentity The security identity
   */
  void SetSecurityIdentity (uint16_t secIdentity);
  
  /**
   * \brief Set the sequence number (16-bit value)
   * 
   * \param sequenceNumber The sequence number
   */
  void SetSequenceNumber (uint16_t sequenceNumber);

  /**
   * \brief Get the SDU type
   * 
   * \return The SDU type
   */
  uint8_t GetSduType () const;
  
  /**
   * \brief Get the PGK index
   * 
   * \return The PGK index
   */
  uint8_t GetPgkIndex () const;
  
  /**
   * \brief Get the security identity
   * 
   * \return The security identity
   */
  uint16_t GetSecurityIdentity () const;
  
  /**
   * \brief Get the sequence number
   * 
   * \return The sequence number
   */
  uint16_t GetSequenceNumber () const;

  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint8_t m_sduType; ///< The SDU type
  uint8_t m_pgkIndex; ///< The PGK index
  uint16_t m_secIdentity; ///< The security index
  uint16_t m_sequenceNumber; ///< The sequence number

};

} // namespace ns3

#endif // LTE_SL_PDCP_HEADER_H
