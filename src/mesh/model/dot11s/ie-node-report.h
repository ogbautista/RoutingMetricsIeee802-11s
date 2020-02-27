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

#ifndef WIFI_NODE_REPORT_H
#define WIFI_NODE_REPORT_H

#include <map>

#include "ns3/mac48-address.h"
#include "ns3/mesh-information-element-vector.h"
#include "ns3/vector.h"

namespace ns3 {
namespace dot11s {

class IeNodeReport : public WifiInformationElement
{
public:
	///Constructor
	IeNodeReport();
	///Destructor
	~IeNodeReport();
 /**
  * Stores the Node's Location
	* \param location, a Vector3D
	*/
  void SetLocation(Vector location);
 /**
  * Gets the location
  * \returns the location
  */
 	Vector GetLocation() const;
 /**
	* Stores the Node's Velocity
	* \param velocity, a Vector3D
	*/
	void SetVelocity(Vector velocity);
 /**
	* Gets the velocity
	* \returns the velocity
	*/
	Vector GetVelocity() const;
 /**
 	* Set the Node Id
 	* \param  nodeId, the node Id
 	*/
	void SetNodeId (uint8_t nodeId);
 /**
	* Get the Node Id
	* \returns the Node Id
	*/
	uint8_t GetNodeId ();

	// Inherited from WifiInformationElement
	virtual WifiInformationElementId ElementId() const;
	virtual void SerializeInformationField(Buffer::Iterator i) const;
	virtual uint8_t DeserializeInformationField(Buffer::Iterator start, uint8_t length);
	virtual uint8_t GetInformationFieldSize() const;
	virtual void Print(std::ostream& os) const;

private:
	uint8_t    m_nodeId;
  Vector     m_nodeLocation;  //< Location of the node in tenths of meters, all components are assumed to be positive, max component value is 6.55 Km
  Vector     m_nodeVelocity;  //< Velocity of the node in tenths of meter/seg, each dimension could be positive or negative

	/**
	* equality operator
	*
	* \param a lhs
	* \param b rhs
	* \returns true if equal
	*/
	friend bool operator== (const IeNodeReport & a, const IeNodeReport & b);
};
bool operator== (const IeNodeReport & a, const IeNodeReport & b);
std::ostream &operator << (std::ostream &os, const IeNodeReport &nodeReport);

} // namespace dot11s
} // namespace ns3
#endif
