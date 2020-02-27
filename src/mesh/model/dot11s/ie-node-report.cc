/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* Copyright (c) 2008,2009 IITP RAS
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
* Based on original mesh dot11s model by:
* Kirill Andreev <andreev@iitp.ru>
*
* Author: Oscar Bautista <obaut004@fiu.edu>, <ogbautista@gmail.com>
* For definition of a new metric for mobile networks. July, 2019
*/

#include "ie-node-report.h"
#include "ns3/address-utils.h"
#include "ns3/assert.h"
#include "ns3/packet.h"

namespace ns3 {
namespace dot11s {
/*******************************
* IeNodeReport
*******************************/
IeNodeReport::~IeNodeReport () {}

IeNodeReport::IeNodeReport () {}

WifiInformationElementId
IeNodeReport::ElementId() const
{
	return IE_NODE_REPORT;
}

void
IeNodeReport::SetLocation (Vector location)
{
 m_nodeLocation = location;
}

Vector
IeNodeReport::GetLocation() const
{
 return m_nodeLocation;
}

void
IeNodeReport::SetVelocity (Vector velocity)
{
 m_nodeVelocity = velocity;
}

Vector
IeNodeReport::GetVelocity() const
{
 return m_nodeVelocity;
}

void
IeNodeReport::SetNodeId (uint8_t nodeId)
{
  m_nodeId = nodeId;
}

uint8_t
IeNodeReport::GetNodeId()
{
  return m_nodeId;
}

void
IeNodeReport::SerializeInformationField(Buffer::Iterator i) const
{
	i.WriteU8 (m_nodeId);
	i.WriteHtolsbU16 ((uint16_t) (m_nodeLocation.x*10));
	i.WriteHtolsbU16 ((uint16_t) (m_nodeLocation.y*10));
	i.WriteHtolsbU16 ((uint16_t) (m_nodeLocation.z*10));
	i.WriteHtolsbU16 ((int16_t) (m_nodeVelocity.x*10));
	i.WriteHtolsbU16 ((int16_t) (m_nodeVelocity.y*10));
	i.WriteHtolsbU16 ((int16_t) (m_nodeVelocity.z*10));
}

uint8_t
IeNodeReport::DeserializeInformationField(Buffer::Iterator start, uint8_t length)
{
	Buffer::Iterator i = start;
	m_nodeId = i.ReadU8 ();
  m_nodeLocation.x = (float) i.ReadLsbtohU16()/10;
  m_nodeLocation.y = (float) i.ReadLsbtohU16()/10;
  m_nodeLocation.z = (float) i.ReadLsbtohU16()/10;
	m_nodeVelocity.x = (float) (int16_t) i.ReadLsbtohU16()/10;
	m_nodeVelocity.y = (float) (int16_t) i.ReadLsbtohU16()/10;
	m_nodeVelocity.z = (float) (int16_t) i.ReadLsbtohU16()/10;
	uint8_t dist = i.GetDistanceFrom(start);
	NS_ASSERT(dist == GetInformationFieldSize());
	return dist;
}

uint8_t
IeNodeReport::GetInformationFieldSize() const
{
	uint8_t retval = 1  //Node Id
	  + 6   //3D Location
		+ 6;  //3D Velocity
	return (retval);
}

void
IeNodeReport::Print(std::ostream &os) const
{
  os << "NODE_REPORT=(Node ID=" << m_nodeId
     << ", Node Location=" << m_nodeLocation
     << ", Node Velocity=" << m_nodeVelocity << ")";
}

bool
operator== (const IeNodeReport & a, const IeNodeReport & b)
{
	if (a.m_nodeId != b.m_nodeId || a.m_nodeLocation.x != b.m_nodeLocation.x || a.m_nodeLocation.y != b.m_nodeLocation.y
		|| a.m_nodeLocation.z != b.m_nodeLocation.z || a.m_nodeVelocity.x != b.m_nodeVelocity.x || a.m_nodeVelocity.y != b.m_nodeVelocity.y
		|| a.m_nodeVelocity.z != b.m_nodeVelocity.z)
	{
		return false;
	}
	return true;
}

std::ostream &
operator << (std::ostream &os, const IeNodeReport &a)
{
	a.Print(os);
	return os;
}

} // namespace dot11s
} // namespace ns3
