//Topology.cc

#include "Topology.h"
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <unistd.h>
#include <netdb.h>

Topology::Topology(int numRouters, int rid, int nsefd, struct sockaddr_in *addrptr)
	: numRouters(numRouters), rid(rid), nsefd(nsefd), addrptr(addrptr){
	for (int i = 1; i <= numRouters; ++i) {
		link[i].nbr_link = 0;
		helloed[i] = false;
		for (int j = 1; j <= numRouters; ++j) 
			adjMatrix[i][j] = make_pair(-1, -1);
	}
	existedLink.clear();
}

void Topology::addSelf(struct circuit_DB cdb) {
	link[rid] = cdb;
	for (unsigned int i = 0; i < cdb.nbr_link; ++i) {
		existedLink[cdb.linkcost[i].link] = rid;
	}
}

void Topology::printTopo() {
	cout << endl;
	cout << "# Topology DB" << endl;
	for (int i = 1; i <= numRouters; ++i) {
		cout << "R" << rid << " -> R" << i << " nbr link " << link[i].nbr_link << endl;
		for (unsigned int j = 0; j < link[i].nbr_link; ++j) {
			cout << "R" << rid << " -> R" << i;
			cout << " link " << link[i].linkcost[j].link;
			cout << " cost " << link[i].linkcost[j].cost << endl;
		}
	}
	cout << endl;
}

void Topology::replyHello(struct pkt_HELLO *pktptr) {
	helloed[pktptr->router_id] = true;
	if (existedLink[pktptr->link_id] > 0) {
		int tempCost;
		for (unsigned int i = 0; i < link[rid].nbr_link; ++i) 
			if (link[rid].linkcost[i].link == pktptr->link_id)
				tempCost = link[rid].linkcost[i].cost;
		adjMatrix[pktptr->router_id][rid] 
			= make_pair(pktptr->link_id, tempCost);
		adjMatrix[rid][pktptr->router_id] 
			= make_pair(pktptr->link_id, tempCost);
	} else {
		existedLink[pktptr->link_id] = pktptr->router_id;
	}
	struct pkt_LSPDU lspkt;
	for (unsigned int i = 0; i < link[rid].nbr_link; ++i) {
		lspkt.sender = rid;
		lspkt.router_id = rid;
		lspkt.link_id = link[rid].linkcost[i].link;
		lspkt.cost = link[rid].linkcost[i].link;
		lspkt.via = pktptr->link_id;
		cout << "R" << rid << " sends LSPDU sender " << lspkt.sender << " router_id " << 
			lspkt.router_id << " link_id " << lspkt.link_id << " cost " << 
			lspkt.cost << " via " << lspkt.via << " to " << pktptr->router_id << endl;
		if (sendto(nsefd, &lspkt, sizeof(lspkt), 0, 
			(struct sockaddr *)addrptr, sizeof(struct sockaddr_in)) < 0) {
			cerr << "ERROR: can not send" << endl;
		}
	}
}

void Topology::handleLSPDU(struct pkt_LSPDU *pktptr) {
	//check exist
	for (unsigned int i = 0; i < link[pktptr->router_id].nbr_link; ++i) {
		if (link[pktptr->router_id].linkcost[i].link == pktptr->link_id)
			return; //exist then ignore
	}

	//update
	link[pktptr->router_id].linkcost[link[pktptr->router_id].nbr_link].link = pktptr->link_id;
	link[pktptr->router_id].linkcost[link[pktptr->router_id].nbr_link].cost = pktptr->cost;
	link[pktptr->router_id].nbr_link++;
	if (existedLink[pktptr->link_id] > 0) {
		adjMatrix[pktptr->router_id][existedLink[pktptr->link_id]] 
			= make_pair(pktptr->link_id, pktptr->cost);
		adjMatrix[existedLink[pktptr->link_id]][pktptr->router_id] 
			= make_pair(pktptr->link_id, pktptr->cost);
	} else {
		existedLink[pktptr->link_id] = pktptr->router_id;
	}

	//print new
	printTopo();

	//spreading
	pktptr->sender = rid;
	for (int i = 1; i <= numRouters; ++i) 
		if (adjMatrix[i][rid].second > 0 && (unsigned int)i != pktptr->sender && helloed[i]) {
			pktptr->via = adjMatrix[i][rid].first;
			cout << "R" << rid << " sends LSPDU sender " << pktptr->sender << " router_id " << 
				pktptr->router_id << " link_id " << pktptr->link_id << " cost " << 
				pktptr->cost << " via " << pktptr->via << " to " << i << endl;
			if (sendto(nsefd, pktptr, sizeof(*pktptr), 0, 
				(struct sockaddr *)addrptr, sizeof(struct sockaddr_in)) < 0) {
				cerr << "ERROR: can not send" << endl;
			}
		}

	//dji
	djiAlgo();

	//printRIB
	printRIB();
}

void Topology::djiAlgo() {
	//init
	bool npi[numRouters];
	for (int i = 1; i <= numRouters; ++i) {
		npi[i] = false;
		if (adjMatrix[i][rid].first > 0) {
			ans[i].first = i;
			ans[i].second = adjMatrix[i][rid].second;
		} else {
			ans[i].first = -1;
			ans[i].second = MY_INF;
		}
	}
	npi[rid] = true;

	//loop
	while (true) {
		//cerr << "k" << endl;
		//check exit
		bool flag = true;
		for (int i = 1; i <= numRouters; ++i) {
			if (!npi[i]) {
				flag = false;
				break;
			}
		}
		if (flag) break;

		//find w
		pair<int, int> minAns; // pair<router, cost>
		minAns.second = MY_INF; 
		for (int i = 1; i <= numRouters; ++i) {
			if (!npi[i] && ans[i].second <= minAns.second) {
				minAns.first = i;
				minAns.second = ans[i].second;
			}
		}

		//cerr << "minAns " << minAns.first << " " << minAns.second << endl;

		//add w to npi
		npi[minAns.first] = true;

		//update w's neighbour
		for (int i = 1; i <= numRouters; ++i) {
			if (!npi[i] && adjMatrix[i][minAns.first].first > 0) {
				if (ans[minAns.first].second + adjMatrix[i][minAns.first].second < ans[i].second) {
					ans[i].first = ans[minAns.first].first;
					ans[i].second = ans[minAns.first].second + adjMatrix[i][minAns.first].second;
				}
			}
		}//for
	}//while
}

void Topology::printRIB() {
	cout << endl;
	cout << "# RIB" << endl;
	for (int i = 1; i <= numRouters; ++i) {
		if (i == rid) {
			cout << "R" << rid << " -> R" << i << " -> Local, 0" << endl;
		} else {
			cout << "R" << rid << " -> R" << i << " -> R" << ans[i].first
				<< ", " << ans[i].second << endl;
		}
	}
	cout << endl;
}