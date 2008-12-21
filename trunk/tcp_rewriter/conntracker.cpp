#include "conntracker.h"

#include <netinet/tcp.h>

#include <iostream>

ConnTracker::ConnTracker()
{

}

	
void ConnTracker::addPacket(const uint8_t* packetData, struct pcap_pkthdr* packetHeader)
{
	static uint64_t id = 0;
	TcpFlowKey key(packetData);
	if (!key.isTCP()) 
		return;

	ConnPacket connPacket;
	connPacket.oldId = id;
	connPacket.newId = id;
	connPacket.seq = key.seq;
	connPacket.ack = key.ack;

	connList[key].push_back(connPacket);
	id++;
}

void ConnTracker::reorder()
{
	int connCounter = 1;
	for (ConnList::iterator i = connList.begin(); i != connList.end(); ++i) {
		std::cout << "Reordering connction " << connCounter << std::endl;

		++connCounter;
	}
}

void ConnTracker::removeDuplicates()
{

}

void ConnTracker::generateOutputList()
{
	for (ConnList::iterator i = connList.begin(); i != connList.end(); ++i) {
		for (PacketList::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			outputList[j->oldId] = j->newId;
		}
	}
}