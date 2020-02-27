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

#include "ie-lpp.h"
#include "ns3/address-utils.h"
#include "ns3/assert.h"
#include "ns3/packet.h"

namespace ns3 {
namespace dot11s {
/*******************************
* IeLpp
*******************************/
IeLpp::~IeLpp() {}

IeLpp::IeLpp() {}

WifiInformationElementId
IeLpp::ElementId() const
{
	return IE_LPP;
}

void
IeLpp::SerializeInformationField(Buffer::Iterator i) const
{
	i.WriteU8(m_lppId);
	WriteTo(i, m_originAddr);
	i.WriteHtolsbU32(m_originSeqno);
	i.WriteU8(GetNumberNeighbors());
	std::map<Mac48Address, uint8_t>::const_iterator j;
	for (j = m_neighborsLppCnt.begin(); j != m_neighborsLppCnt.end(); ++j)
	{
		WriteTo(i, (*j).first);
		i.WriteU8((*j).second);
	}
}

uint8_t
IeLpp::DeserializeInformationField(Buffer::Iterator start, uint8_t length)
{
	Buffer::Iterator i = start;
	m_lppId = i.ReadU8();
	ReadFrom(i, m_originAddr);
	m_originSeqno = i.ReadLsbtohU32();
	uint8_t numberNeighbors = i.ReadU8();
	m_neighborsLppCnt.clear();
	Mac48Address neighborAddr;
	uint8_t lppCnt;
	for (uint8_t k = 0; k < numberNeighbors; ++k)
	{
		ReadFrom(i, neighborAddr);
		lppCnt = i.ReadU8();
		m_neighborsLppCnt.insert(std::make_pair(neighborAddr, lppCnt));
	}

	uint8_t dist = i.GetDistanceFrom(start);
	NS_ASSERT(dist == GetInformationFieldSize());
	return dist;
}

uint8_t
IeLpp::GetInformationFieldSize() const
{
	uint8_t retval = 1 //LPP ID
		+ 6   //Source address (originator)
		+ 4   //Originator seqno
		+ 1;  //Number of Neighbors

	return (retval + 7 * GetNumberNeighbors());
}

void
IeLpp::Print(std::ostream &os) const
{
	os << "LPP=(Lpp ID: " << m_lppId << ", Originator MAC address: " << m_originAddr;
	os << "Originator Sequence number: " << m_originSeqno;
	os << "Number of neighbors: " << (*this).GetNumberNeighbors();
	os << "Neighbors (Mac address, received LPP count): ";
	std::map<Mac48Address, uint8_t>::const_iterator j;
	for (j = m_neighborsLppCnt.begin(); j != m_neighborsLppCnt.end(); ++j)
	{
		os << (*j).first << ", " << (*j).second;
	}
	os << ")";
}

bool
IeLpp::AddToNeighborsList(Mac48Address neighbor, uint8_t lppCnt)
{
	if (m_neighborsLppCnt.find(neighbor) != m_neighborsLppCnt.end())
	{
		return true;
	}

	NS_ASSERT(GetNumberNeighbors() < 255); // can't support more than 2^8 - 1 neighbors
	m_neighborsLppCnt.insert(std::make_pair(neighbor, lppCnt));
	return true;
}

bool
IeLpp::RemoveFromNeighborsList(std::pair<Mac48Address, uint8_t> & un)
{
	if (m_neighborsLppCnt.empty())
	{
		return false;
	}

	std::map<Mac48Address, uint8_t>::iterator i = m_neighborsLppCnt.begin();
	un = *i;
	m_neighborsLppCnt.erase(i);
	return true;
}

void
IeLpp::ClearNeighborsList()
{
	m_neighborsLppCnt.clear();
}

bool
operator== (const IeLpp & a, const IeLpp & b)
{
	if (a.m_lppId != b.m_lppId || a.m_originAddr != b.m_originAddr || a.m_originSeqno != b.m_originSeqno
		|| a.GetNumberNeighbors() != b.GetNumberNeighbors())
	{
		return false;
	}
	std::map<Mac48Address, uint8_t>::const_iterator j = a.m_neighborsLppCnt.begin();
	std::map<Mac48Address, uint8_t>::const_iterator k = b.m_neighborsLppCnt.begin();
	for (uint8_t i = 0; i < a.GetNumberNeighbors(); ++i)
	{
		if ((j->first != k->first) || (j->second != k->second))
		{
			return false;
		}
		j++;
		k++;
	}
	return true;
}

std::ostream &
operator << (std::ostream &os, const IeLpp &a)
{
	a.Print(os);
	return os;
}

} // namespace dot11s
} // namespace ns3
