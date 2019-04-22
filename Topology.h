//Topology.h

#include "common.h"
#include <netinet/in.h>
#include <list>
#include <utility>
#include <map>

using namespace std;

class Topology {
	//private members
	int numRouters;
	int rid;
	int nsefd;
	struct sockaddr_in *addrptr;

	struct circuit_DB link[NBR_ROUTER + 1]; 
	map<int, int> existedLink; // map<link, router>
	pair<int, int> adjMatrix[NBR_ROUTER + 1][NBR_ROUTER + 1];// pair<link, cost>
	bool helloed[NBR_ROUTER + 1];

	//for dji
	void djiAlgo();
	pair<int, int> ans[NBR_ROUTER + 1];//pair<router, cost>

	void printRIB();

  public:
  	Topology(int, int, int, struct sockaddr_in *);
  	void addSelf(struct circuit_DB);
  	void printTopo();
  	void replyHello(struct pkt_HELLO *);
  	void handleLSPDU(struct pkt_LSPDU *);
};