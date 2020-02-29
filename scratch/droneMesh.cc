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
 * Author: Oscar Bautista <obaut004@fiu.edu>, (2020)
 *
 * By default this script creates a mesh network with ns2mobility
 * IEEE802.11s stack installed at each node with peering management
 * and HWMP protocol.
 *
 * If mobility is disabled the nodes' location is set according to he
 * #include n_eq_* files and the number of nodes especified (default is 60)
 * Additionally, if grid is enabled, this script creates
 * m_xSize * m_ySize square grid topology as defined by Kirill Andreev:
 *
 * The side of the square cell is defined by m_step parameter.
 *
 *  m_xSize * step
 *  |<--------->|
 *   step
 *  |<--->|
 *  * --- * --- * <---Ping sink  _
 *  | \   |   / |                ^
 *  |   \ | /   |                |
 *  * --- * --- * m_ySize * step |
 *  |   / | \   |                |
 *  | /   |   \ |                |
 *  * --- * --- *                _
 *  ^ Ping source
 *
 *  See also MeshTest::Configure to read more about configurable
 *  parameters.
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mesh-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/propagation-module.h"
//#include "ns3/data-rate.h"

//For non-grid topology
#include "n_eq_coord.h"
#include "n_eq_20_3d.h"
#include "n_eq_40_3d.h"
#include "n_eq_60_3d.h"

//For Animation with NetAnim
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestMeshScript");

// Declared globally to be able to be called from static method RouteChangeSink
Mac48Address g_sinkMac;
std::string g_rChangeFile = "rChanges.csv";
std::string g_courseChangeFile = "courseChanges.csv";
/**
 * \ingroup mesh
 * \brief MeshTest class
 */
class MeshTest
{
public:
  /// Init test
  MeshTest ();
  /**
   * Configure test from command line arguments
   *
   * \param argc command line argument count
   * \param argv command line arguments
   */
  void Configure (int argc, char ** argv);
  /**
   * Run test
   * \returns the test status
   */
  int Run ();
private:
  int       m_xSize; ///< X size
  int       m_ySize; ///< Y size
  int       m_nNodes; ///< if topology is different to a grid, this selects the size of the distribution
  double    m_step; ///< step
  double    m_randomStart; ///< random start
  double    m_startTime;  ///< Start of Data Traffic
  double    m_totalTime; ///< total time
  uint16_t  m_packetSize; ///< packet size
  uint64_t  m_iDataRate; ///< data rate
  int       m_sink;    ///< Sink of data Stream
  uint32_t  m_nIfaces; ///< number interfaces
  double    m_txpower; ///< Tw Power in dbm
  bool      m_chan; ///< channel
  bool      m_pcap; ///< PCAP
  bool      m_ascii; ///< ASCII
  ///To implement a topology different than the standard grid:
  bool      m_gridtopology;
  int       m_topoId;
  /// Hwmp related parameters
  bool      m_etxMetric;
  bool      m_enableLpp;
  bool      m_airTimeBMetric;
  uint16_t  m_beaconWinSize;
  bool      m_hopCntMetric;
  bool      m_srAirtime;
  uint16_t  m_metricRxPowerCoef;
  bool      m_doFlag;
  bool      m_rfFlag;
  bool      m_ns2Mobil;

  vector< coordinates > nodeCoords;

  std::string m_stack;         ///< stack
  std::string m_metric;        ///< The routing metric [airtime, airtime-b, etx]
  std::string m_wifiStandard;  ///< The 802.11(?) Wifi Standard reference name   [ 80211a, 80211g, 80211n2.4, 80211n5 ]
  std::string m_remStaManager; ///< The Remote Station Manager reference name  [ arf, minstrel, minstrelht, constantrate ]
  std::string m_linkRate;      ///< The link rate to use with ConstantRateWifiManager
  std::string m_propLoss;      ///< The Propagation Loss Model reference name  [ logdistance, kun2600, itur1411, friis ]
  std::string m_root;          ///< root
  std::string m_UdpTcpMode;    ///< to hold Command Line Argument Value
  std::string m_asciiFile;     ///< Suffix for diagnostics files
  std::string m_scenario;      ///< ns2 trace file for node location and mobility

