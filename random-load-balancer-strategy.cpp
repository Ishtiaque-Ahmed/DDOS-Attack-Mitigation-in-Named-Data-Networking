/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California,
 *                      Arizona Board of Regents,
 *                      Colorado State University,
 *                      University Pierre & Marie Curie, Sorbonne University,
 *                      Washington University in St. Louis,
 *                      Beijing Institute of Technology,
 *                      The University of Memphis
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "random-load-balancer-strategy.hpp"
#include <boost/random/uniform_int_distribution.hpp>
#include <list>
#include <ndn-cxx/util/random.hpp>
#include "core/logger.hpp"
#include "ns3/node.h"
#include "ns3/ndnSIM-module.h"
#include <fstream>
#include <vector>
#include <cstdlib>
#include <stdio.h>
#include <ctime>

#include <iostream>

NFD_LOG_INIT("RandomLoadBalancerStrategy");

namespace nfd {
namespace fw {
//void get_malicious_links();
const Name
  RandomLoadBalancerStrategy::STRATEGY_NAME("ndn:/localhost/nfd/strategy/random-load-balancer");

RandomLoadBalancerStrategy::RandomLoadBalancerStrategy(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder, name)

  {
        {

           using namespace std;
           freopen("src/ndnSIM/examples/weight.txt","r",stdin);
           int n,k;k=0;
               while(cin>>n)
                {
                    cout<<n<<endl;
                    k++;
                }
                no_of_producers=k;

                for(int i=0;i<=k;i++)
                {
                    usage[i]=0;
                }


        ifstream myfile;
        myfile.open("src/ndnSIM/examples/safe_links.txt");
        std::string link;
        int limit;
        while(myfile>>link>>limit)
        {
            safe_interest[link]=0;
            max_limit[link]=limit;
        }
        cout<<"printing malicious links "<<endl;
        for(auto it=safe_interest.begin();it!=safe_interest.end();it++)
        {
            cout<<it->first<<" "<<it->second<<endl;

        }

                //get_malicious_links();

       }
}


RandomLoadBalancerStrategy::~RandomLoadBalancerStrategy()
{
}

bool is_safe_interest(string link)
{



}

static bool
canForwardToNextHop(const Face& inFace, shared_ptr<pit::Entry> pitEntry, const fib::NextHop& nexthop)
{
  return !wouldViolateScope(inFace, pitEntry->getInterest(), nexthop.getFace()) &&
    canForwardToLegacy(*pitEntry, nexthop.getFace());
}

string extract_interest_name(string s)
{
    {
        using namespace std;
        int last_index_prefix=-1;
        for(int i=0;i<s.length();i++)
        {
            if(s[i]=='/')last_index_prefix=i;
        }
        string original_name="";
        for(int i=0;i<last_index_prefix;i++)
        {
            original_name+=s[i];
        }
        return original_name;

    }


}



static bool
hasFaceForForwarding(const Face& inFace, const fib::NextHopList& nexthops, const shared_ptr<pit::Entry>& pitEntry)
{
  return std::find_if(nexthops.begin(), nexthops.end(), bind(&canForwardToNextHop, cref(inFace), pitEntry, _1))
         != nexthops.end();
}
vector<int>get_weight()
{
    {
    using namespace std;
    ifstream myfile;
    myfile.open("src/ndnSIM/examples/weight.txt");
    int n;
    int start=0;
     cout<<"start "<<start<<endl;
    vector<int>producer_index; // set according to their weight
    while(myfile>>n)
    {
        n=n/100;
        //cout<<""
        for(int i=0;i<n;i++)
        {
            producer_index.push_back(start);

        }
        //cout<<"start "<<start<<endl;
        start++;
    }

     random_device rd;
    mt19937 g(rd());
    shuffle(producer_index.begin(), producer_index.end(), g);

   // myfile.close();
    return producer_index;
    }

}

/*size_t get_random_producer(vector<int>v)
{

    size_t randomIndex=rand()%(v.size());
    return v[randomIndex];
}*/

void
RandomLoadBalancerStrategy::afterReceiveInterest(const Face& inFace, const Interest& interest,
                                                 const shared_ptr<pit::Entry>& pitEntry)
{

  NFD_LOG_TRACE("afterReceiveInterest");
    std::string s=pitEntry.get()->getName().toUri();
    std::string link=extract_interest_name(s);
    std::cout<<"interest name  "<<link<<"\n";



    auto it=safe_interest.find(link);
    if(it==safe_interest.end())
    {
        auto it1=malicious_interest.find(link);
        if(it1==malicious_interest.end())
        {
            malicious_interest[link]=0;
        }
        else
        {
            malicious_interest[link]+=1;
        }
    this->rejectPendingInterest(pitEntry);
    return;


    }
    safe_interest[link]+=1;


    //int it=pitEntry.get()->getInRecords().size();
    //std::cout<<"pit "<<it<<"\n";
   // std::cout<<"no of machine 0 "<<get_count<<"\n";
   // std::cout<<"Interest life time is  "<<interest.getInterestLifetime()<<"\n";
   // std::cout<<"Node Id "<<this->GetObject<Node>()->GetId()<<"\n";

    //Ptr<Pit> pit = this->GetObject<Pit>();

 //std::cout<<"interest name  "<< pitEntry.getName()<<"\n";
 //const ns3::ndn::ndnSIM::LoadStatsNode &stats = this->GetStats (Name ());


  if (hasPendingOutRecords(*pitEntry)) {
    // not a new Interest, don't forward
    return;
  }

  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();
  //const pit::Entry& Entry =
  //std::cout<<nfd::pit::size()<<"  is the size \n";
  // Ensure there is at least 1 Face is available for forwarding
  if (!hasFaceForForwarding(inFace, nexthops, pitEntry)) {
    this->rejectPendingInterest(pitEntry);
    return;
  }
    std::cout<<"size is "<<nexthops.size()<<"\n";
   fib::NextHopList::const_iterator selected;
  //int cnt=0;
  do {

    vector<int>v=get_weight();
    boost::random::uniform_int_distribution<> dist(0, no_of_producers-1);
    size_t randomIndex = dist(m_randomGenerator);
    std::cout<<"now selected :  "<<randomIndex<<"\n";
    usage[randomIndex]+=1;

    for(int i=0;i<no_of_producers;i++)
    {
        cout<<"producer no : "<<i+1<<" usage : "<<usage[i]<<"\n";
    }

     std::cout<<"incoming face is"<<inFace.getId()<"\n";
    /*boost::random::uniform_int_distribution<> dist(10, 180);
    size_t randomIndex = dist(m_randomGenerator);
    randomIndex=randomIndex%4;
    if(randomIndex>2)randomIndex=2;
    std::cout<<"r="<<randomIndex;
    if(randomIndex==1)
    {
        //cnt++;
        //std::cout<<"count is "<<cnt;
        //if(cnt>1)randomIndex=2;
    }
    std::cout<<"hi  "<<randomIndex;

    if(randomIndex==0)get_count++;*/
    uint64_t currentIndex = 0;
    for (selected = nexthops.begin(); selected != nexthops.end() && currentIndex != randomIndex; ++selected,++currentIndex) {


    }
  } while (!canForwardToNextHop(inFace, pitEntry, *selected));

  this->sendInterest(pitEntry, selected->getFace(), interest);
}

} // namespace fw
} // namespace nfd
