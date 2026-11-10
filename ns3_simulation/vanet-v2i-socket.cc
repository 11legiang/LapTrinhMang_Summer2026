#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <map>
#ifdef __linux__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/itu-r-1411-los-propagation-loss-model.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/integer.h"
#include "ns3/wave-bsm-helper.h"
#include "ns3/wave-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/netanim-module.h"
using namespace ns3;
using namespace dsr;

NS_LOG_COMPONENT_DEFINE ("vanet-routing-compare");

/**
 * \ingroup wave
 * \brief The RoutingStats class manages collects statistics
 * on routing data (application-data packet and byte counts)
 * for the vehicular network
 */
class RoutingStats
{
public:
  /**
   * \brief Constructor
   * \return none
   */
  RoutingStats ();

  /**
   * \brief Returns the number of bytes received
   * \return the number of bytes received
   */
  uint32_t GetRxBytes ();

  /**
   * \brief Returns the cumulative number of bytes received
   * \return the cumulative number of bytes received
   */
  uint32_t GetCumulativeRxBytes ();

  /**
   * \brief Returns the count of packets received
   * \return the count of packets received
   */
  uint32_t GetRxPkts ();

  /**
   * \brief Returns the cumulative count of packets received
   * \return the cumulative count of packets received
   */
  uint32_t GetCumulativeRxPkts ();

  /**
   * \brief Increments the number of (application-data)
   * bytes received, not including MAC/PHY overhead
   * \param rxBytes the number of bytes received
   * \return none
   */
  void IncRxBytes (uint32_t rxBytes);

  /**
   * \brief Increments the count of packets received
   * \return none
   */
  void IncRxPkts ();

  /**
   * \brief Sets the number of bytes received.
   * \param rxBytes the number of bytes received
   * \return none
   */
  void SetRxBytes (uint32_t rxBytes);

  /**
   * \brief Sets the number of packets received
   * \param rxPkts the number of packets received
   * \return none
   */
  void SetRxPkts (uint32_t rxPkts);

  /**
   * \brief Returns the number of bytes transmitted
   * \return the number of bytes transmitted
   */
  uint32_t GetTxBytes ();

  /**
   * \brief Returns the cumulative number of bytes transmitted
   * \return none
   */
  uint32_t GetCumulativeTxBytes ();

  /**
   * \brief Returns the number of packets transmitted
   * \return the number of packets transmitted
   */
  uint32_t GetTxPkts ();

  /**
   * \brief Returns the cumulative number of packets transmitted
   * \return the cumulative number of packets transmitted
   */
  uint32_t GetCumulativeTxPkts ();

  /**
   * \brief Increment the number of bytes transmitted
   * \param txBytes the number of additional bytes transmitted
   * \return none
   */
  void IncTxBytes (uint32_t txBytes);

  /**
   * \brief Increment the count of packets transmitted
   * \return none
   */
  void IncTxPkts ();

  /**
   * \brief Sets the number of bytes transmitted
   * \param txBytes the number of bytes transmitted
   * \return none
   */
  void SetTxBytes (uint32_t txBytes);

  /**
   * \brief Sets the number of packets transmitted
   * \param txPkts the number of packets transmitted
   * \return none
   */
  void SetTxPkts (uint32_t txPkts);

private:
  uint32_t m_RxBytes; ///< reeive bytes
  uint32_t m_cumulativeRxBytes; ///< cumulative receive bytes
  uint32_t m_RxPkts; ///< receive packets
  uint32_t m_cumulativeRxPkts; ///< cumulative receive packets
  uint32_t m_TxBytes; ///< transmit bytes
  uint32_t m_cumulativeTxBytes; ///< cumulative transmit bytes
  uint32_t m_TxPkts; ///< transmit packets
  uint32_t m_cumulativeTxPkts; ///< cumulative transmit packets
};

RoutingStats::RoutingStats ()
  : m_RxBytes (0),
    m_cumulativeRxBytes (0),
    m_RxPkts (0),
    m_cumulativeRxPkts (0),
    m_TxBytes (0),
    m_cumulativeTxBytes (0),
    m_TxPkts (0),
    m_cumulativeTxPkts (0)
{
}

uint32_t
RoutingStats::GetRxBytes ()
{
  return m_RxBytes;
}

uint32_t
RoutingStats::GetCumulativeRxBytes ()
{
  return m_cumulativeRxBytes;
}

uint32_t
RoutingStats::GetRxPkts ()
{
  return m_RxPkts;
}

uint32_t
RoutingStats::GetCumulativeRxPkts ()
{
  return m_cumulativeRxPkts;
}

void
RoutingStats::IncRxBytes (uint32_t rxBytes)
{
  m_RxBytes += rxBytes;
  m_cumulativeRxBytes += rxBytes;
}

void
RoutingStats::IncRxPkts ()
{
  m_RxPkts++;
  m_cumulativeRxPkts++;
}

void
RoutingStats::SetRxBytes (uint32_t rxBytes)
{
  m_RxBytes = rxBytes;
}

void
RoutingStats::SetRxPkts (uint32_t rxPkts)
{
  m_RxPkts = rxPkts;
}

uint32_t
RoutingStats::GetTxBytes ()
{
  return m_TxBytes;
}

uint32_t
RoutingStats::GetCumulativeTxBytes ()
{
  return m_cumulativeTxBytes;
}

uint32_t
RoutingStats::GetTxPkts ()
{
  return m_TxPkts;
}

uint32_t
RoutingStats::GetCumulativeTxPkts ()
{
  return m_cumulativeTxPkts;
}

void
RoutingStats::IncTxBytes (uint32_t txBytes)
{
  m_TxBytes += txBytes;
  m_cumulativeTxBytes += txBytes;
}

void
RoutingStats::IncTxPkts ()
{
  m_TxPkts++;
  m_cumulativeTxPkts++;
}

void
RoutingStats::SetTxBytes (uint32_t txBytes)
{
  m_TxBytes = txBytes;
}

void
RoutingStats::SetTxPkts (uint32_t txPkts)
{
  m_TxPkts = txPkts;
}

/**
 * \ingroup wave
 * \brief The RoutingHelper class generates routing data between
 * nodes (vehicles) and uses the RoutingStats class to collect statistics
 * on routing data (application-data packet and byte counts).
 * A routing protocol is configured, and all nodes attempt to send
 * (i.e. route) small packets to another node, which acts as
 * data sinks.  Not all nodes act as data sinks.
 * for the vehicular network
 */
class RoutingHelper : public Object
{
public:
  /**
   * \brief Get class TypeId
   * \return the TypeId for the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   * \return none
   */
  RoutingHelper ();

  /**
   * \brief Destructor
   * \return none
   */
  virtual ~RoutingHelper ();

  /**
   * \brief Installs routing functionality on nodes and their
   * devices and interfaces.
   * \param c node container
   * \param d net device container
   * \param i IPv4 interface container
   * \param totalTime the total time that nodes should attempt to
   * route data
   * \param protocol the routing protocol (1=OLSR;2=AODV;3=DSDV;4=DSR)
   * \param nSinks the number of nodes which will act as data sinks
   * \param routingTables dump routing tables at t=5 seconds (0=no;1=yes)
   * \return none
   */
  void Install (NodeContainer & c,
                NetDeviceContainer & d,
                Ipv4InterfaceContainer & i,
                double totalTime,
                int protocol,
                uint32_t nSinks,
                int routingTables);

  /**
   * \brief Trace the receipt of an on-off-application generated packet
   * \param context this object
   * \param packet a received packet
   * \return none
   */
  void OnOffTrace (std::string context, Ptr<const Packet> packet);

  /**
   * \brief Returns the RoutingStats instance
   * \return the RoutingStats instance
   */
  RoutingStats & GetRoutingStats ();

  /**
   * \brief Enable/disable logging
   * \param log non-zero to enable logging
   * \return none
   */
  void SetLogging (int log);

private:
  /**
   * \brief Sets up the protocol protocol on the nodes
   * \param c node container
   * \return none
   */
  void SetupRoutingProtocol (NodeContainer & c);

  /**
   * \brief Assigns IPv4 addresses to net devices and their interfaces
   * \param d net device container
   * \param adhocTxInterfaces IPv4 interface container
   * \return none
   */
  void AssignIpAddresses (NetDeviceContainer & d,
                          Ipv4InterfaceContainer & adhocTxInterfaces);

  /**
   * \brief Sets up routing messages on the nodes and their interfaces
   * \param c node container
   * \param adhocTxInterfaces IPv4 interface container
   * \return none
   */
  void SetupRoutingMessages (NodeContainer & c,
                             Ipv4InterfaceContainer & adhocTxInterfaces);

  /**
   * \brief Sets up a routing packet for tranmission
   * \param addr destination address
   * \param node source node
   * \return Socket to be used for sending/receiving a routed data packet
   */
  Ptr<Socket> SetupRoutingPacketReceive (Ipv4Address addr, Ptr<Node> node);

  /**
   * \brief Process a received routing packet
   * \param socket the receiving socket
   * \return none
   */
  void ReceiveRoutingPacket (Ptr<Socket> socket);

  double m_TotalSimTime;        ///< seconds
  uint32_t m_protocol;       ///< routing protocol; 0=NONE, 1=OLSR, 2=AODV, 3=DSDV, 4=DSR
  uint32_t m_port;           ///< port
  uint32_t m_nSinks;              ///< number of sink nodes (< all nodes)
  int m_routingTables;      ///< dump routing table (at t=5 sec).  0=No, 1=Yes
  RoutingStats routingStats; ///< routing statistics
  std::string m_protocolName; ///< protocol name
  int m_log; ///< log
};

NS_OBJECT_ENSURE_REGISTERED (RoutingHelper);

TypeId
RoutingHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RoutingHelper")
    .SetParent<Object> ()
    .AddConstructor<RoutingHelper> ();
  return tid;
}

RoutingHelper::RoutingHelper ()
  : m_TotalSimTime (300.01),
    m_protocol (0),
    m_port (9),
    m_nSinks (0),
    m_routingTables (0),
    m_log (0)
{
}

RoutingHelper::~RoutingHelper ()
{
}

void
RoutingHelper::Install (NodeContainer & c,
                        NetDeviceContainer & d,
                        Ipv4InterfaceContainer & i,
                        double totalTime,
                        int protocol,
                        uint32_t nSinks,
                        int routingTables)
{
  m_TotalSimTime = totalTime;
  m_protocol = protocol;
  m_nSinks = nSinks;
  m_routingTables = routingTables;

  SetupRoutingProtocol (c);
  AssignIpAddresses (d, i);
  SetupRoutingMessages (c, i);
}

Ptr<Socket>
RoutingHelper::SetupRoutingPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, m_port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&RoutingHelper::ReceiveRoutingPacket, this));

  return sink;
}

void
RoutingHelper::SetupRoutingProtocol (NodeContainer & c)
{
  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  DsrHelper dsr;
  DsrMainHelper dsrMain;
  Ipv4ListRoutingHelper list;
  InternetStackHelper internet;

  Time rtt = Time (5.0);
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> rtw = ascii.CreateFileStream ("routing_table");

  switch (m_protocol)
    {
    case 0:
      m_protocolName = "NONE";
      break;
    case 1:
      if (m_routingTables != 0)
        {
          olsr.PrintRoutingTableAllAt (rtt, rtw);
        }
      list.Add (olsr, 100);
      m_protocolName = "OLSR";
      break;
    case 2:
      if (m_routingTables != 0)
        {
          aodv.PrintRoutingTableAllAt (rtt, rtw);
        }
      list.Add (aodv, 100);
      m_protocolName = "AODV";
      break;
    case 3:
      if (m_routingTables != 0)
        {
          dsdv.PrintRoutingTableAllAt (rtt, rtw);
        }
      list.Add (dsdv, 100);
      m_protocolName = "DSDV";
      break;
    case 4:
      // setup is later
      m_protocolName = "DSR";
      break;
    default:
      NS_FATAL_ERROR ("No such protocol:" << m_protocol);
      break;
    }

  if (m_protocol < 4)
    {
      internet.SetRoutingHelper (list);
      internet.Install (c);
    }
  else if (m_protocol == 4)
    {
      internet.Install (c);
      dsrMain.Install (dsr, c);
    }

  if (m_log != 0)
    {
      NS_LOG_UNCOND ("Routing Setup for " << m_protocolName);
    }
}

