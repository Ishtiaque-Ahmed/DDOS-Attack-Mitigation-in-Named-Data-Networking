/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-load-balancer.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include <string>

#include "ndn-load-balancer/random-load-balancer-strategy.hpp"

using namespace ns3;

/**
 * This scenario simulates a load balancer topology (using topology reader module)
 *
 *               ( ) ----- ( ) ---- (consumer)
 *                |
 *        ------ ( ) -----
 *        |               |
 *    (producer) ---- (producer)
 *
 * All links are 1Mbps with propagation 10ms delay.
 *
 * FIB is populated using NdnGlobalRoutingHelper.
 *
 * Consumer requests data from the two producers with frequency 10 interests per
 * second (interests contain constantly increasing sequence number).
 *
 * For every received interest, a load balancing operation is performed
 * (based on a custom forwarding strategy) and the selected producer
 * replies with a data packet, containing 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-load-balancer
 */

using ns3::ndn::StackHelper;
using ns3::ndn::AppHelper;
using ns3::ndn::GlobalRoutingHelper;
using ns3::ndn::StrategyChoiceHelper;
using ns3::AnnotatedTopologyReader;

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/topo-load-balancer.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  StackHelper ndnHelper;
  ndnHelper.SetOldContentStore ("ns3::ndn::cs::Nocache");
  ndnHelper.InstallAll();

  // Installing global routing interface on all nodes
  GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> producer1 = Names::Find<Node>("pr1");
  Ptr<Node> producer2 = Names::Find<Node>("pr2");
  Ptr<Node> producer3 = Names::Find<Node>("pr3");

  Ptr<Node>hub=Names::Find<Node>("x1");

  NodeContainer consumerNodes;
  consumerNodes.Add(Names::Find<Node>("x9"));
  NodeContainer maliciousNodes;
  maliciousNodes.Add(Names::Find<Node>("x5"));
//consumerNodes.Add(Names::Find<Node>("x9"));
  // Install NDN applications
  std::string prefix = "/x1/hellooscar";

  // Install random-load-balancer forwarding strategy in
  // node PQR
  StrategyChoiceHelper::Install<nfd::fw::RandomLoadBalancerStrategy>(hub,
                                                                     prefix);



  prefix+="/ok";
  AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Frequency", StringValue("15")); // 100 interests a second
  consumerHelper.Install(consumerNodes);
//  std::string new_prefix=prefix+"/data";
   consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Frequency", StringValue("10"));
  consumerHelper.Install(maliciousNodes);


  /*AppHelper consumerHelper1("CustomApp");
  //consumerHelper1.SetPrefix(new_prefix);
  //consumerHelper1.SetAttribute("Frequency", StringValue("5")); // 100 interests a second
  consumerHelper1.Install(new_consumer);*/

  AppHelper producerHelper("ns3::ndn::Producer");

 // AppHelper producerHelper("CustomApp");
// std::string producer_prefix=prefix+"/ok";
  producerHelper.SetPrefix(prefix);

 // producerHelper.SetAttribute("prefix",StringValue(prefix));
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(producer1);
  producerHelper.Install(producer2);
  producerHelper.Install(producer3);

  // Add /prefix origins to ndn::GlobalRouter
  ndnGlobalRoutingHelper.AddOrigins(prefix, producer1);
  ndnGlobalRoutingHelper.AddOrigins(prefix, producer2);
   ndnGlobalRoutingHelper.AddOrigins(prefix, producer3);
   std::string new_producers="pr";
   for(int i=4;i<11;i++)
   {
        std::string temp=new_producers+to_string(i);
        std::cout<<"producer name : "<<temp<<"\n";
        Ptr<Node> producer= Names::Find<Node>(temp);
        producerHelper.Install(producer);
         ndnGlobalRoutingHelper.AddOrigins(prefix, producer);


   }


  // Calculate and install FIBs
  GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(2.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
