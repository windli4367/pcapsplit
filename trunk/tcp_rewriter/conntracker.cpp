#include "conntracker.h"

#include <netinet/tcp.h>

#include <iostream>
#include <algorithm>

uint32_t ConnTracker::start_seq = 0;

bool ConnTracker::compare_seq(const ConnPacket& l, const ConnPacket& r)
{
	if ((l.seq >= start_seq && r.seq >= start_seq) || (l.seq < start_seq && r.seq < start_seq))
		return l.seq < r.seq;
	else if (l.seq < start_seq && r.seq >= start_seq)
		return true;
	else if (r.seq < start_seq && l.seq >= start_seq)
		return false;

	// we should never get here
	std::cerr << "ConnTracker::compare_seq logical error detected! start_seq == " << start_seq
		  << " l.seq == " << l.seq << " r.seq == " << r.seq << std::endl;
	return true;
}

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
		std::cout << "Reordering connction " << connCounter << "..." << std::endl;
		reorderConnection(i->second);
		++connCounter;
	}
}

void ConnTracker::reorderConnection(PacketList& pList)
{
	start_seq = pList.begin()->seq;
	std::sort(pList.begin(), pList.end(), compare_seq);
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
