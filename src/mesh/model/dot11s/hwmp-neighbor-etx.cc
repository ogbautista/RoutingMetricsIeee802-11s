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

#include "hwmp-neighbor-etx.h"
#include "ns3/log.h"
#include <math.h>
#include <stdint.h>

namespace ns3
{
#define ETX_MAX 200000  //The maximum value for etx metric when there is no information to calculate it
NS_LOG_COMPONENT_DEFINE ("HwmpNeighborEtx");

namespace dot11s
{

NeighborEtx::NeighborEtx () : m_lppTimeStamp (0) {}

// There are 12 different ETX timeslots: [ 0, 1, 2, ..., 11, 0, 1, ...] each with value 0 or 1
// but 2 values are not included in etx (lpp count): current and oldest

uint8_t
NeighborEtx::CalculateNextLppTimeStamp (uint8_t currTimeStamp)
{
  uint8_t nextTimeStamp = currTimeStamp + 1;
  if (nextTimeStamp > 11)
    {
      nextTimeStamp = 0;
    }
  return nextTimeStamp;
}

void
NeighborEtx::GotoNextLppTimeStamp ()
{
  m_lppTimeStamp = CalculateNextLppTimeStamp (m_lppTimeStamp);
}

// There are 12 different ETX timeslots: [ 0, 1, 2, ..., 11, 0, 1, ...] each with value 0 or 1
// but 2 values are not included in etx (lpp count):
// 1. curent time slot because of jitter some nodes have transmited before this node
//    and some nodes will transmit after current, so packet count would not be fair (nodes that transmitted before
//    would be having one more lpp count)
// 2. oldest time slot value will be deleted so it sholud not be included in calculation of lpp count
uint8_t
NeighborEtx::Lpp10bMapToCnt (uint16_t lpp10bMap)
{
  uint8_t lpp = 0;
  for (int j=0; j<12; ++j)
    {
      if ((j!=m_lppTimeStamp) && (j!=CalculateNextLppTimeStamp(m_lppTimeStamp)))
        {
          lpp += (lpp10bMap & ((uint16_t)0x0001 << j)) ? 1 : 0;
        }
    }
  return lpp;
}

void
NeighborEtx::GotoNextTimeStampAndClearOldest ()
{
  GotoNextLppTimeStamp (); // go to  next times slot which becomes current time slot
  // Clear oldest time slot lpp count values, this is next time slot related to current

  // Oldest Time Slot is next bit (cyclically) after current Time Slot
  uint8_t OldestLppTimeSlot = CalculateNextLppTimeStamp (m_lppTimeStamp);

  for (std::map<Mac48Address, Etx>::iterator i = m_neighborEtx.begin (); i != m_neighborEtx.end (); ++i)
    {
      Etx etx = i->second;
      uint16_t lppMyCnt10bMap = etx.m_lppMyCnt10bMap;
      // Delete oldest times slot lpp count (this is next time slot related to current)
      // Only lower 12 bits are used
      lppMyCnt10bMap &= (uint16_t)(~((uint16_t)0x0001 << OldestLppTimeSlot) & (uint16_t)0x0FFF);
      etx.m_lppMyCnt10bMap = lppMyCnt10bMap;
      i->second = etx;
    }
}

void
NeighborEtx::FillLppCntData (IeLpp &ielpp)
{
  for (std::map<Mac48Address, Etx>::iterator i = m_neighborEtx.begin (); i != m_neighborEtx.end (); ++i)
        {
          uint8_t lpp = Lpp10bMapToCnt (i->second.m_lppMyCnt10bMap);
          if (lpp > 0)
            {
              ielpp.AddToNeighborsList (i->first, lpp);
              //NS_LOG_UNCOND ("           MAC=" <", lpp=" << (uint16_t)lpp << ", rev=" << (uint16_t)(i->second.m_lppReverse) << ", ETX-bin-shift=" << CalculateBinaryShiftedEtx (i->second));
            }
        }
}

bool
NeighborEtx::UpdateNeighborEtx (Mac48Address addr, uint8_t lppTimeStamp, uint8_t lppReverse)
{
  std::map<Mac48Address, Etx>::iterator i = m_neighborEtx.find (addr);
  if (i == m_neighborEtx.end ())
    {
      // No address, insert new entry
      Etx etx;
      etx.m_lppReverse = lppReverse;
      etx.m_lppMyCnt10bMap = (uint16_t)0x0001 << lppTimeStamp;
      std::pair<std::map<Mac48Address, Etx>::iterator, bool> result = m_neighborEtx.insert (std::make_pair (addr, etx));
      return result.second;
    }
  else
    {
      // Address found, update existing entry
      i->second.m_lppReverse = lppReverse;
      (i->second.m_lppMyCnt10bMap) |= ((uint16_t)0x0001 << lppTimeStamp);
      return true;
    }
}

uint32_t
NeighborEtx::CalculateBinaryShiftedEtx (Etx etxStruct)
{
  //uint32_t etx = UINT32_MAX;  //This is causing inexplicable and negative behavior in routing table
  uint32_t etx = ETX_MAX;
  if ((Lpp10bMapToCnt (etxStruct.m_lppMyCnt10bMap)!=0) && (etxStruct.m_lppReverse!=0))
    {
      //Multiplier to have resolution of 3 decimal digits shifted to integer position
      etx = (uint32_t) (round (100000.0 / (Lpp10bMapToCnt (etxStruct.m_lppMyCnt10bMap) * etxStruct.m_lppReverse)));
    }
  //NS_LOG_UNCOND ("ETX binary: " << etx);
  return etx;
}

uint32_t
NeighborEtx::GetEtxForNeighbor (Mac48Address addr)
{
  uint32_t etx;
  std::map<Mac48Address, Etx>::iterator i = m_neighborEtx.find (addr);
  if (i == m_neighborEtx.end ())
    {
      // No address, ETX -> oo (= UINT32_MAX)
      //etx = UINT32_MAX; //This is causing inexplicable and negative behavior in routing table
      etx = ETX_MAX;
      return etx;
    }
  else
    {
      // Address found, calculate and return current ETX value
      return CalculateBinaryShiftedEtx (i->second);
    }
}

void
NeighborEtx::Print (std::ostream & os)
{
  uint32_t linkMetric;
  Etx etx;
  os << "<EtxMetric currentLppTimeSlot=\"" << (uint32_t) m_lppTimeStamp <<"\">" << std::endl;
  for (std::map<Mac48Address, Etx>::const_iterator i = m_neighborEtx.begin (); i != m_neighborEtx.end (); ++i)
    {
      etx = i->second;
      linkMetric = CalculateBinaryShiftedEtx (etx);
      os << "<PeerLink peerAddress=\"" << i->first << "\" metric=\"" << linkMetric << "\" mapCountForward=\"" << (uint32_t) etx.m_lppMyCnt10bMap << "\" lppCountReverse=\"" << (uint32_t) etx.m_lppReverse << "\"/>" << std::endl;
    }
  os << "</EtxMetric>" << std::endl;
}

} // namespace dot11s
} // namespace ns3
