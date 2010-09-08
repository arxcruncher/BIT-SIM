/*
 * Defines a BitTorrent Tracker.
 *
 * This file is part of a BitTorrent Simulator
 *
 * Copyright (c) 2009. Blekinge Institute of Technology (BTH).
 * All Rights Reserved.
 *
 * Contact information for redistribution (binary or source code):
 *
 * Karel De Vogeleer
 * Department of Telecommunications System
 * School of Engineering
 * SE-371 79 Karlskrona
 * SWEDEN
 *
 * kdv@bth.se
 *
 */

#include <omnetpp.h>

#include "BTTracker.h"
#include "Convert.h"

#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

Define_Module(BTTracker);

BTTracker::BTTracker(){
	left = 0;
	complete = 0;
	incomplete = 0;
	amountOfClients = 0;
	interval = 3600;
// 	port = 6969;
 	peer_id="NULL";
}

void BTTracker::setAmountOfClients(int clients){
	amountOfClients = clients;
}

void BTTracker::freeGate(const char *peer){
	// cout << "Tracker is deleting connection to " << peer << endl;
	int gateNum = getGate(peer);
	if(gateNum != -1){
		if(gate("out", gateNum)->isConnected() == true){
			gate("out", gateNum)->disconnect();
		}
	}
}

void BTTracker::initialize(){
	
	WATCH(complete);
	WATCH(incomplete);
	WATCH(URL);
	
	completedpeers.setName("completedDownloads");
	totpeers.setName("peersInSwarm");
	totseeds.setName("seedsInSwarm");
	peersinswarm = 0;
	
	
	torrentFile = check_and_cast<TorrentFile*>(getParentModule()->getSubmodule("torrentFile"));
	URL = torrentFile->getValue("announce");
	ev << "Tracker " << URL << " is ready." << endl;
}

void BTTracker::handleMessage(cMessage* msg){
	if(msg->getKind()== Tracker_Request){
		TrackerRequest *trq = check_and_cast<TrackerRequest*>(msg);
		vector<userID>::iterator it = peerLookUp(trq->getPeer_id());
		
		// If peer not in list, the peer is a new client, so add him to the database
		if( it == users.end()){
			//Generate a Tracker response message
			string msgName = "Response .torrent : ";
			msgName.append(trq->getPeer_id());
			TrackerResponse *tr = new TrackerResponse(msgName.c_str());
			tr->setInterval(interval);
			tr->setComplete(complete);
			tr->setIncomplete(incomplete);		
			tr->setKind(Tracker_Response);
			
			// Add peers to the peerlist
			getRandomPeers(tr, (unsigned int)trq->getNumwant(), trq->getPeer_id());
			
			// Add requesting user to the database
			if(string(trq->getEvent()).compare("started") == 0) incomplete++;
			if(string(trq->getEvent()).compare("completed") == 0) complete++;
			userID newUser;
			newUser.peer_id = trq->getPeer_id();
			newUser.downloaded = trq->getDownloaded();
			newUser.uploaded = trq->getUploaded();
			newUser.gate = trq->getArrivalGate()->getIndex();
			ev << " setting gate ID " << trq->getArrivalGate()->getIndex() << endl;
			newUser.left = trq->getLeft();
			users.push_back(newUser);
			
			//Send response through the correct gate
			sentToClient(trq->getPeer_id(), tr);
			// ev << "Tracker " << URL << " added " << trq->getPeer_id() << " to the swarm." << endl;
			// cout << " TRACKER: " << trq->getPeer_id() << "\tjoined the swarm, amount of registred users: " << users.size()+left << "/" << amountOfClients << endl;

			totpeers.recordWithTimestamp(simTime(),incomplete);
			totseeds.recordWithTimestamp(simTime(),complete);
		}
		else{
			// ev << "Tracker " << trq->getPeer_id() << ", complete: "  << complete << ", incomplete: " << incomplete << ", mount of registred users: " << users.size() << endl;
			if(string(trq->getEvent()).compare("status") == 0){
				// Client is here for status report
				it->downloaded = trq->getDownloaded();
				it->uploaded = trq->getUploaded();
				it->left = trq->getLeft();
			}
			else if(string(trq->getEvent()).compare("completed") == 0){
				// Client is here to say it is a seeder now
				complete++;
				incomplete--;
				it->downloaded = trq->getDownloaded();
				it->uploaded = trq->getUploaded();
				it->left = trq->getLeft();
				//cout << " TRACKER: " << trq->getPeer_id() << "\tfinished, complete: "  << complete << ", incomplete: " << incomplete << ", amount of registred users: " << users.size()+left << "/" << amountOfClients << endl;
				totpeers.recordWithTimestamp(simTime(),incomplete);
				totseeds.recordWithTimestamp(simTime(),complete);
			}
			else if(string(trq->getEvent()).compare("stopped") == 0){
				// Client is here to say it is going to leave the swarm
				it->downloaded = trq->getDownloaded();
				it->uploaded = trq->getUploaded();
				it->left = trq->getLeft();
				vector<userID>::iterator it = peerLookUp(trq->getPeer_id());
				if(it != users.end()){
					users.erase(it);
					//cout << " TRACKER: " << trq->getPeer_id() << "\tleft the swarm."  << endl;
					// displayUser();
					left++;
				}
				// TODO: In real BT, a peer can leave before becoming a seed.
				complete--;
				totpeers.recordWithTimestamp(simTime(),incomplete);
				totseeds.recordWithTimestamp(simTime(),complete);
			}
			else if(string(trq->getEvent()).compare("newPeers") == 0){
				// Client is here to ask for new peers
				string msgName = "Response .torrent : ";
				msgName.append(trq->getPeer_id());
				TrackerResponse *tr = new TrackerResponse(msgName.c_str());
				tr->setInterval(interval);
				tr->setComplete(complete);
				tr->setIncomplete(incomplete);		
				tr->setKind(Tracker_Response);
				// Add peers to the peerlist
				//cout << " TRACKER response to " << trq->getPeer_id() << endl;
				getRandomPeers(tr, (unsigned int)trq->getNumwant(), trq->getPeer_id());
				sentToClient(trq->getPeer_id(), tr);
			}
			if(incomplete == 0 && users.size()+left == amountOfClients){
				delete trq;
				//cout << " TRACKER: is ending simulation at " << simTime() << "."  << endl;
				endSimulation();
			}
		}
	}
	delete msg;
}

