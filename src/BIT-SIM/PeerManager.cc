/*
 * Defines a peer managment class.
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

#include <vector>
#include <iostream>
#include <string>

#include "PeerManager.h"

using namespace std;

vector<Peer>::iterator PeerManager::peerLookUp(const char *peer){
	for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
		if(it->peer_id.compare(peer)==0) return it;
	}
	return peers.end();
}

void PeerManager::getNewPeers(vector<string> *peer){
	for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
		if(it->status == CLOSED){
			peer->push_back(it->peer_id);
		}
	}
}

int PeerManager::registredPeer(const char *peer){
	if(peerLookUp(peer) == peers.end()) return -1;
	else return 0;
}

int PeerManager::getStatus(const char *peer){
	vector<Peer>::iterator it = peerLookUp(peer);
	if(it == peers.end()) return -1;
	else return it->status;	
}

void PeerManager::setPeerAsSeeder(const char *peer){
	vector<Peer>::iterator it = peerLookUp(peer);
	it->seeder = true;
}

int PeerManager::isPeerASeeder(const char *peer){
	vector<Peer>::iterator it = peerLookUp(peer);
	if(it->seeder) return 0;
	return  -1;
}

int PeerManager::getGate(const char *peer){
	vector<Peer>::iterator it = peerLookUp(peer);
	if(it != peers.end()) return it->gate;
	return -1;
}

void PeerManager::setGate(const char *peer, int gate){
	vector<Peer>::iterator it = peerLookUp(peer);
	if(it != peers.end()){
		it->gate =gate;
	}
}

void PeerManager::addPeer(const char *peer){
	Peer newPeer;
	newPeer.peer_id = "";
	newPeer.status = CLOSED;
	newPeer.interested = false;
	newPeer.choked = true;
	newPeer.seeder = false;
	newPeer.downloaded = 0;
	newPeer.statusChange = 0.0;
	newPeer.time = 0;
	newPeer.gate = -1;
	ostringstream entry;
	newPeer.peer_id = peer;
	if(peerLookUp(newPeer.peer_id.c_str()) == peers.end()) peers.push_back(newPeer);
}

int PeerManager::resetPendingPeers(simtime_t time){
	vector<Peer>::iterator it = peers.begin();
	while(it != peers.end()){
		if(it->status == PENDING){
			if((it->statusChange+60.0) < time){
				it->status = CLOSED;
			}
		}
		it++;
	}
	return 0;
}

void PeerManager::conOpenend(const char *peer, simtime_t time){
	vector<Peer>::iterator it = peerLookUp(peer);
	if(it != peers.end()){
		it->status = OPEN;
		it->statusChange = time;
	}
}

void PeerManager::conPending(const char *peer, simtime_t time){
	vector<Peer>::iterator it = peerLookUp(peer);
	if(it != peers.end()){
		it->status = PENDING;
		it->statusChange = time;
	}
}

const char* PeerManager::getRandomInterested(){
	int random = 0;
	if(getIntrests() == 0){
		while(true){
			random = intuniform(0, peers.size()-1);
			if(peers.at(random).interested && peers.at(random).choked) return peers.at(random).peer_id.c_str();
		}
	}
	return NULL;
}

// TODO: getRandomInterested() and getRandomChoked() are the same?

const char* PeerManager::getRandomChoked(){
	int random = 0;
	if(getChoked()==0){
		while(true){
			random = intuniform(0, peers.size()-1);
			if(peers.at(random).choked && peers.at(random).interested) return peers.at(random).peer_id.c_str();
		}
	}
	return NULL;
}

int PeerManager::getPeerChoked(const char *peer){
	vector<Peer>::iterator it = peerLookUp(peer);
	if(it->choked) return 0;
	return -1;
}

void PeerManager::getFastestDownloader(vector<string> *peerList){
	int max = 0;
	vector<Peer>::iterator fastest, number1, number2 = peers.end();
	
	//Go through the top 3 downloaders
	for(unsigned int i = 0; i<3; i++){
		max = 0;
		//Make sure there are enough candidates
		if(peers.size()>i){
			for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
				if(it->downloaded > max && !it->seeder && it!=number1 && it!=number2){
					max = it->downloaded;
					fastest = it;
				}
			}
			if(max != 0){
				if     (i==0) number1 = fastest;
				else if(i==1) number2 = fastest;
				
				if(fastest != peers.end()){
					peerList->push_back(fastest->peer_id);
				}
			}
			max = 0;
		}
	}
}

void PeerManager::getRecentlyUnchoked(vector<string> *peerList){
	simtime_t max = 0;
	vector<Peer>::iterator fastest, number1, number2, number3 = peers.end();

	//Go through the top 3 downloaders
	for(unsigned int i = 0; i<4; i++){
		max = 0;
		//Make sure there are enough candidates
		if(peers.size()>i){
			for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
				if(it->time > max && !it->choked && it->interested && it!=number1 && it!=number2 && it!=number3){
					max = it->time;
					fastest = it;
				}
			}
			if(max != 0){
				if     (i==0) number1 = fastest;
				else if(i==1) number2 = fastest;
				else if(i==2) number3 = fastest;
				
				if(fastest != peers.end()){
					peerList->push_back(fastest->peer_id);
				}
			}
		}
	}
}

void PeerManager::setChokePeer(const char *peer, bool choke, simtime_t time){
	vector<Peer>::iterator it = peerLookUp(peer);
	it->choked = choke;
	it->time = time;
}

void PeerManager::isPeerChoked(vector<string> *unChoked, vector<string> *choked){
	for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
		if(!it->choked){
			bool nameMatch = false;
			for(unsigned int i = 0; i < unChoked->size(); i++){
				if(it->peer_id.compare(unChoked->at(i)) == 0) nameMatch = true;	
			}
			if(!nameMatch){
				choked->push_back(it->peer_id);
			}
		}
	}
}

void PeerManager::deletePeer(const char *peer){
	vector<Peer>::iterator it = peerLookUp(peer);
	if(it != peers.end()) peers.erase(it);
}

void PeerManager::resetDownload(){
	for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
		it->downloaded = 0;
	}
}

void PeerManager::registerDownload(const char *peer, int byte){
	vector<Peer>::iterator it = peerLookUp(peer);
	if(it != peers.end()) it->downloaded += byte;
}

void PeerManager::setPeerInterested(const char* peer, bool interest){
	vector<Peer>::iterator it = peerLookUp(peer);
	it->interested = interest;
}

void PeerManager::getPeerList(vector<string> *peerList){
	for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
		peerList->push_back(it->peer_id);
	}
}

int PeerManager::getIntrests(){
	for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
		if(it->interested && it->choked) return 0;
	}
	return -1;
}

int PeerManager::getChoked(){
	for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
		if(it->choked && it->interested) return 0;
	}
	return -1;
}

int PeerManager::getAmountOfConnections(){
	int peersCon = 0;
	for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++) if(it->status == 2 || it->status == 1) peersCon++;
	return peersCon;
}

void PeerManager::writePeers(){
	cout << "--------------PeerManager-OPEN-------------" << endl;
	for(vector<Peer>::iterator it = peers.begin(); it != peers.end(); it++){
		cout << "PeerID: " << it->peer_id << "\tstatus: " << it->status << "   choked: " << it->choked << "   interseted: " << it->interested << endl;
	}
	cout << "--------------PeerManager-END--------------" << endl;
}

void PeerManager::setPeerID(string name){
	peer_id = name;
}

PeerManager::~PeerManager(){
	writePeers();
}
