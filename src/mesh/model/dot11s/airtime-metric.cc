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
 * Authors: Kirill Andreev <andreev@iitp.ru>
 *
 * Modified by: Oscar Bautista <obaut004@fiu.edu>
 * to implement SrFTime and CRP metrics. (2019)
 */

#include "airtime-metric.h"
#include "ns3/wifi-phy.h"

namespace ns3 {
namespace dot11s {
NS_OBJECT_ENSURE_REGISTERED (AirtimeLinkMetricCalculator);

TypeId
AirtimeLinkMetricCalculator::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::dot11s::AirtimeLinkMetricCalculator")
    .SetParent<Object> ()
    .SetGroupName ("Mesh")
    .AddConstructor<AirtimeLinkMetricCalculator> ()
    .AddAttribute ( "TestLength",
                    "Number of bytes in test frame (a constant 1024 in the standard)",
                    UintegerValue (1024),
                    MakeUintegerAccessor (
                      &AirtimeLinkMetricCalculator::SetTestLength),
                    MakeUintegerChecker<uint16_t> (1)
                    )
    .AddAttribute ( "Dot11MetricTid",
                    "TID used to calculate metric (data rate)",
                    UintegerValue (0),
                    MakeUintegerAccessor (
                      &AirtimeLinkMetricCalculator::SetHeaderTid),
                    MakeUintegerChecker<uint8_t> (0)
                    )
    .AddAttribute ( "FerFromBeacon",
                    "Enable using Beacons to calculate avg frame error",
                    BooleanValue (false),
                    MakeBooleanAccessor (
                      &AirtimeLinkMetricCalculator::m_useBeacon),
                    MakeBooleanChecker ()
                    )
    .AddAttribute ( "SquareRootTime",
                    "Airtime Square Root is used for metric calculation",
                    BooleanValue (false),
                    MakeBooleanAccessor (
                      &AirtimeLinkMetricCalculator::m_useRootSquare),
                    MakeBooleanChecker ()
                    )
    .AddAttribute ( "MetricRxPowerCoef",
                    "Coefficient to account for packet's Rx Power into link metric",
                    UintegerValue (0),
                    MakeUintegerAccessor (
                      &AirtimeLinkMetricCalculator::m_metricRxPowerCoef),
                    MakeUintegerChecker<uint16_t> (0)
                    )
  ;
  return tid;
}
AirtimeLinkMetricCalculator::AirtimeLinkMetricCalculator ()
{
}
void
AirtimeLinkMetricCalculator::SetHeaderTid (uint8_t tid)
{
  m_testHeader.SetType (WIFI_MAC_DATA);
  m_testHeader.SetDsFrom ();
  m_testHeader.SetDsTo ();
  m_testHeader.SetQosTid (tid);
}
void
AirtimeLinkMetricCalculator::SetTestLength (uint16_t testLength)
{
  m_testFrame = Create<Packet> (testLength + 6 /*Mesh header*/ + 36 /*802.11 header*/);
}
uint32_t
AirtimeLinkMetricCalculator::CalculateMetric (Mac48Address peerAddress, Ptr<MeshWifiInterfaceMac> mac)
{
  /* Airtime link metric is defined in Section 13.9 of 802.11-2012 as:
   *
   * airtime = (O + Bt/r) /  (1 - frame error rate), where
   * o  -- the PHY dependent channel access which includes frame headers, training sequences,
   *       access protocol frames, etc.
   * bt -- the test packet length in bits (8192 by default),
   * r  -- the current bitrate of the packet,
   *
   * Final result is expressed in units of 0.01 Time Unit = 10.24 us (as required by 802.11s draft)
   */
  double failAvg;
  NS_ASSERT (!peerAddress.IsGroup ());
  //obtain current rate:
  WifiMode mode = mac->GetWifiRemoteStationManager ()->GetDataTxVector (peerAddress, &m_testHeader, m_testFrame).GetMode();
  //obtain frame error rate:
  if (m_useBeacon)
  {
    failAvg = mac->GetFailAvg (peerAddress);
  }
  else
  {
    failAvg = mac->GetWifiRemoteStationManager ()->GetInfo (peerAddress).GetFrameErrorRate ();
  }

  if (failAvg == 1)
    {
      // Return max metric value when frame error rate equals to 1
      return (uint32_t) 0xffffffff;
    }
  NS_ASSERT (failAvg < 1.0);
  WifiTxVector txVector;
  txVector.SetMode (mode);
  txVector.SetPreambleType (WIFI_PREAMBLE_LONG);
  //calculate metric
  uint32_t metric;

  double powerBudget = mac->GetPeerRxPower (peerAddress) - mac->GetWifiPhy ()->GetEdThreshold ();
  double powerCompensation = 0;
  double powerFactor = 1;
  if (powerBudget < 3)
  {
    powerCompensation = m_metricRxPowerCoef*(std::pow (10 , (3.0 - powerBudget)/10) - 1);
  }
  if (m_useRootSquare)
  {
    metric = (uint32_t) ( ( ((double)( /*Overhead + payload*/
                                   mac->GetPifs () + mac->GetSlot () + mac->GetEifsNoDifs ()).GetMicroSeconds() + //DIFS + SIFS + AckTxTime = PIFS + SLOT + EifsNoDifs
                                   20*std::sqrt ((double)(mac->GetWifiPhy ()->CalculateTxDuration (m_testFrame->GetSize (), txVector, mac->GetWifiPhy ()->GetFrequency ())).GetMicroSeconds ())
                            ) / 10.24 + (powerCompensation)
                          )* powerFactor/(1.0 - failAvg)
                        );
  }
  else
  {
    metric = (uint32_t) ( ( ((double)( /*Overhead + payload*/
                                    mac->GetPifs () + mac->GetSlot () + mac->GetEifsNoDifs () + //DIFS + SIFS + AckTxTime = PIFS + SLOT + EifsNoDifs
                                    mac->GetWifiPhy ()->CalculateTxDuration (m_testFrame->GetSize (), txVector, mac->GetWifiPhy ()->GetFrequency ())).GetMicroSeconds ()
                            ) / 10.24 + (powerCompensation)
                          )* powerFactor/(1.0 - failAvg)
                        );
  }
  return metric;
}
} // namespace dot11s
} // namespace ns3