void
RoutingHelper::AssignIpAddresses (NetDeviceContainer & d,
                                  Ipv4InterfaceContainer & adhocTxInterfaces)
{
  NS_LOG_INFO ("Assigning IP addresses");
  Ipv4AddressHelper addressAdhoc;
  // we may have a lot of nodes, and want them all
  // in same subnet, to support broadcast
  addressAdhoc.SetBase ("10.1.0.0", "255.255.0.0");
  adhocTxInterfaces = addressAdhoc.Assign (d);
}

void
RoutingHelper::SetupRoutingMessages (NodeContainer & c,
                                     Ipv4InterfaceContainer & adhocTxInterfaces)
{
  // Setup routing transmissions
  OnOffHelper onoff1 ("ns3::UdpSocketFactory",Address ());
  onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

  Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
  int64_t stream = 2;
  var->SetStream (stream);
  for (uint32_t i = 0; i < m_nSinks; i++)
    {
      // protocol == 0 means no routing data, WAVE BSM only
      // so do not set up sink
      if (m_protocol != 0)
        {
          Ptr<Socket> sink = SetupRoutingPacketReceive (adhocTxInterfaces.GetAddress (i), c.Get (i));
        }

      AddressValue remoteAddress (InetSocketAddress (adhocTxInterfaces.GetAddress (i), m_port));
      onoff1.SetAttribute ("Remote", remoteAddress);

      ApplicationContainer temp = onoff1.Install (c.Get (i + m_nSinks));
      temp.Start (Seconds (var->GetValue (1.0,2.0)));
      temp.Stop (Seconds (m_TotalSimTime));
    }
}

static inline std::string
PrintReceivedRoutingPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address srcAddress)
{
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (InetSocketAddress::IsMatchingType (srcAddress))
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (srcAddress);
      oss << " received one packet from " << addr.GetIpv4 ();
    }
  else
    {
      oss << " received one packet!";
    }
  return oss.str ();
}

void
RoutingHelper::ReceiveRoutingPacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address srcAddress;
  while ((packet = socket->RecvFrom (srcAddress)))
    {
      // application data, for goodput
      uint32_t RxRoutingBytes = packet->GetSize ();
      GetRoutingStats ().IncRxBytes (RxRoutingBytes);
      GetRoutingStats ().IncRxPkts ();
      if (m_log != 0)
        {
          NS_LOG_UNCOND (m_protocolName + " " + PrintReceivedRoutingPacket (socket, packet, srcAddress));
        }
    }
}

void
RoutingHelper::OnOffTrace (std::string context, Ptr<const Packet> packet)
{
  uint32_t pktBytes = packet->GetSize ();
  routingStats.IncTxBytes (pktBytes);
}

RoutingStats &
RoutingHelper::GetRoutingStats ()
{
  return routingStats;
}

void
RoutingHelper::SetLogging (int log)
{
  m_log = log;
}

/**
 * \ingroup wave
 * \brief The WifiPhyStats class collects Wifi MAC/PHY statistics
 */
class WifiPhyStats : public Object
{
public:
  /**
   * \brief Gets the class TypeId
   * \return the class TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   * \return none
   */
  WifiPhyStats ();

  /**
   * \brief Destructor
   * \return none
   */
  virtual ~WifiPhyStats ();

  /**
   * \brief Returns the number of bytes that have been transmitted
   * (this includes MAC/PHY overhead)
   * \return the number of bytes transmitted
   */
  uint32_t GetTxBytes ();

  /**
   * \brief Callback signiture for Phy/Tx trace
   * \param context this object
   * \param packet packet transmitted
   * \param mode wifi mode
   * \param preamble wifi preamble
   * \param txPower transmission power
   * \return none
   */
  void PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower);

  /**
   * \brief Callback signiture for Phy/TxDrop
   * \param context this object
   * \param packet the tx packet being dropped
   * \return none
   */
  void PhyTxDrop (std::string context, Ptr<const Packet> packet);

  /**
   * \brief Callback signiture for Phy/RxDrop
   * \param context this object
   * \param packet the rx packet being dropped
   * \return none
   */
  void PhyRxDrop (std::string context, Ptr<const Packet> packet);

private:
  uint32_t m_phyTxPkts; ///< phy transmit packets
  uint32_t m_phyTxBytes; ///< phy transmit bytes
};

NS_OBJECT_ENSURE_REGISTERED (WifiPhyStats);

TypeId
WifiPhyStats::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiPhyStats")
    .SetParent<Object> ()
    .AddConstructor<WifiPhyStats> ();
  return tid;
}

WifiPhyStats::WifiPhyStats ()
  : m_phyTxPkts (0),
    m_phyTxBytes (0)
{
}

WifiPhyStats::~WifiPhyStats ()
{
}

void
WifiPhyStats::PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower)
{
  NS_LOG_FUNCTION (this << context << packet << "PHYTX mode=" << mode );
  ++m_phyTxPkts;
  uint32_t pktSize = packet->GetSize ();
  m_phyTxBytes += pktSize;

  //NS_LOG_UNCOND ("Received PHY size=" << pktSize);
}

void
WifiPhyStats::PhyTxDrop (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_UNCOND ("PHY Tx Drop");
}

void
WifiPhyStats::PhyRxDrop (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_UNCOND ("PHY Rx Drop");
}

uint32_t
WifiPhyStats::GetTxBytes ()
{
  return m_phyTxBytes;
}

/**
 * \ingroup wave
 * \brief The WifiApp class enforces program flow for ns-3 wifi applications
 */
class WifiApp
{
public:
  /**
   * \brief Constructor
   * \return none
   */
  WifiApp ();

  /**
   * \brief Destructor
   * \return none
   */
  virtual ~WifiApp ();

  /**
   * \brief Enacts simulation of an ns-3 wifi application
   * \param argc program arguments count
   * \param argv program arguments
   * \return none
   */
  void Simulate (int argc, char **argv);

protected:
  /**
   * \brief Sets default attribute values
   * \return none
   */
  virtual void SetDefaultAttributeValues ();

  /**
   * \brief Process command line arguments
   * \param argc program arguments count
   * \param argv program arguments
   * \return none
   */
  virtual void ParseCommandLineArguments (int argc, char **argv);

  /**
   * \brief Configure nodes
   * \return none
   */
  virtual void ConfigureNodes ();

  /**
   * \brief Configure channels
   * \return none
   */
  virtual void ConfigureChannels ();

  /**
   * \brief Configure devices
   * \return none
   */
  virtual void ConfigureDevices ();

  /**
   * \brief Configure mobility
   * \return none
   */
  virtual void ConfigureMobility ();

  /**
   * \brief Configure applications
   * \return none
   */
  virtual void ConfigureApplications ();

  /**
   * \brief Configure tracing
   * \return none
   */
  virtual void ConfigureTracing ();

  /**
   * \brief Run the simulation
   * \return none
   */
  virtual void RunSimulation ();

  /**
   * \brief Process outputs
   * \return none
   */
  virtual void ProcessOutputs ();
};

WifiApp::WifiApp ()
{
}

WifiApp::~WifiApp ()
{
}

void
WifiApp::Simulate (int argc, char **argv)
{
  // Simulator Program Flow:
  // (source:  NS-3 Annual Meeting, May, 2014, session 2 slides 6, 28)
  //   (HandleProgramInputs:)
  //   SetDefaultAttributeValues
  //   ParseCommandLineArguments
  //   (ConfigureTopology:)
  //   ConfigureNodes
  //   ConfigureChannels
  //   ConfigureDevices
  //   ConfigureMobility
  //   ConfigureApplications
  //     e.g AddInternetStackToNodes
  //         ConfigureIpAddressingAndRouting
  //         configureSendMessages
  //   ConfigureTracing
  //   RunSimulation
  //   ProcessOutputs

  SetDefaultAttributeValues ();
  ParseCommandLineArguments (argc, argv);
  ConfigureNodes ();
  ConfigureChannels ();
  ConfigureDevices ();
  ConfigureMobility ();
  ConfigureApplications ();
  ConfigureTracing ();
  RunSimulation ();
  ProcessOutputs ();
}

void
WifiApp::SetDefaultAttributeValues ()
{
}

void
WifiApp::ParseCommandLineArguments (int argc, char **argv)
{
}

void
WifiApp::ConfigureNodes ()
{
}

void
WifiApp::ConfigureChannels ()
{
}

void
WifiApp::ConfigureDevices ()
{
}

void
WifiApp::ConfigureMobility ()
{
}

void
WifiApp::ConfigureApplications ()
{
}

void
WifiApp::ConfigureTracing ()
{
}

void
WifiApp::RunSimulation ()
{
}

void
WifiApp::ProcessOutputs ()
{
}

/**
 * \ingroup wave
 * \brief The ConfigStoreHelper class simplifies config-store raw text load and save
 */
class ConfigStoreHelper
{
public:
  /**
   * \brief Constructor
   * \return none
   */
  ConfigStoreHelper ();

  /**
   * \brief Loads a saved config-store raw text configuration from a given named file
   * \param configFilename the name of the config-store raw text file
   * \return none
   */
  void LoadConfig (std::string configFilename);

  /**
   * \brief Saves a configuration to a given named config-store raw text configuration file
   * \param configFilename the name of the config-store raw text file
   * \return none
   */
  void SaveConfig (std::string configFilename);
};

ConfigStoreHelper::ConfigStoreHelper ()
{
}

void
ConfigStoreHelper::LoadConfig (std::string configFilename)
{
  // Input config store from txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (configFilename));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  //inputConfig.ConfigureAttributes ();
}

void
ConfigStoreHelper::SaveConfig (std::string configFilename)
{
  // only save if a non-empty filename has been specified
  if (configFilename.compare ("") != 0)
    {
      // Output config store to txt format
      Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (configFilename));
      Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
      Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
      ConfigStore outputConfig;
      outputConfig.ConfigureDefaults ();
      //outputConfig.ConfigureAttributes ();
    }
}

/**
 * \ingroup wave
 * \brief The VanetRoutingExperiment class implements a wifi app that
 * allows VANET routing experiments to be simulated
 */
class VanetRoutingExperiment : public WifiApp
{
public:
  /**
   * \brief Constructor
   * \return none
   */
  VanetRoutingExperiment ();

protected:
  /**
   * \brief Sets default attribute values
   * \return none
   */
  virtual void SetDefaultAttributeValues ();

  /**
   * \brief Process command line arguments
   * \param argc program arguments count
   * \param argv program arguments
   * \return none
   */
  virtual void ParseCommandLineArguments (int argc, char **argv);

  /**
   * \brief Configure nodes
   * \return none
   */
  virtual void ConfigureNodes ();

  /**
   * \brief Configure channels
   * \return none
   */
  virtual void ConfigureChannels ();

  /**
   * \brief Configure devices
   * \return none
   */
  virtual void ConfigureDevices ();

  /**
   * \brief Configure mobility
   * \return none
   */
  virtual void ConfigureMobility ();