  /// List of network nodes
  NodeContainer nodes;
  /// List of all mesh point devices
  NetDeviceContainer meshDevices;
  /// Addresses of interfaces:
  Ipv4InterfaceContainer interfaces;
  /// MeshHelper. Report is not static methods
  MeshHelper mesh;
private:
  /// Create nodes and setup their mobility
  void CreateNodes ();
  /// Install internet m_stack on nodes
  void InstallInternetStack ();
  /// Install applications
  void InstallApplication ();
  /// Print mesh devices diagnostics
  void Report ();
  /// Functions called to process tracing information
  static void RouteChangeSink(std::string context, ns3::dot11s::RouteChange rChange);
  static void CourseChange (std::string context, Ptr<const MobilityModel> model);
  void ExportMobility (std::string stage);
};
MeshTest::MeshTest () :
  m_xSize (4),
  m_ySize (4),
  m_nNodes (60),
  m_step (100.0),
  m_randomStart (0.25),
  m_startTime (5.0),
  m_totalTime (125.0),
  m_packetSize (536),
  m_iDataRate (5),
  m_nIfaces (1),
  m_txpower (-3.0),
  m_chan (true),
  m_pcap (false),
  m_ascii (false),
  m_gridtopology (false),
  m_topoId (0),
  m_etxMetric (false),
  m_enableLpp (false),
  m_airTimeBMetric (false),
  m_beaconWinSize (30),
  m_hopCntMetric (false),
  m_srAirtime (false),
  m_metricRxPowerCoef (0),
  m_doFlag (true),
  m_rfFlag (true),
  m_ns2Mobil (true),
  m_stack ("ns3::Dot11sStack"),
  m_metric ("airtime"),
  m_wifiStandard ("80211n2.4"),
  m_remStaManager ("minstrelht"),
  m_linkRate ("ErpOfdmRate6Mbps"),
  m_propLoss ("friis"),
  m_root ("00:00:00:00:00:01"), // Default: "ff:ff:ff:ff:ff:ff"
  m_UdpTcpMode ("udp"),
  m_asciiFile  ("mesh.tr"),
  m_scenario ("gm3d-60-1.ns_movements")
{
  // The node that receives the data from all the other nodes
  m_sink = 0;// m_xSize * m_ySize - 1; //15
}
void
MeshTest::Configure (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.AddValue ("x-size", "Number of nodes in a row grid", m_xSize);
  cmd.AddValue ("y-size", "Number of rows in a grid", m_ySize);
  cmd.AddValue ("nodes", "Number of nodes in a custom distribution", m_nNodes);
  cmd.AddValue ("step",   "Size of edge in our grid (meters)", m_step);
  // Avoid starting all mesh nodes at the same time (beacons may collide)
  cmd.AddValue ("start", "Maximum random start delay for beacon jitter (sec)", m_randomStart);
  cmd.AddValue ("time",  "Simulation time (sec)", m_totalTime);
  cmd.AddValue ("packet-start", "Start of Data Traffic (sec)", m_startTime);
  cmd.AddValue ("packet-size", "Size of packets (bytes)", m_packetSize);
  cmd.AddValue ("data-rate",   "Data Rate (kbps)", m_iDataRate);
  cmd.AddValue ("interfaces", "Number of radio interfaces used by each mesh point", m_nIfaces);
  cmd.AddValue ("tx-power", "Transmission Power in dBm", m_txpower);
  cmd.AddValue ("channels",   "Use different frequency channels for different interfaces", m_chan);
  cmd.AddValue ("pcap",  "Enable PCAP traces on interfaces", m_pcap);
  cmd.AddValue ("ascii", "Enable Ascii traces on interfaces", m_ascii);
  cmd.AddValue ("grid", "Choice whether grid or random topology", m_gridtopology);
  cmd.AddValue ("topology", "Number of topology from predefined list (0-29)", m_topoId);
  cmd.AddValue ("ns2mobility", "Nodes move per ns2 mobility trace file", m_ns2Mobil);
  cmd.AddValue ("stack", "Type of protocol stack. ns3::Dot11sStack by default", m_stack);
  cmd.AddValue ("metric", "Selection of routing metric by name, it affects the boolean metric selecting attributes", m_metric);
  cmd.AddValue ("root", "Mac address of root mesh point in HWMP", m_root);
  cmd.AddValue ("sink", "Sink node ID", m_sink);
  cmd.AddValue ("protocol", "UDP or TCP mode", m_UdpTcpMode);
  cmd.AddValue ("etx", "Enable use of ETX Metric overriding AirTime Metric", m_etxMetric);
  cmd.AddValue ("lpp", "Enable the Transmission of LPP needed to calculate the ETX Metric", m_enableLpp);
  cmd.AddValue ("airtime-b", "Variation of Airtime link Metric that calculates avg pkg fail from beacons", m_airTimeBMetric);
  cmd.AddValue ("beacon-window", "Number of beacons to be considered for failAvg for airtime-b metric (Max.30)", m_beaconWinSize);
  cmd.AddValue ("hop-count", "Enable use of Hop Count Metric overriding AirTime Metric", m_hopCntMetric);
  cmd.AddValue ("sr-airtime", "Airtime Square Root is used for metric calculation", m_srAirtime);
  cmd.AddValue ("metric-rxpower-coef", "Coefficient to account for packet's Rx Power into link metric", m_metricRxPowerCoef);
  cmd.AddValue ("do-flag", "HWMP Destination only flag", m_doFlag);
  cmd.AddValue ("rf-flag", "HWMP reply and forward flag", m_rfFlag );
  cmd.AddValue ("wifi-standard", "The 802.11(?) to be used by wifi stations (string)", m_wifiStandard );
  cmd.AddValue ("remote-station-manager", "The remote station manager (string)", m_remStaManager);
  cmd.AddValue ("link-rate", "The link speed used with Constant Rate Wifi Manager (string)", m_linkRate);
  cmd.AddValue ("propagation-loss-model", "The Propagation Loss Model for the medium (string)", m_propLoss);
  cmd.AddValue ("ascii-file", "The Ascii report filename", m_asciiFile);
  cmd.AddValue ("scenario", "Ns2 trace file with location and mobility scenario", m_scenario);

  cmd.Parse (argc, argv);
  NS_ASSERT_MSG (m_beaconWinSize < 31, "Maximum Size of Beacons Window is 30.");
  // g_sinkMac = Mac48Address(m_root.c_str());
  if (m_root != "ff:ff:ff:ff:ff:ff")
    {
      g_sinkMac = Mac48Address (m_root.c_str());
    }
  else
    {
      // if no root is set, then the first node is considered the sink of data
      g_sinkMac = Mac48Address ("00:00:00:00:00:01");
    }
  // default metric is airtime, even if a random string is indicated, the routing protocol will use airtime
  if (m_metric == "airtime-b") m_airTimeBMetric = true;
  if (m_metric == "etx") m_etxMetric = true;
  if (m_metric == "hop-count") m_hopCntMetric = true;
  if (m_metric == "srftime") { m_srAirtime = true; m_airTimeBMetric = true; }

  if (m_gridtopology)
  {
    NS_LOG_DEBUG ("Grid: " << m_xSize << "*" << m_ySize);
  }
  else
  {
    NS_LOG_DEBUG ("Custom topology: " << m_nNodes << " nodes" );
  }
  NS_LOG_DEBUG ("Simulation time: " << m_totalTime << " s");
  if (m_ascii)
    {
      PacketMetadata::Enable ();
    }
}
void
MeshTest::CreateNodes ()
{
  /*
   * Create m_ySize*m_xSize stations to form a grid topology
   */
  if (m_gridtopology) m_nNodes = m_xSize * m_ySize;

  nodes.Create (m_nNodes);

  // Configure YansWifiChannel

 /* Default YansWifiPhyHelper:
  * SetErrorRateModel to "ns3::NistErrorRateModel"
  */
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
 /*
  * Configuration of Physical layer parameters
  */
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-87.0) );  //Default: -96.0
  //wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-96.0) );       //Default: -99.0
  //wifiPhy.Set ("TxGain", DoubleValue (1.0) );                    //Default: 0
  //wifiPhy.Set ("RxGain", DoubleValue (1.0) );                    //Default: 0
  //wifiPhy.Set ("TxPowerLevels", UintegerValue (1) );
  wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txpower) );          //Default: 16.0206
  wifiPhy.Set ("TxPowerStart", DoubleValue (m_txpower) );        //Default: 16.0206
  //wifiPhy.Set ("RxNoiseFigure", DoubleValue (7.0) );           //Default: 7.0
  wifiPhy.Set ("Antennas", UintegerValue (2) );
  ///Parameters specific for 802.11n:
  wifiPhy.Set ("GreenfieldEnabled", BooleanValue (false) );
  ///Parameters specific for 802.11n/ac/ax:
  wifiPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (1) );
  wifiPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (1) );
  wifiPhy.Set ("ShortGuardEnabled", BooleanValue (false) );

  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper ();

  if (m_propLoss == "logdistance")  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
  if (m_propLoss == "kun2600")      wifiChannel.AddPropagationLoss ("ns3::Kun2600MhzPropagationLossModel");
  if (m_propLoss == "itur1411")     wifiChannel.AddPropagationLoss ("ns3::ItuR1411LosPropagationLossModel", "Frequency", DoubleValue(2.437e9));
  if (m_propLoss == "itur1411NLos") wifiChannel.AddPropagationLoss ("ns3::ItuR1411NlosOverRooftopPropagationLossModel", "Frequency", DoubleValue(2.437e9),
                                                                    "RooftopLevel", DoubleValue (60));
  if (m_propLoss == "friis")        wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(2.437e9));

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Configure the parameters of the Peer Link
  Config::SetDefault ("ns3::dot11s::PeerLink::MaxBeaconLoss", UintegerValue (10));   //Default: 2
  Config::SetDefault ("ns3::dot11s::PeerLink::MaxRetries", UintegerValue (4));
  Config::SetDefault ("ns3::dot11s::PeerLink::MaxPacketFailure", UintegerValue (5)); //Default: 2
  Config::SetDefault ("ns3::dot11s::PeerLink::BeaconWinSize", UintegerValue (m_beaconWinSize));

  // Configure the parameters of the HWMP
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPnetDiameterTraversalTime", TimeValue (Seconds (.4096))); //Default: .1024
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPactivePathTimeout", TimeValue (Seconds (5.12)));    //Default: 5.12
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPactiveRootTimeout", TimeValue (Seconds (5.12)));    //Default: 5.12
  // Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPpathToRootInterval", TimeValue (Seconds (3.072)));       //Default: 2.048
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPmaxPREQretries", UintegerValue (3));
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::UnicastPreqThreshold",UintegerValue (1));
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::UnicastDataThreshold",UintegerValue (1));
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::DoFlag", BooleanValue (m_doFlag));   //Default: false
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::RfFlag", BooleanValue (m_rfFlag));   //Default: true
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::EtxMetric", BooleanValue (m_etxMetric));
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::LinkProbePacket", BooleanValue (m_enableLpp));
  Config::SetDefault ("ns3::dot11s::HwmpProtocol::HopCountMetric", BooleanValue (m_hopCntMetric));

  // Configure parameters of the MeshWifiInterfaceMac
  // Config::SetDefault ("ns3::MeshWifiInterfaceMac::BeaconInterval", TimeValue (Seconds (1.0)));   //Default: 0.5

  // Configure parameters of the LinkMetricCalculator
  Config::SetDefault ("ns3::dot11s::AirtimeLinkMetricCalculator::FerFromBeacon", BooleanValue (m_airTimeBMetric));
  Config::SetDefault ("ns3::dot11s::AirtimeLinkMetricCalculator::SquareRootTime", BooleanValue (m_srAirtime));
  Config::SetDefault ("ns3::dot11s::AirtimeLinkMetricCalculator::MetricRxPowerCoef", UintegerValue(m_metricRxPowerCoef));

  /*
   * Create mesh helper and set stack installer to it
   * Stack installer creates all needed protocols and install them to
   * mesh point device
   */
  mesh = MeshHelper::Default ();
  if (!Mac48Address (m_root.c_str ()).IsBroadcast ())
    {
      mesh.SetStackInstaller (m_stack, "Root", Mac48AddressValue (Mac48Address (m_root.c_str ())));
    }
  else
    {
      //If root is not set, we do not use "Root" attribute, because it
      //is specified only for 11s
      mesh.SetStackInstaller (m_stack);
    }
  if (m_chan)
    {
      mesh.SetSpreadInterfaceChannels (MeshHelper::SPREAD_CHANNELS);
    }
  else
    {
      mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);
    }

  if (m_remStaManager == "arf")           mesh.SetRemoteStationManager ("ns3::ArfWifiManager");
  if (m_remStaManager == "minstrel")      mesh.SetRemoteStationManager ("ns3::MinstrelWifiManager");
  if (m_remStaManager == "minstrelht")    mesh.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");
  if (m_remStaManager == "constantrate")  mesh.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (m_linkRate),
                                                                        "ControlMode", StringValue(m_linkRate));

  if (m_wifiStandard == "80211a")     mesh.SetStandard (WIFI_PHY_STANDARD_80211a);
  if (m_wifiStandard == "80211b")     mesh.SetStandard (WIFI_PHY_STANDARD_80211b);
  if (m_wifiStandard == "80211g")     mesh.SetStandard (WIFI_PHY_STANDARD_80211g);
  if (m_wifiStandard == "80211n2.4")  mesh.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
  if (m_wifiStandard == "80211n5")    mesh.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  mesh.SetMacType ("RandomStart", TimeValue (Seconds (m_randomStart)));
  // Set number of interfaces - default is single-interface mesh point
  mesh.SetNumberOfInterfaces (m_nIfaces);
  // Install protocols and return container if MeshPointDevices
  meshDevices = mesh.Install (wifiPhy, nodes);

  MobilityHelper mobility;
  // Setup ns2 mobility
  if (m_ns2Mobil)
  {
    Ns2MobilityHelper ns2mobility = Ns2MobilityHelper(m_scenario);
    ns2mobility.Install();
  }
  else if (m_gridtopology)
  {
    // Setup mobility - static grid topology
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (m_step),
                                 "DeltaY", DoubleValue (m_step),
                                 "GridWidth", UintegerValue (m_xSize),
                                 "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (nodes);
  }
  else
  {
    // Setup static node locations from file
    switch (m_nNodes) {
      case 20:
        for (int i = 0; i < m_nNodes; i++)
          nodeCoords.push_back (n_eq_20_3d[m_topoId][i]);
        break;
      case 40:
        for (int i = 0; i < m_nNodes; i++)
          nodeCoords.push_back (n_eq_40_3d[m_topoId][i]);
        break;
      case 60:
        for (int i = 0; i < m_nNodes; i++)
          nodeCoords.push_back (n_eq_60_3d[m_topoId][i]);
        break;
    }
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    for (vector< coordinates >::iterator j = nodeCoords.begin (); j != nodeCoords.end (); j++)
      {
        positionAlloc->Add (Vector ((*j).X, (*j).Y, (*j).Z));
      }
    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (nodes);
  }

  if (m_pcap)
    wifiPhy.EnablePcapAll (std::string ("mp-"));
  if (m_ascii)
    {
      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream (m_asciiFile));
    }
}
void
MeshTest::InstallInternetStack ()
{
  InternetStackHelper internetStack;
  internetStack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (meshDevices);
}
void
MeshTest::InstallApplication ()
{
  int shift = 0;
  std::string m_protocol;
  if (m_UdpTcpMode=="udp") m_protocol = "ns3::UdpSocketFactory";
  else m_protocol = "ns3::TcpSocketFactory";

  PacketSinkHelper sink (m_protocol, InetSocketAddress (interfaces.GetAddress (m_sink), 40));
  ApplicationContainer receiver = sink.Install (nodes.Get (m_sink));
  receiver.Start (Seconds (0.0));
  receiver.Stop (Seconds (m_totalTime));

  OnOffHelper onoff (m_protocol, InetSocketAddress (interfaces.GetAddress (m_sink), 40));
  onoff.SetConstantRate (DataRate (m_iDataRate * 1000), m_packetSize);
  ApplicationContainer transmitters [m_nNodes-1];

  Ptr<UniformRandomVariable> rand_start = CreateObject<UniformRandomVariable> ();
  rand_start->SetAttribute ("Min", DoubleValue (-0.01));
  rand_start->SetAttribute ("Max", DoubleValue (0.01));

  for (int i = 0; i < m_nNodes; i++) {
    if (i != m_sink) {
      double timeOffset;
      //timeOffset = rand_start->GetValue();
      timeOffset = ( (i - shift) - m_nNodes +1 ) * 0.005;
      transmitters[i - shift] = onoff.Install (nodes.Get (i));
      transmitters[i - shift].Start ( Seconds(m_startTime + timeOffset));
      transmitters[i - shift].Stop ( Seconds(m_totalTime + timeOffset));
    }
    else {
      shift++;
    }
  }
}
void
MeshTest::RouteChangeSink(std::string context, ns3::dot11s::RouteChange rChange)
{
  if (rChange.destination == g_sinkMac)
  {
    std::ofstream osf (g_rChangeFile.c_str(), std::ios::out | std::ios::app);
    if (!osf.is_open ())
      {
        std::cerr << "Error: Can't open file " << g_rChangeFile << "\n";
        return;
      }
    osf << Simulator::Now() << ",";
    uint8_t index = 10;
    while (context[index] != '/')
    {
      osf << context[index];
      index++;
    }
    osf << "," << rChange.type << "," << rChange.destination << "," << rChange.retransmitter;
    osf << "," << rChange.metric << "," << rChange.seqnum << std::endl;
    osf.close();
  }
}
void
MeshTest::CourseChange(std::string context, Ptr<const MobilityModel> model)
{
  std::ofstream osf (g_courseChangeFile.c_str(), std::ios::out | std::ios::app);
  if (!osf.is_open())
  {
    std::cerr << "Error: Can't open File " << g_courseChangeFile << "\n";
    return;
  }
  uint8_t index = 10;
  while (context[index] != '/')
  {
    osf << context[index];
    index++;
  }
  osf << "," << Simulator::Now();
  osf << "," << model->GetPosition() << "," << model->GetVelocity() << std::endl;
  osf.close();
}
int
MeshTest::Run ()
{
  CreateNodes ();
  InstallInternetStack ();
  InstallApplication ();
  Config::Connect ("/NodeList/*/DeviceList/0/$ns3::MeshPointDevice/RoutingProtocol/$ns3::dot11s::HwmpProtocol/RouteChange", MakeCallback (&RouteChangeSink));
  Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));
  Simulator::Schedule (Seconds (m_totalTime), &MeshTest::Report, this);
  // Prepare file to store Route Changes
  std::ofstream osf (g_rChangeFile.c_str ());
  osf << "Time,Node,Type,Destination,Retransmitter,Metric,SeqNumber" << std::endl;
  osf.close();
  // Prepare file to store Course Changes
  ExportMobility ("start");
  //Flow monitor
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();
  //NetAnim
  // AnimationInterface anim ("MeshAnimation.xml");
  // anim.SetMobilityPollInterval (Seconds (0.5));

  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();

  flowMonitor->SerializeToXmlFile("MeshPerformance.xml", true, true);

  //After Simulation save final node locations
  ExportMobility ("end");

  Simulator::Destroy ();
  return 0;
}
void
MeshTest::Report ()
{
  unsigned n (0);
  for (NetDeviceContainer::Iterator i = meshDevices.Begin (); i != meshDevices.End (); ++i, ++n)
    {
      std::ostringstream os;
      os << "mp-report-" << n << ".xml";
      std::cerr << "Printing mesh point device #" << n << " diagnostics to " << os.str () << "\n";
      std::ofstream of;
      of.open (os.str ().c_str ());
      if (!of.is_open ())
        {
          std::cerr << "Error: Can't open file " << os.str () << "\n";
          return;
        }
      mesh.Report (*i, of);
      of.close ();
    }
}
void
MeshTest::ExportMobility (std::string stage)
{
  std::ofstream osf;
  if (stage == "start") {
    osf.open (g_courseChangeFile.c_str());
    osf << "Node,Time,Position,Velocity" << std::endl;
  }
  else {
    osf.open (g_courseChangeFile.c_str(), std::ios::app);
  }
  for (NodeContainer::Iterator j = nodes.Begin(); j != nodes.End(); ++j) {
      Ptr<Node> node = *j;
      Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
      osf << node->GetId() << "," << Simulator::Now();
      osf << "," << mobility->GetPosition() << "," << mobility->GetVelocity() << std::endl;
  }
  osf.close();
}
int
main (int argc, char *argv[])
{
  // LogComponentEnable ("HwmpProtocol", LOG_LEVEL_FUNCTION);
  // LogComponentEnable ("HwmpProtocolMac", LOG_LEVEL_FUNCTION);
  // LogComponentEnable ("PeerManagementProtocol", LOG_LEVEL_FUNCTION);
  // LogComponentEnable ("PeerManagementProtocolMac", LOG_LEVEL_FUNCTION);
  // LogComponentEnable ("Ns2MobilityHelper", LOG_LEVEL_DEBUG);
  MeshTest t;
  t.Configure (argc, argv);
  return t.Run ();
}