vector<userID>::iterator BTTracker::peerLookUp(const char *peer){
	vector<userID>::iterator it = users.begin();
	for(; it != users.end(); it++){
		if(it->peer_id.compare(peer) == 0) return it;
	}
	return users.end();
}

int BTTracker::getGate(const char *peer){
	vector<userID>::iterator it = peerLookUp(peer);
	if(it != users.end()) return it->gate;
	return -1;
}

void BTTracker::displayUser(){
	cout << "Tracker --------------------- PeerList -------------------------- BEGIN" << endl;
	for(vector<userID>::iterator it = users.begin(); it != users.end() ;it++){
		cout << it->peer_id << endl;
	}
	cout << "Tracker --------------------- PeerList ---------------------------- END" << endl;
}

void BTTracker::sentToClient(const char *peer, cMessage *msg){
	int gateNum = getGate(peer);
	if(gateNum != -1){
		ev << "Tracker outgate size: " << gateSize("out") << ", sending to " << gateNum << endl;
		send(msg, "out", gateNum);
	}
}

void BTTracker::getRandomPeers(TrackerResponse *tr, unsigned int numWanted, const char *peer){
	// Calculate the size of the peer array. If the number of peers
	//	 requested is bigger than the registred number
	//   of peers, then use the amount of registred users.
	int user = 0;
 	if(users.size() <= numWanted ) user = users.size();
 	else user = (int)numWanted;
	tr->setPeersArraySize(user);
	int peers[user];
 	int index = 0;
	// Choose the peers send back ad random, but make sure there are no duplicates.
 	if((unsigned int)user != users.size()){
		bool found = false;
 		while(index < user){
			int random = intuniform(0,(int)users.size()-1);
 			for(int i = 0; i <= index ; i++) if(peers[i] == random && users.at(i).peer_id.compare(peer) == 0) found = true;
 			if(!found){
				peers[index] = random;
 				index++;
 			}
			else found = false;
 		}
 	}
 	else for(int i = 0; i < user ; i++) peers[i] = (int)i;
	
	// Add the peers to the tracker response
	for(int i = 0; i < user; i++){
		string entry = users.at(peers[i]).peer_id;
		tr->setPeers(i, entry.c_str());
	}
}

void BTTracker::finish(){

}