  /**
   * \brief Configure applications
   * \return none
   */
  virtual void ConfigureApplications ();

  /**
   * \brief Configure tracing
   * \return none
   */
  virtual void ConfigureTracing ();

  /**
   * \brief Run the simulation
   * \return none
   */
  virtual void RunSimulation ();

  /**
   * \brief Process outputs
   * \return none
   */
  virtual void ProcessOutputs ();

private:
  /**
   * \brief Run the simulation
   * \return none
   */
  void Run ();

  /**
   * \brief Run the simulation
   * \param argc command line argument count
   * \param argv command line parameters
   * \return none
   */
  void CommandSetup (int argc, char **argv);

  /**
   * \brief Checks the throughput and outputs summary to CSV file1.
   * This is scheduled and called once per second
   * \return none
   */
  void CheckThroughput ();

  /**
   * \brief Set up log file
   * \return none
   */
  void SetupLogFile ();

  /**
   * \brief Set up logging
   * \return none
   */
  void SetupLogging ();

  /**
   * \brief Configure default attributes
   * \return none
   */
  void ConfigureDefaults ();

  /**
   * \brief Set up the adhoc mobility nodes
   * \return none
   */
  void SetupAdhocMobilityNodes ();

  /**
   * \brief Set up the adhoc devices
   * \return none
   */
  void SetupAdhocDevices ();

  /**
   * \brief Set up generation of IEEE 1609 WAVE messages,
   * as a Basic Safety Message (BSM).  The BSM is typically
   * a ~200-byte packets broadcast by all vehicles at a nominal
   * rate of 10 Hz
   * \return none
   */
  void SetupWaveMessages ();

  /**
   * \brief Set up generation of packets to be routed
   * through the vehicular network
   * \return none
   */
  void SetupRoutingMessages ();

  /**
   * \brief Set up a prescribed scenario
   * \return none
   */
  void SetupScenario ();

  /**
   * \brief Write the header line to the CSV file1
   * \return none
   */
  void WriteCsvHeader ();

  /**
   * \brief Set up configuration parameter from the global variables
   * \return none
   */
  void SetConfigFromGlobals ();

  /**
   * \brief Set up the global variables from the configuration parameters
   * \return none
   */
  void SetGlobalsFromConfig ();

  /**
   * Course change function
   * \param os the output stream
   * \param context trace source context (unused)
   * \param mobility the mobility model
   */
  static void
  CourseChange (std::ostream *os, std::string context, Ptr<const MobilityModel> mobility);

  /**
   * \brief Setup V2I UDP Sockets on RSU servers and OBU vehicle clients
   */
  void SetupV2iSockets ();

  /**
   * \brief Periodically transmit V2I telemetry & safety packets from OBU to nearest RSU
   */
  void SendV2iTelemetry ();

  /**
   * \brief RSU UDP Socket callback when receiving a V2I packet from a vehicle
   */
  void ReceiveV2iPacket (Ptr<Socket> socket);

  /**
   * \brief Vehicle OBU UDP Socket callback when receiving an I2V warning from an RSU
   */
  void ReceiveI2vWarning (Ptr<Socket> socket);

  /**
   * \brief Forward V2I telemetry to external Central Monitoring Server via POSIX UDP socket
   */
  void ForwardToExternalServer (const std::string &payload);

  uint32_t m_port; ///< port
  std::string m_CSVfileName; ///< CSV file name
  std::string m_CSVfileName2; ///< CSV file name
  uint32_t m_nSinks; ///< number of sinks
  std::string m_protocolName; ///< protocol name
  double m_txp; ///< distance
  bool m_traceMobility; ///< trace mobility
  uint32_t m_protocol; ///< protocol

  uint32_t m_lossModel; ///< loss model
  uint32_t m_fading; ///< fading
  std::string m_lossModelName; ///< loss model name

  std::string m_phyMode; ///< phy mode
  uint32_t m_80211mode; ///< 80211 mode

  std::string m_traceFile; ///< trace file 
  std::string m_logFile; ///< log file
  uint32_t m_mobility; ///< mobility
  uint32_t m_nNodes; ///< number of nodes
  uint32_t m_nRsuNodes; ///< number of fixed Road-Side Unit (RSU) stations
  uint32_t m_v2iPort; ///< UDP port for V2I RSU socket server
  uint32_t m_liveUdpPort; ///< External UDP port for Central Monitoring Server
  uint32_t m_v2iSeq; ///< V2I sequence counter
  uint32_t m_v2iTxPkts; ///< V2I packets transmitted by OBUs
  uint32_t m_v2iRxPkts; ///< V2I packets received by RSUs
  uint32_t m_i2vAlertsSent; ///< I2V warning packets broadcast by RSUs
  double m_v2iDelaySumMs; ///< cumulative V2I socket delay in ms
  int m_extUdpSock; ///< external POSIX UDP socket fd
  std::ofstream m_v2iCsvOs; ///< V2I socket trace CSV output stream
  std::vector<Ptr<Socket> > m_rsuSockets; ///< RSU UDP server sockets
  std::vector<Ptr<Socket> > m_obuSockets; ///< Vehicle OBU UDP client sockets
  std::vector<Vector> m_rsuPositions; ///< Fixed RSU coordinates
  double m_TotalSimTime; ///< total sim time
  std::string m_rate; ///< rate
  std::string m_phyModeB; ///< phy mode
  std::string m_trName; ///< trace file name
  int m_nodeSpeed; ///< in m/s
  int m_nodePause; ///< in s
  uint32_t m_wavePacketSize; ///< bytes
  double m_waveInterval; ///< seconds
  int m_verbose; ///< verbose
  std::ofstream m_os; ///< output stream
  NetDeviceContainer m_adhocTxDevices; ///< adhoc transmit devices
  Ipv4InterfaceContainer m_adhocTxInterfaces; ///< adhoc transmit interfaces
  uint32_t m_scenario; ///< scenario
  double m_gpsAccuracyNs; ///< GPS accuracy
  double m_txMaxDelayMs; ///< transmit maximum delay
  int m_routingTables; ///< routing tables
  int m_asciiTrace; ///< ascii trace
  int m_pcap; ///< PCAP
  std::string m_loadConfigFilename; ///< load config file name
  std::string m_saveConfigFilename; ///< save configi file name

  WaveBsmHelper m_waveBsmHelper; ///< helper
  Ptr<RoutingHelper> m_routingHelper; ///< routing helper
  Ptr<WifiPhyStats> m_wifiPhyStats; ///< wifi phy statistics
  int m_log; ///< log
  /// used to get consistent random numbers across scenarios
  int64_t m_streamIndex;
  NodeContainer m_adhocTxNodes; ///< adhoc transmit nodes
  double m_txSafetyRange1; ///< range 1
  double m_txSafetyRange2; ///< range 2
  double m_txSafetyRange3; ///< range 3
  double m_txSafetyRange4; ///< range 4
  double m_txSafetyRange5; ///< range 5
  double m_txSafetyRange6; ///< range 6
  double m_txSafetyRange7; ///< range 7
  double m_txSafetyRange8; ///< range 8
  double m_txSafetyRange9; ///< range 9
  double m_txSafetyRange10; ///< range 10
  std::vector <double> m_txSafetyRanges; ///< list of ranges
  std::string m_exp; ///< exp
  int m_cumulativeBsmCaptureStart; ///< capture start
};

VanetRoutingExperiment::VanetRoutingExperiment ()
  : m_port (9),
    m_CSVfileName ("vanet-routing.output.csv"),
    m_CSVfileName2 ("vanet-routing.output2.csv"),
    m_nSinks (10),
    m_protocolName ("OLSR"),
    m_txp (20),
    m_traceMobility (false),
    // OLSR default for scenario 2
    m_protocol (1),
    // Two-Ray ground
    m_lossModel (3),
    m_fading (0),
    m_lossModelName (""),
    m_phyMode ("OfdmRate6MbpsBW10MHz"),
    // 1=802.11p
    m_80211mode (1),
    m_traceFile (""),
    m_logFile ("low99-ct-unterstrass-1day.filt.7.adj.log"),
    m_mobility (1),
    m_nNodes (156),
    m_nRsuNodes (4),
    m_v2iPort (9000),
    m_liveUdpPort (6000),
    m_v2iSeq (0),
    m_v2iTxPkts (0),
    m_v2iRxPkts (0),
    m_i2vAlertsSent (0),
    m_v2iDelaySumMs (0.0),
    m_extUdpSock (-1),
    m_TotalSimTime (300.01),
    m_rate ("2048bps"),
    m_phyModeB ("DsssRate11Mbps"),
    m_trName ("vanet-routing-compare"),
    m_nodeSpeed (20),
    m_nodePause (0),
    m_wavePacketSize (200),
    m_waveInterval (0.1),
    m_verbose (0),
    m_scenario (2),
    m_gpsAccuracyNs (40),
    m_txMaxDelayMs (10),
    m_routingTables (0),
    m_asciiTrace (0),
    m_pcap (0),
    m_loadConfigFilename ("load-config.txt"),
    m_saveConfigFilename (""),
    m_log (0),
    m_streamIndex (0),
    m_adhocTxNodes (),
    m_txSafetyRange1 (50.0),
    m_txSafetyRange2 (100.0),
    m_txSafetyRange3 (150.0),
    m_txSafetyRange4 (200.0),
    m_txSafetyRange5 (250.0),
    m_txSafetyRange6 (300.0),
    m_txSafetyRange7 (350.0),
    m_txSafetyRange8 (400.0),
    m_txSafetyRange9 (450.0),
    m_txSafetyRange10 (500.0),
    m_txSafetyRanges (),
    m_exp (""),
    m_cumulativeBsmCaptureStart (0)
{
  m_wifiPhyStats = CreateObject<WifiPhyStats> ();
  m_routingHelper = CreateObject<RoutingHelper> ();

  // set to non-zero value to enable
  // simply uncond logging during simulation run
  m_log = 1;

  // Initialize 4 fixed RSU stations across the urban road network
  m_rsuPositions.push_back (Vector (520.0, 560.0, 6.0));  // RSU-01: Nut giao Tay Bac
  m_rsuPositions.push_back (Vector (880.0, 495.0, 6.0));  // RSU-02: Giao lo Trung tam Hanh chinh
  m_rsuPositions.push_back (Vector (1320.0, 610.0, 6.0)); // RSU-03: Dai lo Dong Tay
  m_rsuPositions.push_back (Vector (1780.0, 420.0, 6.0)); // RSU-04: Cua ngo Khu Do thi Phia Dong
}

void
VanetRoutingExperiment::SetDefaultAttributeValues ()
{
  // handled in constructor
}

