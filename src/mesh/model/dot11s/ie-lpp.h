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
* Author: Oscar Bautista <obaut004@fiu.edu> (2019)
* As part of the implementation of ETX Metric in dot11s. March, 2019
*/

#ifndef WIFI_LPP_INFORMATION_ELEMENT_H
#define WIFI_LPP_INFORMATION_ELEMENT_H

#include <map>

#include "ns3/mac48-address.h"
#include "ns3/mesh-information-element-vector.h"

namespace ns3 {
namespace dot11s {

class IeLpp : public WifiInformationElement
{
public:
	///Constructor
	IeLpp();
	///Destructor
	~IeLpp();

	void SetLppId(uint8_t count)
	{
		m_lppId = count;
	}

	uint8_t GetLppId() const
	{
		return m_lppId;
	}

	void SetOriginAddress(Mac48Address addr)
	{
		m_originAddr = addr;
	}

	Mac48Address GetOriginAddress() const
	{
		return m_originAddr;
	}

	void SetOriginSeqno(uint32_t seqno)
	{
		m_originSeqno = seqno;
	}

	uint32_t GetOriginSeqno() const
	{
		return m_originSeqno;
	}

	uint8_t GetNumberNeighbors() const
	{
		return (uint8_t)m_neighborsLppCnt.size();
	}

	// Control neighbors list
	bool AddToNeighborsList(Mac48Address neighbor, uint8_t lppCnt);
	bool RemoveFromNeighborsList(std::pair<Mac48Address, uint8_t> & un);
	void ClearNeighborsList();

	// Inherited from WifiInformationElement
	virtual WifiInformationElementId ElementId() const;
	virtual void SerializeInformationField(Buffer::Iterator i) const;
	virtual uint8_t DeserializeInformationField(Buffer::Iterator start, uint8_t length);
	virtual uint8_t GetInformationFieldSize() const;
	virtual void Print(std::ostream& os) const;

private:
	uint8_t       m_lppId;         //< LPP ID which is set to the Lpp Time slot (runs 0 to 11 cyclically )
	Mac48Address  m_originAddr;    //< Originator MAC Address
	uint32_t      m_originSeqno;   //< Originator Sequence number

	// List of neighbors: MAC addresses and number of LLP received from each in the last checked 10 time slot period
	std::map<Mac48Address, uint8_t> m_neighborsLppCnt;

	/**
	* equality operator
	*
	* \param a lhs
	* \param b rhs
	* \returns true if equal
	*/
	friend bool operator== (const IeLpp & a, const IeLpp & b);
};
bool operator== (const IeLpp & a, const IeLpp & b);
std::ostream &operator << (std::ostream &os, const IeLpp &lpp);

} // namespace dot11s
} // namespace ns3
#endif
