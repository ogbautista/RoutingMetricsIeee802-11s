/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
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
 *
 * Adapted from AODV version authored by:
 * Nenad Jevtic <n.jevtic@sf.bg.ac.rs>, <nen.jevtc@gmail.com>
 *          Marija Malnar <m.malnar@sf.bg.ac.rs>
 * available at: https://github.com/neje/ns3-aodv-etx (Feb. 2019)
 *
 * Adapted by:
 * Oscar Bautista <obaut004@fiu.edu>,
 * as part of the implementation of ETX Metric in dot11s. March, 2019
 * from: the ETX for AODV implementation by N.J. Jevtic and M.Z. Malnar
 */

#ifndef HWMPNEIGHBORETX_H
#define HWMPNEIGHBORETX_H

#include <map>
#include "ns3/mac48-address.h"
#include "ie-lpp.h"

namespace ns3
{
namespace dot11s
{

class NeighborEtx
{
public:
  NeighborEtx ();
  struct Etx
  {
    uint16_t m_lppMyCnt10bMap;
    uint8_t m_lppReverse;
    Etx () : m_lppMyCnt10bMap (0), m_lppReverse (0) {}
  };

  // Returns current time slot (it is needed for sending LPP packet, used as LPP ID)
  uint8_t GetLppTimeStamp () {return m_lppTimeStamp; }

  // This function is used to prepare for new cycle of sending LPP packets.
  // It clears oldest LPP time slot value for each neighbor and moves to the new time slot.
  // These two, the oldest and this new current time slot value are not used for
  // calculation of ETX metrix (previous 10 time slots are used for ETX calculations)
  void GotoNextTimeStampAndClearOldest ();

  // Fills all ETX data from the neighbors map in the IE LPP
  void FillLppCntData (IeLpp &ielpp);

  // When receive LPP from neighbor node updates my lpp count for that neighbor, and
  // also reads all data from IE LPP and if it finds its MAC address then also
  // updates lpp reverse count
  bool UpdateNeighborEtx (Mac48Address addr, uint8_t lppTimeStamp, uint8_t lppReverse);

  // Look for neighbor and return its ETX, return etx ->oo (max of uint32_t) if there
  // is no neighbor in the map (this is unlikely since it will receive at least
  // one LPP packet from this neighbor and therefore neighbor will be in the map).
  uint32_t GetEtxForNeighbor (Mac48Address addr);

  // Print the etx metric for all links to neighbor nodes
  // param os The output stream
  void Print (std::ostream & os);

private:
  std::map<Mac48Address, Etx> m_neighborEtx;
  uint8_t m_lppTimeStamp; // has to be incremented every lpp time period; holds last 10 events (slots of 1 second by default)

  uint32_t CalculateBinaryShiftedEtx (struct Etx etxStruct);
  uint8_t Lpp10bMapToCnt (uint16_t lpp10bMap);

  void GotoNextLppTimeStamp ();
  static uint8_t CalculateNextLppTimeStamp (uint8_t currTimeStamp);
};

} // namespace dot11s
} // namespace ns3

#endif /* HWMPNEIGHBORETX_H */
