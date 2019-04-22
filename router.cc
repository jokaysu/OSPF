//router.cc

#include "Topology.h"
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <netdb.h>

using namespace std;

int main(int argc, char const *argv[]) {
	int rid, nsehost, nseport, rport;

	//handle in-line arg
	if (argc != 5) {
		cerr << "Usage: ./router router_id nse_host nse_port router_port" << endl;
		return -1;
	}
	rid = atoi(argv[1]);
	struct hostent *hlist;
	hlist = gethostbyname(argv[2]);
	if (hlist == NULL) {
		cerr << "ERROR: can not get hostname" << endl;
		return -2;
	}
	memcpy(&nsehost, hlist->h_addr_list[0], hlist->h_length);
	nseport = atoi(argv[3]);
	rport = atoi(argv[4]);

	//connect with nse
	struct sockaddr_in address;
	int nsefd;
	if ((nsefd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		cerr << "ERROR: can not build socket" << endl;
		return -3;
	}
	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(rport);
	if (bind(nsefd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		close(nsefd);
		cerr << "ERROR: can not bind socket" << endl;
		return -4;
	}
	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(nseport);
	memcpy(&(address.sin_addr), &nsehost, sizeof(int));

	//initializing logging
	stringstream ss;
	ss << "router" << rid << ".log";
	string tempStr = ss.str();
	const char *logFileName = tempStr.c_str();
	freopen(logFileName, "w", stdout);

	//send init packet
	struct pkt_INIT initpkt;
	initpkt.router_id = (unsigned int) rid;
	cout << "R" << rid << " sends INIT" << endl;
	if (sendto(nsefd, &initpkt, sizeof(initpkt), 0, 
		(struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0) {
		cerr << "ERROR: can not send" << endl;
		return -5;
	}

	//receive from nse
	struct circuit_DB cdb;
	cout << "R" << rid << " receives initial CDB" << endl;
	int rc = recvfrom(nsefd, &cdb, sizeof(cdb), 0, NULL, NULL);
	if (rc != sizeof(circuit_DB)) {
		cerr << "ERROR: receive something wrong" << endl;
		return -6;
	}

	//send hello to neighbour
	for (unsigned int i = 0; i < cdb.nbr_link; ++i) {
		struct pkt_HELLO hellopkt;
		hellopkt.router_id = rid;
		hellopkt.link_id = cdb.linkcost[i].link;
		cout << "R" << rid << " sends Hello via link " << cdb.linkcost[i].link << endl;
		if (sendto(nsefd, &hellopkt, sizeof(hellopkt), 0, 
			(struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0) {
			cerr << "ERROR: can not send" << endl;
			return -5;
		}
	}

	//receiving
	Topology topo(NBR_ROUTER, rid, nsefd, &address);
	topo.addSelf(cdb);
	topo.printTopo();
	char buf[sizeof(struct pkt_LSPDU)];
	while (true) {
		int rc = recvfrom(nsefd, &buf, sizeof(buf), 0, NULL, NULL);
		if (rc < 0) {
			cerr << "ERROR: receive something wrong" << endl;
			return -6;
		} else if (rc == sizeof(struct pkt_HELLO)) {
			struct pkt_HELLO *helloptr = (struct pkt_HELLO *) buf;
			cout << "R" << rid << " receives Hello via link " << helloptr->link_id
				<< " from " << helloptr->router_id << endl;
			topo.replyHello(helloptr);
		} else if (rc == sizeof(struct pkt_LSPDU)) {
			struct pkt_LSPDU *lspduptr = (struct pkt_LSPDU *) buf;
			cout << "R" << rid << " receives from R" << lspduptr->sender << " via link id " <<
				lspduptr->via << " that R" << lspduptr->router_id  << " has a link with id " <<
				lspduptr->link_id << " and cost " << lspduptr->cost << endl;
			topo.handleLSPDU(lspduptr);
		}
	}

	//closing everything
	fclose(stdout);
}