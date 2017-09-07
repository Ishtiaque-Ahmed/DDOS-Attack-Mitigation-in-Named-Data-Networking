#topology and cpp file should be in ndnSIM/ns-3/src/ndnSIM/examples
#topology should be in topology
cd ..
cd ndnSIM
cd ns-3
NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-load-balancer