// important configuration items stored in global values
static ns3::GlobalValue g_port ("VRCport",
                                "Port",
                                ns3::UintegerValue (9),
                                ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_nSinks ("VRCnSinks",
                                  "Number of sink nodes for routing non-BSM traffic",
                                  ns3::UintegerValue (10),
                                  ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_traceMobility ("VRCtraceMobility",
                                         "Trace mobility 1=yes;0=no",
                                         ns3::UintegerValue (0),
                                         ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_protocol ("VRCprotocol",
                                    "Routing protocol",
                                    ns3::UintegerValue (2),
                                    ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_lossModel ("VRClossModel",
                                     "Propagation Loss Model",
                                     ns3::UintegerValue (3),
                                     ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_fading ("VRCfading",
                                  "Fast Fading Model",
                                  ns3::UintegerValue (0),
                                  ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_80211mode ("VRC80211mode",
                                     "802.11 mode (0=802.11a;1=802.11p)",
                                     ns3::UintegerValue (1),
                                     ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_mobility ("VRCmobility",
                                    "Mobility mode 0=random waypoint;1=mobility trace file",
                                    ns3::UintegerValue (1),
                                    ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_nNodes ("VRCnNodes",
                                  "Number of nodes (vehicles)",
                                  ns3::UintegerValue (156),
                                  ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_nodeSpeed ("VRCnodeSpeed",
                                     "Node speed (m/s) for RWP model",
                                     ns3::UintegerValue (20),
                                     ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_nodePause ("VRCnodePause",
                                     "Node pause time (s) for RWP model",
                                     ns3::UintegerValue (0),
                                     ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_wavePacketSize ("VRCwavePacketSize",
                                          "Size in bytes of WAVE BSM",
                                          ns3::UintegerValue (200),
                                          ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_verbose ("VRCverbose",
                                   "Verbose 0=no;1=yes",
                                   ns3::UintegerValue (0),
                                   ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_scenario ("VRCscenario",
                                    "Scenario",
                                    ns3::UintegerValue (1),
                                    ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_routingTables ("VRCroutingTables",
                                         "Dump routing tables at t=5 seconds 0=no;1=yes",
                                         ns3::UintegerValue (0),
                                         ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_asciiTrace ("VRCasciiTrace",
                                      "Dump ASCII trace 0=no;1=yes",
                                      ns3::UintegerValue (0),
                                      ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_pcap ("VRCpcap",
                                "Generate PCAP files 0=no;1=yes",
                                ns3::UintegerValue (0),
                                ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_cumulativeBsmCaptureStart ("VRCcumulativeBsmCaptureStart",
                                                     "Simulation starte time for capturing cumulative BSM",
                                                     ns3::UintegerValue (0),
                                                     ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_txSafetyRange1 ("VRCtxSafetyRange1",
                                          "BSM range for PDR inclusion",
                                          ns3::DoubleValue (50.0),
                                          ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_txSafetyRange2 ("VRCtxSafetyRange2",
                                          "BSM range for PDR inclusion",
                                          ns3::DoubleValue (100.0),
                                          ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_txSafetyRange3 ("VRCtxSafetyRange3",
                                          "BSM range for PDR inclusion",
                                          ns3::DoubleValue (150.0),
                                          ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_txSafetyRange4 ("VRCtxSafetyRange4",
                                          "BSM range for PDR inclusion",
                                          ns3::DoubleValue (200.0),
                                          ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_txSafetyRange5 ("VRCtxSafetyRange5",
                                          "BSM range for PDR inclusion",
                                          ns3::DoubleValue (250.0),
                                          ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_txSafetyRange6 ("VRCtxSafetyRange6",
                                          "BSM range for PDR inclusion",
                                          ns3::DoubleValue (300.0),
                                          ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_txSafetyRange7 ("VRCtxSafetyRange7",
                                          "BSM range for PDR inclusion",
                                          ns3::DoubleValue (350.0),
                                          ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_txSafetyRange8 ("VRCtxSafetyRange8",
                                          "BSM range for PDR inclusion",
                                          ns3::DoubleValue (400.0),
                                          ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_txSafetyRange9 ("VRCtxSafetyRange9",
                                          "BSM range for PDR inclusion",
                                          ns3::DoubleValue (450.0),
                                          ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_txSafetyRange10 ("VRCtxSafetyRange10",
                                           "BSM range for PDR inclusion",
                                           ns3::DoubleValue (500.0),
                                           ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_txp ("VRCtxp",
                               "Transmission power dBm",
                               ns3::DoubleValue (7.5),
                               ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_totalTime ("VRCtotalTime",
                                     "Total simulation time (s)",
                                     ns3::DoubleValue (300.01),
                                     ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_waveInterval ("VRCwaveInterval",
                                        "Interval (s) between WAVE BSMs",
                                        ns3::DoubleValue (0.1),
                                        ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_gpsAccuracyNs ("VRCgpsAccuracyNs",
                                         "GPS sync accuracy (ns)",
                                         ns3::DoubleValue (40),
                                         ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_txMaxDelayMs ("VRCtxMaxDelayMs",
                                        "Tx May Delay (ms)",
                                        ns3::DoubleValue (10),
                                        ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_CSVfileName ("VRCCSVfileName",
                                       "CSV filename (for time series data)",
                                       ns3::StringValue ("vanet-routing.output.csv"),
                                       ns3::MakeStringChecker ());
static ns3::GlobalValue g_CSVfileName2 ("VRCCSVfileName2",
                                        "CSV filename 2 (for overall simulation scenario results)",
                                        ns3::StringValue ("vanet-routing.output2.csv"),
                                        ns3::MakeStringChecker ());
static ns3::GlobalValue g_phyMode ("VRCphyMode",
                                   "PHY mode (802.11p)",
                                   ns3::StringValue ("OfdmRate6MbpsBW10MHz"),
                                   ns3::MakeStringChecker ());
static ns3::GlobalValue g_traceFile ("VRCtraceFile",
                                     "Mobility trace filename",
                                     ns3::StringValue ("./src/wave/examples/low99-ct-unterstrass-1day.filt.7.adj.mob"),
                                     ns3::MakeStringChecker ());
static ns3::GlobalValue g_logFile ("VRClogFile",
                                   "Log filename",
                                   ns3::StringValue ("low99-ct-unterstrass-1day.filt.7.adj.log"),
                                   ns3::MakeStringChecker ());
static ns3::GlobalValue g_rate ("VRCrate",
                                "Data rate",
                                ns3::StringValue ("2048bps"),
                                ns3::MakeStringChecker ());
static ns3::GlobalValue g_phyModeB ("VRCphyModeB",
                                    "PHY mode (802.11a)",
                                    ns3::StringValue ("DsssRate11Mbps"),
                                    ns3::MakeStringChecker ());
static ns3::GlobalValue g_trName ("VRCtrName",
                                  "Trace name",
                                  ns3::StringValue ("vanet-routing-compare"),
                                  ns3::MakeStringChecker ());

void
VanetRoutingExperiment::ParseCommandLineArguments (int argc, char **argv)
{
  CommandSetup (argc, argv);
  SetupScenario ();

  // user may specify up to 10 different tx distances
  // to be used for calculating different values of Packet
  // Delivery Ratio (PDR). Used to see the effects of
  // fading over distance
  m_txSafetyRanges.resize (10, 0);
  m_txSafetyRanges[0] = m_txSafetyRange1;
  m_txSafetyRanges[1] = m_txSafetyRange2;
  m_txSafetyRanges[2] = m_txSafetyRange3;
  m_txSafetyRanges[3] = m_txSafetyRange4;
  m_txSafetyRanges[4] = m_txSafetyRange5;
  m_txSafetyRanges[5] = m_txSafetyRange6;
  m_txSafetyRanges[6] = m_txSafetyRange7;
  m_txSafetyRanges[7] = m_txSafetyRange8;
  m_txSafetyRanges[8] = m_txSafetyRange9;
  m_txSafetyRanges[9] = m_txSafetyRange10;

  ConfigureDefaults ();

  // we are done with all configuration
  // save config-store, if requested
  SetGlobalsFromConfig ();
  ConfigStoreHelper configStoreHelper;
  configStoreHelper.SaveConfig (m_saveConfigFilename);

  m_waveBsmHelper.GetWaveBsmStats ()->SetLogging (m_log);
  m_routingHelper->SetLogging (m_log);
}

void
VanetRoutingExperiment::ConfigureNodes ()
{
  // Create vehicle OBU nodes + fixed RSU infrastructure nodes
  m_adhocTxNodes.Create (m_nNodes + m_nRsuNodes);
}

void
VanetRoutingExperiment::ConfigureChannels ()
{
  // set up channel and devices
  SetupAdhocDevices ();
}

void
VanetRoutingExperiment::ConfigureDevices ()
{
  // devices are set up in SetupAdhocDevices(),
  // called by ConfigureChannels()

  // every device will have PHY callback for tracing
  // which is used to determine the total amount of
  // data transmitted, and then used to calculate
  // the MAC/PHY overhead beyond the app-data
  Config::Connect ("/NodeList/*/DeviceList/*/Phy/State/Tx", MakeCallback (&WifiPhyStats::PhyTxTrace, m_wifiPhyStats));
  // TxDrop, RxDrop not working yet.  Not sure what I'm doing wrong.
  Config::Connect ("/NodeList/*/DeviceList/*/ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback (&WifiPhyStats::PhyTxDrop, m_wifiPhyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback (&WifiPhyStats::PhyRxDrop, m_wifiPhyStats));
}

void
VanetRoutingExperiment::ConfigureMobility ()
{
  SetupAdhocMobilityNodes ();
}

void
VanetRoutingExperiment::ConfigureApplications ()
{
  // Traffic mix consists of:
  // 1. routing data
  // 2. Broadcasting of Basic Safety Message (BSM)
  // 3. V2I Socket Application (Vehicle OBU <-> RSU Server <-> Central Server)
  SetupRoutingMessages ();
  SetupWaveMessages ();
  SetupV2iSockets ();

  // config trace to capture app-data (bytes) for
  // routing data, subtracted and used for
  // routing overhead
  std::ostringstream oss;
  oss.str ("");
  oss << "/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx";
  Config::Connect (oss.str (), MakeCallback (&RoutingHelper::OnOffTrace, m_routingHelper));
}

void
VanetRoutingExperiment::ConfigureTracing ()
{
  WriteCsvHeader ();
  SetupLogFile ();
  SetupLogging ();

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (m_trName + ".mob"));
}

void
VanetRoutingExperiment::RunSimulation ()
{
  Run ();
}

void
VanetRoutingExperiment::ProcessOutputs ()
{
  // calculate and output final results
  double bsm_pdr1 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (1);
  double bsm_pdr2 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (2);
  double bsm_pdr3 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (3);
  double bsm_pdr4 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (4);
  double bsm_pdr5 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (5);
  double bsm_pdr6 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (6);
  double bsm_pdr7 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (7);
  double bsm_pdr8 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (8);
  double bsm_pdr9 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (9);
  double bsm_pdr10 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (10);

  double averageRoutingGoodputKbps = 0.0;
  uint32_t totalBytesTotal = m_routingHelper->GetRoutingStats ().GetCumulativeRxBytes ();
  averageRoutingGoodputKbps = (((double) totalBytesTotal * 8.0) / m_TotalSimTime) / 1000.0;

  // calculate MAC/PHY overhead (mac-phy-oh)
  // total WAVE BSM bytes sent
  uint32_t cumulativeWaveBsmBytes = m_waveBsmHelper.GetWaveBsmStats ()->GetTxByteCount ();
  uint32_t cumulativeRoutingBytes = m_routingHelper->GetRoutingStats ().GetCumulativeTxBytes ();
  uint32_t totalAppBytes = cumulativeWaveBsmBytes + cumulativeRoutingBytes;
  uint32_t totalPhyBytes = m_wifiPhyStats->GetTxBytes ();
  // mac-phy-oh = (total-phy-bytes - total-app-bytes) / total-phy-bytes
  double mac_phy_oh = 0.0;
  if (totalPhyBytes > 0)
    {
      mac_phy_oh = (double) (totalPhyBytes - totalAppBytes) / (double) totalPhyBytes;
    }

  if (m_log != 0)
    {
      NS_LOG_UNCOND ("BSM_PDR1=" << bsm_pdr1 << " BSM_PDR2=" << bsm_pdr2 << " BSM_PDR3=" << bsm_pdr3 << " BSM_PDR4=" << bsm_pdr4 << " BSM_PDR5=" << bsm_pdr5 << " BSM_PDR6=" << bsm_pdr6 << " BSM_PDR7=" << bsm_pdr7 << " BSM_PDR8=" << bsm_pdr8 << " BSM_PDR9=" << bsm_pdr9 << " BSM_PDR10=" << bsm_pdr10 << " Goodput=" << averageRoutingGoodputKbps << "Kbps MAC/PHY-oh=" << mac_phy_oh);
      double v2iPdr = (m_v2iTxPkts > 0) ? (100.0 * m_v2iRxPkts / m_v2iTxPkts) : 0.0;
      double v2iMeanDelay = (m_v2iRxPkts > 0) ? (m_v2iDelaySumMs / m_v2iRxPkts) : 0.0;
      NS_LOG_UNCOND ("=== V2I Socket Summary: Sent=" << m_v2iTxPkts
                     << " Received=" << m_v2iRxPkts
                     << " V2I_PDR=" << v2iPdr << "%"
                     << " MeanDelay=" << v2iMeanDelay << "ms"
                     << " I2V_Alerts=" << m_i2vAlertsSent << " ===");
    }

  std::ofstream out (m_CSVfileName2.c_str (), std::ios::app);

  out << bsm_pdr1 << ","
      << bsm_pdr2 << ","
      << bsm_pdr3 << ","
      << bsm_pdr4 << ","
      << bsm_pdr5 << ","
      << bsm_pdr6 << ","
      << bsm_pdr7 << ","
      << bsm_pdr8 << ","
      << bsm_pdr9 << ","
      << bsm_pdr10 << ","
      << averageRoutingGoodputKbps << ","
      << mac_phy_oh << ""
      << std::endl;

  out.close ();

  if (m_v2iCsvOs.is_open ())
    {
      m_v2iCsvOs.close ();
    }
#ifdef __linux__
  if (m_extUdpSock >= 0)
    {
      close (m_extUdpSock);
      m_extUdpSock = -1;
    }
#endif

  m_os.close (); // close log file
}

void
VanetRoutingExperiment::Run ()
{
  NS_LOG_INFO ("Run Simulation.");
  AnimationInterface anim("v2i_netanim.xml");

  // Highlight vehicle OBU nodes vs fixed RSU stations in NetAnim
  for (uint32_t i = 0; i < m_nNodes; ++i)
    {
      std::ostringstream desc;
      desc << "OBU-" << i;
      anim.UpdateNodeDescription (m_adhocTxNodes.Get (i), desc.str ());
      anim.UpdateNodeColor (m_adhocTxNodes.Get (i), 220, 40, 40);
      anim.UpdateNodeSize (m_adhocTxNodes.Get (i)->GetId (), 18.0, 18.0);
    }
  for (uint32_t r = 0; r < m_nRsuNodes; ++r)
    {
      uint32_t nodeIdx = m_nNodes + r;
      std::ostringstream desc;
      desc << "RSU-0" << (r + 1);
      anim.UpdateNodeDescription (m_adhocTxNodes.Get (nodeIdx), desc.str ());
      anim.UpdateNodeColor (m_adhocTxNodes.Get (nodeIdx), 0, 180, 80);
      anim.UpdateNodeSize (m_adhocTxNodes.Get (nodeIdx)->GetId (), 32.0, 32.0);
    }

  //Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  
  
  CheckThroughput ();

  Simulator::Stop (Seconds (m_TotalSimTime));
  Simulator::Run ();
  ///////////////////////// Network Perfomance Calculation ///////////////////////
uint32_t SentPackets = 0;
uint32_t ReceivedPackets = 0;
uint32_t LostPackets = 0;
int j=0;
float AvgThroughput = 0;
Time Jitter;
Time Delay;

Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

NS_LOG_UNCOND("----Flow ID:" <<iter->first);
NS_LOG_UNCOND("Src Addr " <<t.sourceAddress << " Dst Addr "<< t.destinationAddress << " DstPort " << t.destinationPort);
NS_LOG_UNCOND("Sent Packets=" <<iter->second.txPackets);
NS_LOG_UNCOND("Received Packets =" <<iter->second.rxPackets);
NS_LOG_UNCOND("Lost Packets =" <<iter->second.txPackets-iter->second.rxPackets);
if (iter->second.txPackets > 0)
  {
    NS_LOG_UNCOND("Packet delivery ratio =" << (iter->second.rxPackets * 100.0 / iter->second.txPackets) << "%");
    NS_LOG_UNCOND("Packet loss ratio =" << ((iter->second.txPackets - iter->second.rxPackets) * 100.0 / iter->second.txPackets) << "%");
  }
NS_LOG_UNCOND("Delay =" <<iter->second.delaySum);
NS_LOG_UNCOND("Jitter =" <<iter->second.jitterSum);
double durationSec = iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds();
double flowTput = (durationSec > 0.0) ? (iter->second.rxBytes * 8.0 / durationSec / 1024.0) : 0.0;
NS_LOG_UNCOND("Throughput =" << flowTput << "Kbps");

SentPackets = SentPackets +(iter->second.txPackets);
ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
LostPackets = LostPackets + (iter->second.txPackets-iter->second.rxPackets);
AvgThroughput = AvgThroughput + flowTput;
Delay = Delay + (iter->second.delaySum);
Jitter = Jitter + (iter->second.jitterSum);

j = j + 1;

}

if (j > 0)
  {
    AvgThroughput = AvgThroughput / j;
  }
NS_LOG_UNCOND("--------Total Results of the simulation----------"<<std::endl);
NS_LOG_UNCOND("Total sent packets  =" << SentPackets);
NS_LOG_UNCOND("Total Received Packets =" << ReceivedPackets);
NS_LOG_UNCOND("Total Lost Packets =" << LostPackets);
if (SentPackets > 0)
  {
    NS_LOG_UNCOND("Packet Loss ratio =" << ((LostPackets * 100.0) / SentPackets)<< "%");
    NS_LOG_UNCOND("Packet delivery ratio =" << ((ReceivedPackets * 100.0) / SentPackets)<< "%");
  }
NS_LOG_UNCOND("Average Throughput =" << AvgThroughput<< "Kbps");
NS_LOG_UNCOND("End to End Delay =" << Delay);
NS_LOG_UNCOND("End to End Jitter delay =" << Jitter);
NS_LOG_UNCOND("Total Flow id " << j);
monitor->SerializeToXmlFile("v2i_flowmon_stats.xml", true, true);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Simulator::Destroy ();
}

// Prints actual position and velocity when a course change event occurs
void
VanetRoutingExperiment::
CourseChange (std::ostream *os, std::string context, Ptr<const MobilityModel> mobility)
{
  Vector pos = mobility->GetPosition (); // Get position
  Vector vel = mobility->GetVelocity (); // Get velocity

  pos.z = 1.5;

  int nodeId = mobility->GetObject<Node> ()->GetId ();
  double t = (Simulator::Now ()).GetSeconds ();
  if (t >= 1.0)
    {
      WaveBsmHelper::GetNodesMoving ()[nodeId] = 1;
    }

  //NS_LOG_UNCOND ("Changing pos for node=" << nodeId << " at " << Simulator::Now () );

  // Prints position and velocities
  *os << Simulator::Now () << " POS: x=" << pos.x << ", y=" << pos.y
      << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
      << ", z=" << vel.z << std::endl;
}

void
VanetRoutingExperiment::CheckThroughput ()
{
  uint32_t bytesTotal = m_routingHelper->GetRoutingStats ().GetRxBytes ();
  uint32_t packetsReceived = m_routingHelper->GetRoutingStats ().GetRxPkts ();
  double kbps = (bytesTotal * 8.0) / 1000;
  double wavePDR = 0.0;
  int wavePktsSent = m_waveBsmHelper.GetWaveBsmStats ()->GetTxPktCount ();
  int wavePktsReceived = m_waveBsmHelper.GetWaveBsmStats ()->GetRxPktCount ();
  if (wavePktsSent > 0)
    {
      int wavePktsReceived = m_waveBsmHelper.GetWaveBsmStats ()->GetRxPktCount ();
      wavePDR = (double) wavePktsReceived / (double) wavePktsSent;
    }

  int waveExpectedRxPktCount = m_waveBsmHelper.GetWaveBsmStats ()->GetExpectedRxPktCount (1);
  int waveRxPktInRangeCount = m_waveBsmHelper.GetWaveBsmStats ()->GetRxPktInRangeCount (1);
  double wavePDR1_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (1);
  double wavePDR2_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (2);
  double wavePDR3_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (3);
  double wavePDR4_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (4);
  double wavePDR5_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (5);
  double wavePDR6_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (6);
  double wavePDR7_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (7);
  double wavePDR8_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (8);
  double wavePDR9_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (9);
  double wavePDR10_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (10);

  // calculate MAC/PHY overhead (mac-phy-oh)
  // total WAVE BSM bytes sent
  uint32_t cumulativeWaveBsmBytes = m_waveBsmHelper.GetWaveBsmStats ()->GetTxByteCount ();
  uint32_t cumulativeRoutingBytes = m_routingHelper->GetRoutingStats ().GetCumulativeTxBytes ();
  uint32_t totalAppBytes = cumulativeWaveBsmBytes + cumulativeRoutingBytes;
  uint32_t totalPhyBytes = m_wifiPhyStats->GetTxBytes ();
  // mac-phy-oh = (total-phy-bytes - total-app-bytes) / total-phy-bytes
  double mac_phy_oh = 0.0;
  if (totalPhyBytes > 0)
    {
      mac_phy_oh = (double) (totalPhyBytes - totalAppBytes) / (double) totalPhyBytes;
    }

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  if (m_log != 0 )
    {
      NS_LOG_UNCOND ("At t=" << (Simulator::Now ()).GetSeconds () << "s BSM_PDR1=" << wavePDR1_2 << " BSM_PDR1=" << wavePDR2_2 << " BSM_PDR3=" << wavePDR3_2 << " BSM_PDR4=" << wavePDR4_2 << " BSM_PDR5=" << wavePDR5_2 << " BSM_PDR6=" << wavePDR6_2 << " BSM_PDR7=" << wavePDR7_2 << " BSM_PDR8=" << wavePDR8_2 << " BSM_PDR9=" << wavePDR9_2 << " BSM_PDR10=" << wavePDR10_2 << " Goodput=" << kbps << "Kbps" /*<< " MAC/PHY-OH=" << mac_phy_oh*/);
    }

  out << (Simulator::Now ()).GetSeconds () << ","
      << kbps << ","
      << packetsReceived << ","
      << m_nSinks << ","
      << m_protocolName << ","
      << m_txp << ","
      << wavePktsSent << ","
      << wavePktsReceived << ","
      << wavePDR << ","
      << waveExpectedRxPktCount << ","
      << waveRxPktInRangeCount << ","
      << wavePDR1_2 << ","
      << wavePDR2_2 << ","
      << wavePDR3_2 << ","
      << wavePDR4_2 << ","
      << wavePDR5_2 << ","
      << wavePDR6_2 << ","
      << wavePDR7_2 << ","
      << wavePDR8_2 << ","
      << wavePDR9_2 << ","
      << wavePDR10_2 << ","
      << mac_phy_oh << ""
      << std::endl;

  out.close ();

  m_routingHelper->GetRoutingStats ().SetRxBytes (0);
  m_routingHelper->GetRoutingStats ().SetRxPkts (0);
  m_waveBsmHelper.GetWaveBsmStats ()->SetRxPktCount (0);
  m_waveBsmHelper.GetWaveBsmStats ()->SetTxPktCount (0);
  for (int index = 1; index <= 10; index++)
    {
      m_waveBsmHelper.GetWaveBsmStats ()->SetExpectedRxPktCount (index, 0);
      m_waveBsmHelper.GetWaveBsmStats ()->SetRxPktInRangeCount (index, 0);
    }

  double currentTime = (Simulator::Now ()).GetSeconds ();
  if (currentTime <= (double) m_cumulativeBsmCaptureStart)
    {
      for (int index = 1; index <= 10; index++)
        {
          m_waveBsmHelper.GetWaveBsmStats ()->ResetTotalRxPktCounts (index);
        }
    }

  Simulator::Schedule (Seconds (1.0), &VanetRoutingExperiment::CheckThroughput, this);
}

void
VanetRoutingExperiment::SetConfigFromGlobals ()
{
  // get settings saved from config-store
  UintegerValue uintegerValue;
  DoubleValue doubleValue;
  StringValue stringValue;

  // This may not be the best way to manage program configuration
  // (directing them through global values), but management
  // through the config-store here is copied from
  // src/lte/examples/lena-dual-stripe.cc

  GlobalValue::GetValueByName ("VRCport", uintegerValue);
  m_port = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCnSinks", uintegerValue);
  m_nSinks = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCtraceMobility", uintegerValue);
  m_traceMobility = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCprotocol", uintegerValue);
  m_protocol = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRClossModel", uintegerValue);
  m_lossModel = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCfading", uintegerValue);
  m_fading = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRC80211mode", uintegerValue);
  m_80211mode = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCmobility", uintegerValue);
  m_mobility = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCnNodes", uintegerValue);
  m_nNodes = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCnodeSpeed", uintegerValue);
  m_nodeSpeed = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCnodePause", uintegerValue);
  m_nodePause = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCwavePacketSize", uintegerValue);
  m_wavePacketSize = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCverbose", uintegerValue);
  m_verbose = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCscenario", uintegerValue);
  m_scenario = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCroutingTables", uintegerValue);
  m_routingTables = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCasciiTrace", uintegerValue);
  m_asciiTrace = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCpcap", uintegerValue);
  m_pcap = uintegerValue.Get ();
  GlobalValue::GetValueByName ("VRCcumulativeBsmCaptureStart", uintegerValue);
  m_cumulativeBsmCaptureStart = uintegerValue.Get ();

  GlobalValue::GetValueByName ("VRCtxSafetyRange1", doubleValue);
  m_txSafetyRange1 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxSafetyRange2", doubleValue);
  m_txSafetyRange2 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxSafetyRange3", doubleValue);
  m_txSafetyRange3 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxSafetyRange4", doubleValue);
  m_txSafetyRange4 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxSafetyRange5", doubleValue);
  m_txSafetyRange5 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxSafetyRange6", doubleValue);
  m_txSafetyRange6 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxSafetyRange7", doubleValue);
  m_txSafetyRange7 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxSafetyRange8", doubleValue);
  m_txSafetyRange8 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxSafetyRange9", doubleValue);
  m_txSafetyRange9 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxSafetyRange10", doubleValue);
  m_txSafetyRange10 = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxp", doubleValue);
  m_txp = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtotalTime", doubleValue);
  m_TotalSimTime = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCwaveInterval", doubleValue);
  m_waveInterval = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCgpsAccuracyNs", doubleValue);
  m_gpsAccuracyNs = doubleValue.Get ();
  GlobalValue::GetValueByName ("VRCtxMaxDelayMs", doubleValue);
  m_txMaxDelayMs = doubleValue.Get ();

  GlobalValue::GetValueByName ("VRCCSVfileName", stringValue);
  m_CSVfileName = stringValue.Get ();
  GlobalValue::GetValueByName ("VRCCSVfileName2", stringValue);
  m_CSVfileName2 = stringValue.Get ();
  GlobalValue::GetValueByName ("VRCphyMode", stringValue);
  m_phyMode = stringValue.Get ();
  GlobalValue::GetValueByName ("VRCtraceFile", stringValue);
  m_traceFile = stringValue.Get ();
  GlobalValue::GetValueByName ("VRClogFile", stringValue);
  m_logFile = stringValue.Get ();
  GlobalValue::GetValueByName ("VRCrate", stringValue);
  m_rate = stringValue.Get ();
  GlobalValue::GetValueByName ("VRCphyModeB", stringValue);
  m_phyModeB = stringValue.Get ();
  GlobalValue::GetValueByName ("VRCtrName", stringValue);
  m_trName = stringValue.Get ();
}

void
VanetRoutingExperiment::SetGlobalsFromConfig ()
{
  // get settings saved from config-store
  UintegerValue uintegerValue;
  DoubleValue doubleValue;
  StringValue stringValue;

  g_port.SetValue (UintegerValue (m_port));
  g_nSinks.SetValue (UintegerValue (m_nSinks));
  g_traceMobility.SetValue (UintegerValue (m_traceMobility));
  g_protocol.SetValue (UintegerValue (m_protocol));
  g_lossModel.SetValue (UintegerValue (m_lossModel));
  g_fading.SetValue (UintegerValue (m_fading));
  g_80211mode.SetValue (UintegerValue (m_80211mode));
  g_mobility.SetValue (UintegerValue (m_mobility));
  g_nNodes.SetValue (UintegerValue (m_nNodes));
  g_nodeSpeed.SetValue (UintegerValue (m_nodeSpeed));
  g_nodePause.SetValue (UintegerValue (m_nodePause));
  g_wavePacketSize.SetValue (UintegerValue (m_wavePacketSize));
  g_verbose.SetValue (UintegerValue (m_verbose));
  g_scenario.SetValue (UintegerValue (m_scenario));
  g_routingTables.SetValue (UintegerValue (m_routingTables));
  g_asciiTrace.SetValue (UintegerValue (m_asciiTrace));
  g_pcap.SetValue (UintegerValue (m_pcap));
  g_cumulativeBsmCaptureStart.SetValue (UintegerValue (m_cumulativeBsmCaptureStart));

  g_txSafetyRange1.SetValue (DoubleValue (m_txSafetyRange1));
  g_txSafetyRange2.SetValue (DoubleValue (m_txSafetyRange2));
  g_txSafetyRange3.SetValue (DoubleValue (m_txSafetyRange3));
  g_txSafetyRange4.SetValue (DoubleValue (m_txSafetyRange4));
  g_txSafetyRange5.SetValue (DoubleValue (m_txSafetyRange5));
  g_txSafetyRange6.SetValue (DoubleValue (m_txSafetyRange6));
  g_txSafetyRange7.SetValue (DoubleValue (m_txSafetyRange7));
  g_txSafetyRange8.SetValue (DoubleValue (m_txSafetyRange8));
  g_txSafetyRange9.SetValue (DoubleValue (m_txSafetyRange9));
  g_txSafetyRange10.SetValue (DoubleValue (m_txSafetyRange10));
  g_txp.SetValue (DoubleValue (m_txp));
  g_totalTime.SetValue (DoubleValue (m_TotalSimTime));
  g_waveInterval.SetValue (DoubleValue (m_waveInterval));
  g_gpsAccuracyNs.SetValue (DoubleValue (m_gpsAccuracyNs));
  g_txMaxDelayMs.SetValue (DoubleValue (m_txMaxDelayMs));

  g_CSVfileName.SetValue (StringValue (m_CSVfileName));
  g_CSVfileName2.SetValue (StringValue (m_CSVfileName2));
  g_phyMode.SetValue (StringValue (m_phyMode));
  g_traceFile.SetValue (StringValue (m_traceFile));
  g_logFile.SetValue (StringValue (m_logFile));
  g_rate.SetValue (StringValue (m_rate));
  g_phyModeB.SetValue (StringValue (m_phyModeB));
  g_trName.SetValue (StringValue (m_trName));
  GlobalValue::GetValueByName ("VRCtrName", stringValue);
  m_trName = stringValue.Get ();
}

void
VanetRoutingExperiment::CommandSetup (int argc, char **argv)
{
  CommandLine cmd;
  double txDist1 = 50.0;
  double txDist2 = 100.0;
  double txDist3 = 150.0;
  double txDist4 = 200.0;
  double txDist5 = 250.0;
  double txDist6 = 300.0;
  double txDist7 = 350.0;
  double txDist8 = 350.0;
  double txDist9 = 350.0;
  double txDist10 = 350.0;

  // allow command line overrides
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
  cmd.AddValue ("CSVfileName2", "The name of the CSV output file name2", m_CSVfileName2);
  cmd.AddValue ("totaltime", "Simulation end time", m_TotalSimTime);
  cmd.AddValue ("nodes", "Number of nodes (i.e. vehicles)", m_nNodes);
  cmd.AddValue ("sinks", "Number of routing sinks", m_nSinks);
  cmd.AddValue ("txp", "Transmit power (dB), e.g. txp=7.5", m_txp);
  cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
  cmd.AddValue ("protocol", "1=OLSR;2=AODV;3=DSDV;4=DSR", m_protocol);
  cmd.AddValue ("lossModel", "1=Friis;2=ItuR1411Los;3=TwoRayGround;4=LogDistance", m_lossModel);
  cmd.AddValue ("fading", "0=None;1=Nakagami;(buildings=1 overrides)", m_fading);
  cmd.AddValue ("phyMode", "Wifi Phy mode", m_phyMode);
  cmd.AddValue ("80211Mode", "1=802.11p; 2=802.11b; 3=WAVE-PHY", m_80211mode);
  cmd.AddValue ("traceFile", "Ns2 movement trace file", m_traceFile);
  cmd.AddValue ("logFile", "Log file", m_logFile);
  cmd.AddValue ("mobility", "1=trace;2=RWP", m_mobility);
  cmd.AddValue ("rate", "Rate", m_rate);
  cmd.AddValue ("phyModeB", "Phy mode 802.11b", m_phyModeB);
  cmd.AddValue ("speed", "Node speed (m/s)", m_nodeSpeed);
  cmd.AddValue ("pause", "Node pause (s)", m_nodePause);
  cmd.AddValue ("verbose", "0=quiet;1=verbose", m_verbose);
  cmd.AddValue ("bsm", "(WAVE) BSM size (bytes)", m_wavePacketSize);
  cmd.AddValue ("interval", "(WAVE) BSM interval (s)", m_waveInterval);
  cmd.AddValue ("scenario", "1=synthetic, 2=playback-trace", m_scenario);
  // User is allowed to have up to 10 different PDRs (Packet
  // Delivery Ratios) calculate, and so can specify up to
  // 10 different tx distances.
  cmd.AddValue ("txdist1", "Expected BSM tx range, m", txDist1);
  cmd.AddValue ("txdist2", "Expected BSM tx range, m", txDist2);
  cmd.AddValue ("txdist3", "Expected BSM tx range, m", txDist3);
  cmd.AddValue ("txdist4", "Expected BSM tx range, m", txDist4);
  cmd.AddValue ("txdist5", "Expected BSM tx range, m", txDist5);
  cmd.AddValue ("txdist6", "Expected BSM tx range, m", txDist6);
  cmd.AddValue ("txdist7", "Expected BSM tx range, m", txDist7);
  cmd.AddValue ("txdist8", "Expected BSM tx range, m", txDist8);
  cmd.AddValue ("txdist9", "Expected BSM tx range, m", txDist9);
  cmd.AddValue ("txdist10", "Expected BSM tx range, m", txDist10);
  cmd.AddValue ("gpsaccuracy", "GPS time accuracy, in ns", m_gpsAccuracyNs);
  cmd.AddValue ("txmaxdelay", "Tx max delay, in ms", m_txMaxDelayMs);
  cmd.AddValue ("routingTables", "Dump routing tables at t=5 seconds", m_routingTables);
  cmd.AddValue ("asciiTrace", "Dump ASCII Trace data", m_asciiTrace);
  cmd.AddValue ("pcap", "Create PCAP files for all nodes", m_pcap);
  cmd.AddValue ("loadconfig", "Config-store filename to load", m_loadConfigFilename);
  cmd.AddValue ("saveconfig", "Config-store filename to save", m_saveConfigFilename);
  cmd.AddValue ("exp", "Experiment", m_exp);
  cmd.AddValue ("BsmCaptureStart", "Start time to begin capturing pkts for cumulative Bsm", m_cumulativeBsmCaptureStart);
  cmd.AddValue ("rsuNodes", "Number of fixed RSU infrastructure nodes", m_nRsuNodes);
  cmd.AddValue ("v2iPort", "UDP port for V2I RSU socket communication", m_v2iPort);
  cmd.AddValue ("liveUdpPort", "External UDP port for Central Server dashboard", m_liveUdpPort);
  cmd.Parse (argc, argv);

  m_txSafetyRange1 = txDist1;
  m_txSafetyRange2 = txDist2;
  m_txSafetyRange3 = txDist3;
  m_txSafetyRange4 = txDist4;
  m_txSafetyRange5 = txDist5;
  m_txSafetyRange6 = txDist6;
  m_txSafetyRange7 = txDist7;
  m_txSafetyRange8 = txDist8;
  m_txSafetyRange9 = txDist9;
  m_txSafetyRange10 = txDist10;
  // load configuration info from config-store
  ConfigStoreHelper configStoreHelper;
  configStoreHelper.LoadConfig (m_loadConfigFilename);
  // transfer config-store values to config parameters
  SetConfigFromGlobals ();

  // parse again so you can override input file default values via command line
  cmd.Parse (argc, argv);

  m_txSafetyRange1 = txDist1;
  m_txSafetyRange2 = txDist2;
  m_txSafetyRange3 = txDist3;
  m_txSafetyRange4 = txDist4;
  m_txSafetyRange5 = txDist5;
  m_txSafetyRange6 = txDist6;
  m_txSafetyRange7 = txDist7;
  m_txSafetyRange8 = txDist8;
  m_txSafetyRange9 = txDist9;
  m_txSafetyRange10 = txDist10;
}

void
VanetRoutingExperiment::SetupLogFile ()
{
  // open log file for output
  m_os.open (m_logFile.c_str ());
}

void VanetRoutingExperiment::SetupLogging ()
{

  // Enable logging from the ns2 helper
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  Packet::EnablePrinting ();
}

void
VanetRoutingExperiment::ConfigureDefaults ()
{
  Config::SetDefault ("ns3::OnOffApplication::PacketSize",StringValue ("64"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (m_rate));

  //Set Non-unicastMode rate to unicast mode
  if (m_80211mode == 2)
    {
      Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (m_phyModeB));
    }
  else
    {
      Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (m_phyMode));
    }
}

void
VanetRoutingExperiment::SetupAdhocMobilityNodes ()
{
  uint32_t totalNodes = m_nNodes + m_nRsuNodes;
  if (m_mobility == 1)
    {
      // Create Ns2MobilityHelper with the specified trace log file as parameter
      Ns2MobilityHelper ns2 = Ns2MobilityHelper (m_traceFile);
      ns2.Install (); // configure movements for vehicle nodes, while reading trace file
      // Install ConstantPositionMobilityModel on fixed RSU infrastructure nodes
      NodeContainer rsuNodes;
      for (uint32_t r = 0; r < m_nRsuNodes; ++r)
        {
          rsuNodes.Add (m_adhocTxNodes.Get (m_nNodes + r));
        }
      if (m_nRsuNodes > 0)
        {
          MobilityHelper rsuMobility;
          Ptr<ListPositionAllocator> rsuPosAlloc = CreateObject<ListPositionAllocator> ();
          for (uint32_t r = 0; r < m_nRsuNodes; ++r)
            {
              Vector pos = (r < m_rsuPositions.size ()) ? m_rsuPositions[r] : Vector (750.0 + 300.0 * r, 450.0, 5.0);
              rsuPosAlloc->Add (pos);
            }
          rsuMobility.SetPositionAllocator (rsuPosAlloc);
          rsuMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
          rsuMobility.Install (rsuNodes);
        }
      WaveBsmHelper::GetNodesMoving ().resize (totalNodes, 0);
      for (uint32_t r = 0; r < m_nRsuNodes; ++r)
        {
          WaveBsmHelper::GetNodesMoving ()[m_nNodes + r] = 1;
        }
    }
  else if (m_mobility == 2)
    {
      MobilityHelper mobilityAdhoc;

      ObjectFactory pos;
      pos.SetTypeId ("ns3::RandomBoxPositionAllocator");
      pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
      pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
      // we need antenna height uniform [1.0 .. 2.0] for loss model
      pos.Set ("Z", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=2.0]"));

      Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
      m_streamIndex += taPositionAlloc->AssignStreams (m_streamIndex);

      std::stringstream ssSpeed;
      ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_nodeSpeed << "]";
      std::stringstream ssPause;
      ssPause << "ns3::ConstantRandomVariable[Constant=" << m_nodePause << "]";
      mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                      "Speed", StringValue (ssSpeed.str ()),
                                      "Pause", StringValue (ssPause.str ()),
                                      "PositionAllocator", PointerValue (taPositionAlloc));
      mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
      mobilityAdhoc.Install (m_adhocTxNodes);
      m_streamIndex += mobilityAdhoc.AssignStreams (m_adhocTxNodes, m_streamIndex);

      // initially assume all nodes are moving
      WaveBsmHelper::GetNodesMoving ().resize (totalNodes, 1);
    }

  // Configure callback for logging
  Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
                   MakeBoundCallback (&VanetRoutingExperiment::CourseChange, &m_os));
}

void
VanetRoutingExperiment::SetupAdhocDevices ()
{
  if (m_lossModel == 1)
    {
      m_lossModelName = "ns3::FriisPropagationLossModel";
    }
  else if (m_lossModel == 2)
    {
      m_lossModelName = "ns3::ItuR1411LosPropagationLossModel";
    }
  else if (m_lossModel == 3)
    {
      m_lossModelName = "ns3::TwoRayGroundPropagationLossModel";
    }
  else if (m_lossModel == 4)
    {
      m_lossModelName = "ns3::LogDistancePropagationLossModel";
    }
  else
    {
      // Unsupported propagation loss model.
      // Treating as ERROR
      NS_LOG_ERROR ("Invalid propagation loss model specified.  Values must be [1-4], where 1=Friis;2=ItuR1411Los;3=TwoRayGround;4=LogDistance");
    }

  // frequency
  double freq = 0.0;
  if ((m_80211mode == 1)
      || (m_80211mode == 3))
    {
      // 802.11p 5.9 GHz
      freq = 5.9e9;
    }
  else
    {
      // 802.11b 2.4 GHz
      freq = 2.4e9;
    }

  // Setup propagation models
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  if (m_lossModel == 3)
    {
      // two-ray requires antenna height (else defaults to Friss)
      wifiChannel.AddPropagationLoss (m_lossModelName, "Frequency", DoubleValue (freq), "HeightAboveZ", DoubleValue (1.5));
    }
  else
    {
      wifiChannel.AddPropagationLoss (m_lossModelName, "Frequency", DoubleValue (freq));
    }

  // Propagation loss models are additive.
  if (m_fading != 0)
    {
      // if no obstacle model, then use Nakagami fading if requested
      wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
    }

  // the channel
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();

  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (channel);
  // ns-3 supports generate a pcap trace
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);

  YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
  wavePhy.SetChannel (channel);
  wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);

  // Setup WAVE PHY and MAC
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  WaveHelper waveHelper = WaveHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  if (m_verbose)
    {
      wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
      // likewise, turn on WAVE PHY logging
      waveHelper.EnableLogComponents ();
    }

  WifiHelper wifi;

  // Setup 802.11b stuff
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (m_phyModeB),
                                "ControlMode",StringValue (m_phyModeB));

  // Setup 802.11p stuff
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (m_phyMode),
                                      "ControlMode",StringValue (m_phyMode));

  // Setup WAVE-PHY stuff
  waveHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (m_phyMode),
                                      "ControlMode",StringValue (m_phyMode));

  // Set Tx Power
  wifiPhy.Set ("TxPowerStart",DoubleValue (m_txp));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txp));
  wavePhy.Set ("TxPowerStart",DoubleValue (m_txp));
  wavePhy.Set ("TxPowerEnd", DoubleValue (m_txp));

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();

  // Setup net devices

  if (m_80211mode == 3)
    {
      m_adhocTxDevices = waveHelper.Install (wavePhy, waveMac, m_adhocTxNodes);
    }
  else if (m_80211mode == 1)
    {
      m_adhocTxDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, m_adhocTxNodes);
    }
  else
    {
      m_adhocTxDevices = wifi.Install (wifiPhy, wifiMac, m_adhocTxNodes);
    }

  if (m_asciiTrace != 0)
    {
      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream ( (m_trName + ".tr").c_str ());
      wifiPhy.EnableAsciiAll (osw);
      wavePhy.EnableAsciiAll (osw);
    }
  if (m_pcap != 0)
    {
      wifiPhy.EnablePcapAll ("vanet-routing-compare-pcap");
      wavePhy.EnablePcapAll ("vanet-routing-compare-pcap");
    }
}

void
VanetRoutingExperiment::SetupWaveMessages ()
{
  // WAVE PHY mode
  // 0=continuous channel; 1=channel-switching
  int chAccessMode = 0;
  if (m_80211mode == 3)
    {
      chAccessMode = 1;
    }

  m_waveBsmHelper.Install (m_adhocTxInterfaces,
                           Seconds (m_TotalSimTime),
                           m_wavePacketSize,
                           Seconds (m_waveInterval),
                           // GPS accuracy (i.e, clock drift), in number of ns
                           m_gpsAccuracyNs,
                           m_txSafetyRanges,
                           chAccessMode,
                           // tx max delay before transmit, in ms
                           MilliSeconds (m_txMaxDelayMs));

  // fix random number streams
  m_streamIndex += m_waveBsmHelper.AssignStreams (m_adhocTxNodes, m_streamIndex);
}

void
VanetRoutingExperiment::SetupRoutingMessages ()
{
  switch (m_protocol)
    {
    case 1: m_protocolName = "OLSR"; break;
    case 2: m_protocolName = "AODV"; break;
    case 3: m_protocolName = "DSDV"; break;
    case 4: m_protocolName = "DSR"; break;
    default: m_protocolName = "NONE"; break;
    }

  m_routingHelper->Install (m_adhocTxNodes,
                            m_adhocTxDevices,
                            m_adhocTxInterfaces,
                            m_TotalSimTime,
                            m_protocol,
                            m_nSinks,
                            m_routingTables);
}

void
VanetRoutingExperiment::SetupV2iSockets ()
{
  if (m_nRsuNodes == 0)
    {
      return;
    }

  m_v2iCsvOs.open ("v2i_socket_telemetry.csv");
  m_v2iCsvOs << "SimTime,Seq,VehicleNodeId,TargetRsuId,RsuIp,PosX,PosY,Speed,AlertStatus,DelayMs,PayloadSize" << std::endl;

#ifdef __linux__
  if (m_liveUdpPort > 0)
    {
      m_extUdpSock = socket (AF_INET, SOCK_DGRAM, 0);
      if (m_extUdpSock >= 0)
        {
          int flags = fcntl (m_extUdpSock, F_GETFL, 0);
          fcntl (m_extUdpSock, F_SETFL, flags | O_NONBLOCK);
        }
    }
#endif

  TypeId udpTid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  // 1. Bind UDP Socket Server on each fixed RSU node (port m_v2iPort = 9999)
  m_rsuSockets.clear ();
  for (uint32_t r = 0; r < m_nRsuNodes; ++r)
    {
      Ptr<Node> rsuNode = m_adhocTxNodes.Get (m_nNodes + r);
      Ptr<Socket> rsuSock = Socket::CreateSocket (rsuNode, udpTid);
      InetSocketAddress localAddr (Ipv4Address::GetAny (), m_v2iPort);
      rsuSock->Bind (localAddr);
      rsuSock->SetAllowBroadcast (true);
      rsuSock->SetRecvCallback (MakeCallback (&VanetRoutingExperiment::ReceiveV2iPacket, this));
      m_rsuSockets.push_back (rsuSock);
      NS_LOG_UNCOND ("[V2I-RSU] RSU-0" << (r + 1) << " (Node " << rsuNode->GetId ()
                     << ", IP " << m_adhocTxInterfaces.GetAddress (m_nNodes + r)
                     << ") listening on UDP port " << m_v2iPort);
    }

  // 2. Create UDP Client Socket on each mobile Vehicle OBU node (port m_v2iPort + 1 = 10000 for I2V warnings)
  m_obuSockets.clear ();
  for (uint32_t i = 0; i < m_nNodes; ++i)
    {
      Ptr<Node> vehNode = m_adhocTxNodes.Get (i);
      Ptr<Socket> obuSock = Socket::CreateSocket (vehNode, udpTid);
      InetSocketAddress obuLocal (Ipv4Address::GetAny (), m_v2iPort + 1);
      obuSock->Bind (obuLocal);
      obuSock->SetAllowBroadcast (true);
      obuSock->SetRecvCallback (MakeCallback (&VanetRoutingExperiment::ReceiveI2vWarning, this));
      m_obuSockets.push_back (obuSock);
    }

  // Schedule periodic V2I telemetry transmission starting at t = 1.0s
  Simulator::Schedule (Seconds (1.0), &VanetRoutingExperiment::SendV2iTelemetry, this);
}

void
VanetRoutingExperiment::SendV2iTelemetry ()
{
  if (m_nRsuNodes == 0)
    {
      return;
    }

  double nowSec = Simulator::Now ().GetSeconds ();

  for (uint32_t i = 0; i < m_nNodes; ++i)
    {
      Ptr<Node> vehNode = m_adhocTxNodes.Get (i);
      Ptr<MobilityModel> mob = vehNode->GetObject<MobilityModel> ();
      if (!mob)
        {
          continue;
        }
      Vector pos = mob->GetPosition ();
      Vector vel = mob->GetVelocity ();
      double speed = std::sqrt (vel.x * vel.x + vel.y * vel.y);

      // Select nearest RSU station by Euclidean distance
      uint32_t nearestRsu = 0;
      double minDist = 1e9;
      for (uint32_t r = 0; r < m_nRsuNodes; ++r)
        {
          Ptr<MobilityModel> rsuMob = m_adhocTxNodes.Get (m_nNodes + r)->GetObject<MobilityModel> ();
          Vector rPos = rsuMob->GetPosition ();
          double d = CalculateDistance (pos, rPos);
          if (d < minDist)
            {
              minDist = d;
              nearestRsu = r;
            }
        }

      std::string alertStatus = "NORMAL";
      if (speed > 13.5)
        {
          alertStatus = "OVERSPEED_ALERT";
        }
      else if (nowSec > 10.0 && speed < 0.5 && (pos.x > 400.0 && pos.x < 1800.0))
        {
          alertStatus = "SUDDEN_STOP_ALERT";
        }

      ++m_v2iSeq;
      std::ostringstream payload;
      payload << std::fixed << std::setprecision (3)
              << "V2I_DATA|" << m_v2iSeq
              << "|" << i
              << "|RSU-0" << (nearestRsu + 1)
              << "|" << pos.x
              << "|" << pos.y
              << "|" << speed
              << "|" << alertStatus
              << "|" << nowSec;

      std::string dataStr = payload.str ();
      Ptr<Packet> pkt = Create<Packet> (reinterpret_cast<const uint8_t *> (dataStr.c_str ()), dataStr.size ());

      Ipv4Address rsuIp = m_adhocTxInterfaces.GetAddress (m_nNodes + nearestRsu);
      int sent = m_obuSockets[i]->SendTo (pkt, 0, InetSocketAddress (rsuIp, m_v2iPort));
      if (sent >= 0)
        {
          ++m_v2iTxPkts;
        }
    }

  if (nowSec + 1.0 < m_TotalSimTime)
    {
      Simulator::Schedule (Seconds (1.0), &VanetRoutingExperiment::SendV2iTelemetry, this);
    }
}

void
VanetRoutingExperiment::ReceiveV2iPacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address srcAddr;
  while ((packet = socket->RecvFrom (srcAddr)))
    {
      uint32_t pktSize = packet->GetSize ();
      std::vector<uint8_t> buf (pktSize + 1, 0);
      packet->CopyData (buf.data (), pktSize);
      std::string msg (reinterpret_cast<const char *> (buf.data ()));

      double rxTime = Simulator::Now ().GetSeconds ();
      uint32_t rsuNodeId = socket->GetNode ()->GetId ();
      uint32_t rsuIdx = (rsuNodeId >= m_nNodes) ? (rsuNodeId - m_nNodes + 1) : 1;

      // Parse fields: V2I_DATA|seq|vehId|rsuName|x|y|speed|status|txTime
      std::stringstream ss (msg);
      std::string token;
      std::vector<std::string> fields;
      while (std::getline (ss, token, '|'))
        {
          fields.push_back (token);
        }

      double txTime = rxTime;
      uint32_t seq = 0;
      uint32_t vehId = 0;
      double posX = 0.0, posY = 0.0, speed = 0.0;
      std::string status = "NORMAL";

      if (fields.size () >= 9)
        {
          seq = std::stoul (fields[1]);
          vehId = std::stoul (fields[2]);
          posX = std::stod (fields[4]);
          posY = std::stod (fields[5]);
          speed = std::stod (fields[6]);
          status = fields[7];
          txTime = std::stod (fields[8]);
        }

      double delayMs = std::max (0.0, (rxTime - txTime) * 1000.0);
      ++m_v2iRxPkts;
      m_v2iDelaySumMs += delayMs;

      Ipv4Address rsuIp = m_adhocTxInterfaces.GetAddress (rsuNodeId);
      if (m_v2iCsvOs.is_open ())
        {
          m_v2iCsvOs << std::fixed << std::setprecision (4)
                     << rxTime << "," << seq << "," << vehId << ",RSU-0" << rsuIdx << ","
                     << rsuIp << "," << posX << "," << posY << "," << speed << ","
                     << status << "," << delayMs << "," << pktSize << std::endl;
        }

      // Forward live telemetry over POSIX UDP socket to Central Monitoring Server (port 5005)
      ForwardToExternalServer (msg + "|" + std::to_string (delayMs));

      // If vehicle reported hazard/overspeed, RSU sends I2V Warning back via UDP socket
      if (status != "NORMAL" && InetSocketAddress::IsMatchingType (srcAddr))
        {
          InetSocketAddress clientInet = InetSocketAddress::ConvertFrom (srcAddr);
          std::ostringstream warnMsg;
          warnMsg << "I2V_WARNING|RSU-0" << rsuIdx << "|VEH-" << vehId << "|" << status << "|SLOW_DOWN_HAZARD_AHEAD|" << rxTime;
          std::string warnStr = warnMsg.str ();
          Ptr<Packet> warnPkt = Create<Packet> (reinterpret_cast<const uint8_t *> (warnStr.c_str ()), warnStr.size ());
          socket->SendTo (warnPkt, 0, InetSocketAddress (clientInet.GetIpv4 (), m_v2iPort + 1));
          ++m_i2vAlertsSent;
        }
    }
}

void
VanetRoutingExperiment::ReceiveI2vWarning (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address srcAddr;
  while ((packet = socket->RecvFrom (srcAddr)))
    {
      uint32_t pktSize = packet->GetSize ();
      std::vector<uint8_t> buf (pktSize + 1, 0);
      packet->CopyData (buf.data (), pktSize);
      std::string msg (reinterpret_cast<const char *> (buf.data ()));
      if (m_verbose != 0)
        {
          NS_LOG_UNCOND ("[OBU-" << socket->GetNode ()->GetId () << "] Received I2V Socket Alert: " << msg);
        }
    }
}

void
VanetRoutingExperiment::ForwardToExternalServer (const std::string &payload)
{
#ifdef __linux__
  if (m_extUdpSock >= 0 && m_liveUdpPort > 0)
    {
      struct sockaddr_in servAddr;
      servAddr.sin_family = AF_INET;
      servAddr.sin_port = htons (m_liveUdpPort);
      servAddr.sin_addr.s_addr = inet_addr ("127.0.0.1");
      sendto (m_extUdpSock, payload.c_str (), payload.size (), 0,
              reinterpret_cast<struct sockaddr *> (&servAddr), sizeof (servAddr));
    }
#else
  (void) payload;
#endif
}

void
VanetRoutingExperiment::SetupScenario ()
{
  // member variable parameter use
  // defaults or command line overrides,
  // except where scenario={1,2,3,...}
  // have been specified, in which case
  // specify parameters are overwritten
  // here to setup for specific scenarios

  if (m_scenario == 1)
    {
      // 40 nodes in RWP 300 m x 1500 m synthetic highway, 10s
      m_traceFile = "";
      m_logFile = "";
      m_mobility = 2;
      if (m_nNodes == 156)
        {
          m_nNodes = 40;
        }
      if (m_TotalSimTime == 300.01)
        {
          m_TotalSimTime = 10.0;
        }
    }
  else if (m_scenario == 2)
    {
      // Urban V2I Socket Scenario (30 mobile OBU vehicles + 4 fixed RSU stations, 180s)
      if (m_traceFile.empty ())
        {
          m_traceFile = "ns3_mobility_30nodes.tcl";
        }
      m_logFile = "vanet_v2i_sim.log";
      m_mobility = 1;
      m_nNodes = 30;
      m_TotalSimTime = 180.0;
      m_nodeSpeed = 25;
      m_nodePause = 0;
      m_CSVfileName = "vanet_v2i_timeseries.csv";
      m_CSVfileName2 = "vanet_v2i_summary.csv";
    }
}

void
VanetRoutingExperiment::WriteCsvHeader ()
{
  //blank out the last output file and write the column headers
  std::ofstream out (m_CSVfileName.c_str ());
  out << "SimulationSecond," <<
    "ReceiveRate," <<
    "PacketsReceived," <<
    "NumberOfSinks," <<
    "RoutingProtocol," <<
    "TransmissionPower," <<
    "WavePktsSent," <<
    "WavePtksReceived," <<
    "WavePktsPpr," <<
    "ExpectedWavePktsReceived," <<
    "ExpectedWavePktsInCoverageReceived," <<
    "BSM_PDR1," <<
    "BSM_PDR2," <<
    "BSM_PDR3," <<
    "BSM_PDR4," <<
    "BSM_PDR5," <<
    "BSM_PDR6," <<
    "BSM_PDR7," <<
    "BSM_PDR8," <<
    "BSM_PDR9," <<
    "BSM_PDR10," <<
    "MacPhyOverhead" <<
    std::endl;
  out.close ();

  std::ofstream out2 (m_CSVfileName2.c_str ());
  out2 << "BSM_PDR1,"
       << "BSM_PDR2,"
       << "BSM_PDR3,"
       << "BSM_PDR4,"
       << "BSM_PDR5,"
       << "BSM_PDR6,"
       << "BSM_PDR7,"
       << "BSM_PDR8,"
       << "BSM_PDR9,"
       << "BSM_PDR10,"
       << "AverageRoutingGoodputKbps,"
       << "MacPhyOverhead"
       << std::endl;
  out2.close ();
}

int
main (int argc, char *argv[])
{
  VanetRoutingExperiment experiment;
  experiment.Simulate (argc, argv);
}
