/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "rx-power-tag.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

namespace ns3 {

//NS_OBJECT_ENSURE_REGISTERED (RxPowerTag);

TypeId
RxPowerTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RxPowerTag")
    .SetParent<Tag> ()
    .SetGroupName("Wifi")
    .AddConstructor<RxPowerTag> ()
    .AddAttribute ("RxPower",
                   "Received packet power",
                   DoubleValue (0),
                   MakeDoubleAccessor (&RxPowerTag::GetRxPower), //this function is required - #include double.h
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
RxPowerTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
RxPowerTag::GetSerializedSize (void) const
{
  // here I define tag size in bytes
  // for double I check with the sizeof (double) operator
  return sizeof (double);
}

void
RxPowerTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_rxPower);
}

void
RxPowerTag::Deserialize (TagBuffer i)
{
  m_rxPower = i.ReadDouble ();
}

void
RxPowerTag::Print (std::ostream &os) const
{
  os << "RxPower=" << m_rxPower;
}

void
RxPowerTag::SetRxPower (double value)
{
  m_rxPower = value;
}

double
RxPowerTag::GetRxPower (void) const
{
  return m_rxPower;
}

} // namespace ns3
