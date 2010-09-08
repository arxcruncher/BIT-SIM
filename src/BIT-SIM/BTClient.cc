/*
 * Defines a BitTorrent Client.
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
#include <cpacketqueue.h>

#include "BTClient.h"
#include "Convert.h"

#include <iostream>
#include <queue>
#include <vector>
#include <stdlib.h>


using namespace std;

Define_Module(BTClient);

BTClient::BTClient(){
	peerManager = new PeerManager();
	pieceManager = new PieceManager();
	timer = NULL;
	transmissionPacket = new cPacket();
	transmissionPacket->setKind(Transmission_Ready);

	// TODO: remove the gatesOpen variable.
	gatesOpen = true;
	leecher = false;
	seeder = false;
	endGameMode = false;
	initialSeeder = false;
	busySending = false;
	assymetricLinks = 0;
	
	trackerGate = -1;
	chokeRound = 0;
	downloaded = 0;
	uploaded = 0;
	algorithm = 0;
	arrivalTime = 0.0;
	seedingTime = 0.0;
	session_size_in = 0;
	session_size_out = 0;
}

void BTClient::setSeeder(){
	initialSeeder = true;
	seeder = true;
	seed_time = 0.0;
}

void BTClient::setLeecher(simtime_t aTime, simtime_t sTime){
	leecher = true;
	arrivalTime = aTime;
	seedingTime = sTime;
}

void BTClient::initialize(){
	//Generate the nesceccary identifiers
	WATCH(peer_id);
	WATCH(busySending);
	WATCH(leecher);
	WATCH(seeder);
	WATCH(endGameMode);
	WATCH(algorithm);
	myID = 0;
	WATCH(myID);
	string t;
	
	assymetricLinks = par("assymetricLinks");
	algorithm = par("algorithm");
	
	// TODO: very ugly piece of code, must be changed !!
	// Retrieve ID from name
	char *ID = new char[20]; 
	strcpy(ID, getFullName());
	int i = 0;
	while(ID[i] != '\0'){
		if(isdigit(ID[i])){
			myID *=10;
			myID +=ID[i]-48;
		}
		i++;
	}
	peer_id = "PEERID=";
	peer_id.append(stringify(myID));
	delete[] ID;
	
	t = string("incomingPieceRequests_") + stringify(myID);
	incomingRequests.setName(t.c_str());
	t = string("outgoingPieceRequests_") + stringify(myID);
	outgoingRequests.setName(t.c_str());
	t = string("connectedPeers_") + stringify(myID);
	connectedPeers.setName(t.c_str());
	t = string("disconnectedPeers_") + stringify(myID);
	disconnectedPeers.setName(t.c_str());
	t = string("haveMessagesIn_") + stringify(myID);
	haveMessages.setName(t.c_str());
	
	URL = "NULL";
	
	ca = check_and_cast<ClientAssigner*>(getParentModule()->getSubmodule("SmartClientAssigner"));
	
	// Connect to the tracker
	cDatarateChannel *channel1 = (cDatarateChannel*)cChannelType::getDatarateChannelType()->create("DUMMY");
  	cDatarateChannel *channel2 = (cDatarateChannel*)cChannelType::getDatarateChannelType()->create("DUMMY");
  	if(assymetricLinks == 1){
  		channel1->setDelay((double)par("channelDelay")/1000);
  		channel1->setDatarate(par("channelBandwidth"));
  		channel2->setDelay((double)par("channelDelay")/1000);
  		channel2->setDatarate(par("channelBandwidth"));
  	}
  	else{
  		double delay_deprc = (double)par("channelDelay")/1000;
  		double band = par("channelBandwidth");
  		channel1->setDelay(delay_deprc);
  		channel1->setDatarate(band);
  		channel2->setDelay(delay_deprc);
  		channel2->setDatarate(band);
  	}
	int myGate = getFreeGate();
	int otherGate = check_and_cast<BTTracker*>(getParentModule()->getSubmodule("tracker"))->getFreeGate();
	trackerGate = myGate;
	
	gate("out", myGate)->connectTo(check_and_cast<BTTracker*>(getParentModule()->getSubmodule("tracker"))->gate("in", otherGate), channel1);
	check_and_cast<BTTracker*>(getParentModule()->getSubmodule("tracker"))->gate("out", otherGate)->connectTo(gate("in", myGate), channel2);
	
	// Get .torrent information
	torrentFile = check_and_cast<TorrentFile*>(getParentModule()->getSubmodule("torrentFile"));
	pieceManager->init(atof(torrentFile->getValue("length").c_str()), atof(torrentFile->getValue("piece length").c_str()), algorithm);
	// The block length value is interpreted as bytes.
	pieceManager->setBlockLength((int)pow(2,par("blockLength")));
	tracker = torrentFile->getValue("announce");
	
	pieceManager->setPeerID(peer_id);
	peerManager->setPeerID(peer_id);
	
	//Create a tracker request and shedule it
	string msgName = "Waiting to join: ";
	msgName.append(peer_id);
	TrackerRequest *tr = new TrackerRequest(msgName.c_str());
	tr->setKind(Tracker_Request);
	tr->setPeer_id(peer_id.c_str());
	tr->setDownloaded(downloaded);
	tr->setUploaded(uploaded);
	tr->setLeft(pieceManager->getRemainingDownload());
	if(seeder)  tr->setEvent("completed");
	else tr->setEvent("started");
	tr->setNumwant(par("numberOfPeersWanted"));
	scheduleAt(arrivalTime,tr);
	
 	if(seeder){
  		if(ev.isGUI()) getDisplayString().setTagArg("i",1,"gold");
  		pieceManager->setSeeder();
  		pieceManager->initialisePieceArray(1.0);
  	}
  	else{
  		double pieceDistribution = par("pieceDistribution");
  		pieceManager->initialisePieceArray(pieceDistribution);
  	}
	
}

void BTClient::handleTrackerResponse(TrackerResponse *trq){
	for(unsigned int i = 0; i < trq->getPeersArraySize(); i++){
		if(peer_id.compare(trq->getPeers(i)) != 0 ) peerManager->addPeer(trq->getPeers(i));
	}
	vector<string> peer;
	peerManager->getNewPeers(&peer);
	for(unsigned int i = 0; i < peer.size() && peerManager->getAmountOfConnections() <= getAmountOfEmptyGates(); i++){
		if(makeConnectionToPeer(peer.at(i).c_str()) == 0){
			Handshake *msg = new Handshake();
			msg->setKind(HandShake);
			//msg->setByteLength(HandShake_Length);
			msg->setPeer_id(peer_id.c_str());
			queuePacket(peer.at(i).c_str(), msg, 0);
			peerManager->conPending(peer.at(i).c_str(), simTime());
			//else cout << peer_id << " connecting failed!" << endl;
		}
	}
	delete trq;
}

int BTClient::makeConnectionToPeer(const char* peer){
	// TODO change the name client* of all peers to PEERID=*
	int k = 0;
	int clientID = 0;
	// Get the number of the client
	while(peer[k] != '\0'){
		if(isdigit(peer[k])){
			clientID *=10;
			clientID +=peer[k]-48;
		}
		k++;
	}
	
	// If gate is a null pointer a connection is not been made, thus create a connection
		
	BTClient* destination = check_and_cast<BTClient*>(getParentModule()->getSubmodule((string("client")+stringify(clientID)).c_str()));
	
	int otherGate = destination->getFreeGate();
	if(otherGate == -1) return -1;
	int myGate = getFreeGate();
	
	cDatarateChannel *channel1 = (cDatarateChannel*)cChannelType::getDatarateChannelType()->create("DUMMY");
  	cDatarateChannel *channel2 = (cDatarateChannel*)cChannelType::getDatarateChannelType()->create("DUMMY");
  	if(assymetricLinks == 1){
		channel1->setDelay((double)par("channelDelay")/1000);
  		channel1->setDatarate(par("channelBandwidth"));
  		channel2->setDelay((double)par("channelDelay")/1000);
  		channel2->setDatarate(par("channelBandwidth"));
  	}
  	else{
		double delay = (double)par("channelDelay")/1000;
  		double band = par("channelBandwidth");
  		channel1->setDelay(delay);
  		channel1->setDatarate(band);
  		channel2->setDelay(delay);
  		channel2->setDatarate(band);
  	}
	gate("out", myGate)->connectTo(destination->gate("in", otherGate), channel1);
	destination->gate("out", otherGate)->connectTo(gate("in", myGate), channel2);
	
	peerManager->setGate(peer, myGate);	
	return 0;
}

void BTClient::freeBothGates(const char *peer){
	cGate *outGate = NULL;
	cGate *gateDestination = NULL;
	BitTorrent *otherSide = NULL;

	int gateNum = peerManager->getGate(peer);

	if(gateNum >= 0){
		outGate = gate("out",gateNum);
		if(outGate->isConnected() == true){
			gateDestination = outGate->getNextGate();
			otherSide = check_and_cast<BitTorrent*>(gateDestination->getOwnerModule());
			outGate->disconnect();
			otherSide->freeGate(peer_id.c_str());
			peerManager->setGate(peer, -1);
		}
	}
}

void BTClient::freeGate(const char *peer){
	cGate *outGate = NULL;
	int gateNum = peerManager->getGate(peer);
	
	if(gateNum >= 0){
		outGate = gate("out", gateNum);
		outGate->disconnect();
		peerManager->setGate(peer, -1);
	}
}

void BTClient::deleteAllConnections(){
	cGate *outGate = NULL;
	cGate *gateDestination = NULL;
	BitTorrent *otherSide = NULL;
	
	for(int i = 0; i < gateSize("out"); i++){	
		outGate = gate("out",i);
		if(outGate->isConnected() == true){
			gateDestination = outGate->getNextGate();
			otherSide = check_and_cast<BitTorrent*>(gateDestination->getOwnerModule());
			outGate->disconnect();
			otherSide->freeGate(peer_id.c_str());
		}
	}
}

void BTClient::handleHandshake(Handshake *hds){
	string sender = hds->getPeer_id();
	int curnum = peerManager->getAmountOfConnections();
	//Response on a handshake this client has send
	if(peerManager->getStatus(hds->getPeer_id()) == 1){
		peerManager->conOpenend(hds->getPeer_id(), simTime());
		//Send Bitfield over to the other side
		BitField *btf = new BitField();
		btf->setKind(BITField);
		btf->setByteLength(MessageHeader_Length+pieceManager->getLengthOfPieceArray());
		btf->setPeer_id(peer_id.c_str());
		pieceManager->writePiecesToMessage(btf);
		if(queuePacket(hds->getPeer_id(), btf, 0) == 0){}
		delete hds;		
	}
	else{
		//If this peer is new add him to the database
		if(peerManager->registredPeer(hds->getPeer_id())==-1){
			peerManager->addPeer(hds->getPeer_id());
		}
		//Set connection open
		peerManager->conPending(hds->getPeer_id(), simTime());
		//Send a handshake back
		hds->setKind(HandShake);
		hds->setByteLength(HandShake_Length);
		hds->setPeer_id(peer_id.c_str());
		if(queuePacket(sender.c_str(), hds, 0) == 0){}
			connectedPeers.recordWithTimestamp(simTime(), curnum);
	}
}

void BTClient::handleBitfield(BitField *hds){
	pieceManager->addPieceArrayOfPeer(hds);
	pieceManager->buildPieceIndex();
	if(pieceManager->getPieceArrayCompleteOfPeer(hds->getPeer_id()) == 0)
		peerManager->setPeerAsSeeder(hds->getPeer_id());
	string sender = hds->getPeer_id();
	
	if(peerManager->getStatus(hds->getPeer_id())==1){
		peerManager->conOpenend(hds->getPeer_id(), simTime());
		BitField *btf = new BitField();
		btf->setKind(BITField);
		btf->setByteLength( MessageHeader_Length + pieceManager->getLengthOfPieceArray() );
		btf->setPeer_id(peer_id.c_str());
		pieceManager->writePiecesToMessage(btf);
		if(queuePacket(sender.c_str(), btf, 0) == -1){}
	}
	
	if(leecher && pieceManager->getPieceArrayEmpty(hds->getPeer_id()) != 0){
		Interested *inr = new Interested();
		inr->setKind(Interested_Peer);
		inr->setByteLength(MessageHeader_Length);
		inr->setPeer_id(peer_id.c_str());
		queuePacket(sender.c_str(), inr, 0);
		pieceManager->setInterestedInPeer(hds->getPeer_id(), true);
	}
	delete hds;
}

void BTClient::handleHavePiece(HavePiece *hpc){
	pieceManager->addPiece(hpc->getPiece(), hpc->getPeer_id());
	
	if(pieceManager->getPieceArrayCompleteOfPeer(hpc->getPeer_id()) == 0){
		peerManager->setPeerAsSeeder(hpc->getPeer_id());
	}
	
	// Record have messages.
	haveMessages.recordWithTimestamp(simTime(),hpc->getPiece());
	
	// Check if you want to be interested or not in this peer
	if(pieceManager->getInterestedInPeer(hpc->getPeer_id()) == -1 && leecher){
		if(pieceManager->isThereSomethingToBeInterestedIn(hpc->getPeer_id()) == 0){
			Interested *inr = new Interested();
			inr->setKind(Interested_Peer);
			inr->setByteLength(MessageHeader_Length);
			inr->setPeer_id(peer_id.c_str());
			queuePacket(hpc->getPeer_id(), inr, 0);
			pieceManager->setInterestedInPeer(hpc->getPeer_id(), true);
		}
	}
	delete hpc;
}

void BTClient::chokePeer(const char* peer, bool choke){
	if(choke){
		Choke *unc = new Choke();
		unc->setKind(Choke_Peer);
		unc->setByteLength(MessageHeader_Length);
		unc->setPeer_id(peer_id.c_str());
		if(queuePacket(peer, unc, 0) == 0)	peerManager->setChokePeer(peer, true, simTime());
	}
	else{
		UnChoke *unc = new UnChoke();
		unc->setKind(UnChoke_Peer);
		unc->setByteLength(MessageHeader_Length);
		unc->setPeer_id(peer_id.c_str());
		queuePacket(peer, unc, 0);
		peerManager->setChokePeer(peer, false, simTime());
		// else peerManager->setChokePeer(peer, true, simTime());
	}
}

void BTClient::handleCancel(Cancel *cnl){
	for(vector<Piece*>::iterator it = pendingMessages.begin(); it != pendingMessages.end(); it++){
		if((*it)->getIndex() == cnl->getIndex() && (*it)->getBegin() == cnl->getBegin()){
			cancelAndDelete(*it);
			pendingMessages.erase(it);
			break;
		}
	}
	delete cnl;
}

void BTClient::handleKeepAliveTimer(cPacket *msg){
	//This timer arrives every 10s
	chokeRound++;

	//This part handles leecher state
	if(leecher){
		//Every 10 seconds unchoke a few peers
		vector<string> peerList;
		peerManager->getFastestDownloader(&peerList);
		for(unsigned int i = 0; i < peerList.size(); i++){
			if(peerList.at(i).compare("") != 0 && peerManager->getPeerChoked(peerList.at(i).c_str()) == 0){
				chokePeer(peerList.at(i).c_str(), false);
			}
		}
		peerManager->resetDownload();

		//Choke other peer(s) that are unchoked		
		vector<string> choked;
		peerManager->isPeerChoked(&peerList, &choked);
		for(unsigned short i = 0; i < choked.size(); i++){
			chokePeer(choked.at(i).c_str(), true);
			peerManager->setChokePeer(choked.at(i).c_str(), true, simTime());
		}
		
		//Every 30 seconds unchoke random peer
		if(chokeRound%3 == 0){
			if(peerManager->getIntrests()==0){
				chokePeer(peerManager->getRandomInterested(), false);
			}
		}
		
		//Every 5 minutes reconnect to inactive clients
		if(chokeRound%30 == 0){
			vector<string> peerList;
			peerManager->getNewPeers(&peerList);

			int rounds = getAmountOfEmptyGates();
			if(peerList.size() > 0){
				int i = 0;
				while(peerList.size() > i && rounds > i){
					string peer = peerList.at(i);
					if(makeConnectionToPeer(peer.c_str()) == 0){
						Handshake *msg = new Handshake();
						msg->setKind(HandShake);
						msg->setByteLength(HandShake_Length);
						msg->setPeer_id(peer_id.c_str());
						queuePacket(peer.c_str(), msg, 0);
						peerManager->conPending(peer.c_str(), simTime());
					}
					i++;
				}
			}
		}
		
		// Every 30 minutes ask the tracker for more peers
		if(chokeRound%180 == 0){
			string msgName = peer_id;
			msgName.append(": more peers needed");
			TrackerRequest *tr = new TrackerRequest(msgName.c_str());
			tr->setKind(Tracker_Request);
			tr->setPeer_id(peer_id.c_str());
			tr->setDownloaded(downloaded);
			tr->setUploaded(uploaded);
			tr->setLeft(pieceManager->getRemainingDownload());
			tr->setEvent("newPeers");
			tr->setNumwant(par("numberOfPeersWanted"));
			sendToTracker(tr);
		}
	}
	
	//This part handles seeder state
	if(seeder){
		//Every 10 seconds unchoke a few peers
		vector<string> peerList;
		peerManager->getRecentlyUnchoked(&peerList);
		if(chokeRound%3 != 0){
			//Unchoke random peer
			if(peerManager->getChoked() == 0) chokePeer(peerManager->getRandomChoked(), false);
			if(peerList.size() == 4){
				//Choke 4th peer
				chokePeer(peerList.at(3).c_str(), true);
			}
		}
	}
	
	// Every 1 minutes (60s) send Keep-Alive to everyone
	if(chokeRound%6 == 0){
		// Reset all peers that are pending for more then 1 minute.
		// Probably doesn't have any effect
		peerManager->resetPendingPeers(simTime());
	}
	
	if(chokeRound%180 == 0) chokeRound = 0;
	scheduleAt(simTime()+10, msg);
}

void BTClient::handleInterested(Interested *inr){
	peerManager->setPeerInterested(inr->getPeer_id(), true);
	delete inr;
}

void BTClient::handleNotInterested(NotInterested *inr){
	peerManager->setPeerInterested(inr->getPeer_id(), false);
	if(peerManager->getPeerChoked(inr->getPeer_id()) == -1){
		Choke *cpe = new Choke();
		cpe->setKind(Choke_Peer);
		cpe->setBitLength(MessageHeader_Length);
		cpe->setPeer_id(peer_id.c_str());
		if(queuePacket(inr->getPeer_id(), cpe, 0) == 0){}
		peerManager->setChokePeer(inr->getPeer_id(), true, simTime());
	}
	delete inr;
}

void BTClient::handleSelfMessage(cPacket *msg){
	if(msg->getKind()==Tracker_Request){		
		string msgName = "Sending trqcker request : client ";
		msgName.append(peer_id);
		msg->setName(msgName.c_str());
		sendToTracker(msg);
		
		// Launch timer to check different issues (see handleKeepAliveTimer)
		msgName = "Periodic check client ";
		msgName.append(stringify(myID));
		timer = new cPacket(msgName.c_str()); //Karel
		timer->setKind(KeepAlive_Timer);
		scheduleAt(simTime()+10, timer);
	}
	else if(msg->getKind()==Transmission_Ready){
		busySending = false;
		if(!busySending && packetQueue.size()>0){
			sendMessage();
		}
	}
	else if(msg->getKind()==New_Piece){
		Piece *pce = check_and_cast<Piece*>(msg);
		string client = pce->getPeer_id();
		peerManager->registerDownload(pce->getPeer_id(), pce->getLength());
		pce->setPeer_id(peer_id.c_str());
		queuePacket(client.c_str(), pce, 0);
		uploaded += pce->getLength();
		
		for(vector<Piece*>::iterator it = pendingMessages.begin(); it != pendingMessages.end(); it++){
			if((*it) == pce){
				pendingMessages.erase(it);
				break;
			}
		}
	}
	else if(msg->getKind()==KeepAlive_Timer){
		handleKeepAliveTimer(msg);
	}
	else if(msg->getKind()==Leaving_Swarm){
		// Send stopped message to tracker
		TrackerRequest *trq = new TrackerRequest();
		trq->setPeer_id(peer_id.c_str());
		trq->setEvent("stopped");
		trq->setNumwant(0);
		trq->setDownloaded(downloaded);
		trq->setUploaded(uploaded);
		trq->setLeft(pieceManager->getRemainingDownload());
		trq->setKind(Tracker_Request);
		sendToTracker(trq);
		cancelAndDelete(timer);
		gatesOpen = false;
		
		// Also set all channels to disabled, this to avoid problems in special situations
		deleteAllConnections();
		// Tell the client manager you are gone
		// The client manager will tell all the other peers that you are gone.
		ca->peerIsLeaving(peer_id.c_str());
		
		delete msg;
	}
}

void BTClient::requestNewPiece(const char *peer){		
	blockID blockId = pieceManager->pieceRequest(peer);
	// Create error mesage if blockId is out of bound	
	if(blockId.index != -1) {
		PieceRequest *prq = new PieceRequest();
		prq->setKind(Piece_Request);
		prq->setByteLength(MessageHeader_Length+Request_Lenght);
		prq->setPeer_id(peer_id.c_str());
		prq->setIndex(blockId.index);
		prq->setBegin(blockId.offset);
		prq->setBitLength(blockId.length);
		queuePacket(peer, prq, 0.005);
		outgoingRequests.recordWithTimestamp(simTime(), prq->getIndex());
	}
	else {
		// If there are no pieces left from 'peer' that we do not have then send
		// a not interested message.
		if(!endGameMode){
			NotInterested *nint = new NotInterested();
			nint->setKind(UnInterested_Peer);
			nint->setByteLength(MessageHeader_Length);
			nint->setPeer_id(peer_id.c_str());
			if(queuePacket(peer, nint, 0) == -1) {}
		}
	}
}

void BTClient::handleUnChoke(UnChoke *unc){
	pieceManager->setChokedByPeer(unc->getPeer_id(), false);
	requestNewPiece(unc->getPeer_id());
	delete unc;
}

void BTClient::handleChoke(Choke *unc){
	ev << unc->getPeer_id() << " chocked " << peer_id << "." << endl;
	pieceManager->setChokedByPeer(unc->getPeer_id(), true);
	delete unc;
}

void BTClient::handlePieceRequest(PieceRequest *prq){
	if(prq->getIndex() > pieceManager->getAmountOfPieces()){
		cerr << simTime() << " " << peer_id << " oeps, index out of bound from " << prq->getPeer_id() << ": ";
		cerr << prq->getIndex() << ">" << pieceManager->getAmountOfPieces() << endl;
	}
	if(pieceManager->getPiecePresent((unsigned int)prq->getIndex()) == 0){
		Piece *pce = new Piece();
		pce->setTimestamp( simTime());
		string naam = "Waiting to be send ";
		naam.append(peer_id);
		naam.append(" ...");
		pce->setName(naam.c_str());
		pce->setKind(New_Piece);
		
		// The sender is used for identification, the  fields will be filled out
		//   correctly when the message is actually send.
		pce->setPeer_id(prq->getPeer_id());
		pce->setByteLength(PieceHeader_Length+prq->getLength());
		
		pce->setIndex(prq->getIndex());
		pce->setBegin(prq->getBegin());
		pce->setBitLength(prq->getLength());
		simtime_t delay = (simtime_t)par("PieceRequestWaitingTime");
		delay /= 1000;
		pendingMessages.push_back(pce);
		incomingRequests.recordWithTimestamp(simTime(), prq->getIndex());
		scheduleAt(simTime()+delay, pce);
	}
	delete prq;
}

void BTClient::handlePeerIsGone(const char *peer){
	if(peerManager->registredPeer(peer) == 0){
		int curstat = peerManager->getStatus(peer);
		if( curstat == PENDING || curstat == OPEN ) { // PIs peer connected to us?
			disconnectedPeers.recordWithTimestamp(simTime(), 
			peerManager->getAmountOfConnections());
		}
		pieceManager->freePeer(peer);
		peerManager->deletePeer(peer);
	}
	
		
}

void BTClient::handlePiece(Piece *pce){
	// Call probability for bad transmittion if yes, re-request the packet.
	if(uniform(0,1000) != 666.0){
		// Register the download from user
		peerManager->registerDownload(pce->getPeer_id(), (int)pce->getLength());
		downloaded += pce->getLength();
		
		// If in end game mode cancel the other block request.
		if(endGameMode){
			//cout << simTime() << " entering end game mode." << endl;
			for(vector<string>::iterator peerList = endGameModePeers.begin(); peerList != endGameModePeers.end(); peerList++){
				if((*peerList).compare(pce->getPeer_id()) != 0){
					Cancel *cnl = new Cancel();
					cnl->setKind(Cancel_Piece);
					cnl->setByteLength(MessageHeader_Length+Request_Lenght);
					cnl->setPeer_id(peer_id.c_str());
					cnl->setIndex(pce->getIndex());
					cnl->setBegin(pce->getBegin());
					if(queuePacket((*peerList).c_str(), cnl, 0) == 0) {}
				}
			}
		}

		//Register the new block
		if(pieceManager->setBlock(pce->getIndex(), pce->getBegin()) == 1){
			//If piece is complete send a HAVE message to every leecher
			vector<string> peerList;
			peerManager->getPeerList(&peerList);
			for(vector<string>::iterator it = peerList.begin(); it != peerList.end(); it++){
				//Skip the seeders and the peers that still are in the handshakingstage
				//cout << peer_id << " " << (*it).c_str() << " " << peerManager->isPeerASeeder((*it).c_str()) << "==-1  " << peerManager->getStatus((*it).c_str()) << "==2" << endl;
				if(peerManager->isPeerASeeder((*it).c_str()) == -1 && peerManager->getStatus((*it).c_str()) == 2){
					//cout << peer_id << " sending have" << endl;
					HavePiece *hpc = new HavePiece();
					hpc->setKind(Have_Piece);
					hpc->setByteLength(MessageHeader_Length+Have_Length);
					hpc->setPeer_id(peer_id.c_str());
					hpc->setPiece(pce->getIndex());
					if(queuePacket((*it).c_str(), hpc, 0) == 0) {}
				}
			}
		}
		//else cout << simTime() << " piece " << pce->getIndex() << " incomplete." << endl;
		
		if(pieceManager->getPieceArrayComplete() == -1
			&& pieceManager->getChokedByPeer(pce->getPeer_id()) == -1
			&& pieceManager->isThereSomethingToBeInterestedIn(pce->getPeer_id()) == 0
			&& !endGameMode ){
			//cout << simTime() << " requesting new piece." << endl;
			requestNewPiece(pce->getPeer_id());
			}
		// Checking for activation of the end game mode
		if(pieceManager->getAmountOfCompletePieces() + pieceManager->getAmountOfPiecesInProgress() == pieceManager->getAmountOfPieces() && !endGameMode){
			//cout << simTime() << " endGame mode activated." << endl;
			endGameMode = true;
			vector<blockID> blocks;
			pieceManager->getLastBlocks(&blocks);
			pieceManager->getDownloadListOfPeers(&endGameModePeers);
			//cout << "End game mode block list size " << blocks.size() << endl;
			//cout << "End game mode peer list size " << endGameModePeers.size() << endl;
			for(vector<blockID>::iterator it = blocks.begin(); it != blocks.end() ; it++){
				for(vector<string>::iterator peerList = endGameModePeers.begin(); peerList != endGameModePeers.end(); peerList++){
					//cout << "if piece " << it->index << " block " << (unsigned int)it->offset << " is present " << pieceManager->getBlockPresentOfIncompletePeer((unsigned int)it->index, (unsigned int)it->offset, (*peerList).c_str()) << " == -1 " << endl;
					// Is this not always true
					if(pieceManager->getBlockPresentOfIncompletePeer((unsigned int)it->index, (unsigned int)it->offset, (*peerList).c_str()) == -1){
						//cout << "Sending requst for " << it->index << " block " << (unsigned int)it->offset << " is present to " << (*peerList).c_str() << endl;
						PieceRequest *prq = new PieceRequest();
						prq->setKind(Piece_Request);
						prq->setByteLength(MessageHeader_Length+Request_Lenght);
						prq->setPeer_id(peer_id.c_str());
						prq->setIndex((unsigned int)it->index);
						prq->setBegin((unsigned int)it->offset);
						prq->setBitLength((unsigned int)it->length);
						if(queuePacket((*peerList).c_str(), prq, 0) == 0) {}
					}
				}
			}
		}	
	
		if(pieceManager->getPieceArrayComplete() == 0 && leecher){
			leecher = false;
			seeder = true;
			ca->peerIsSeeder(myID, true);
			pieceManager->setSeeder();
			pieceManager->deleteIncompletePieces();
			if(ev.isGUI()) getDisplayString().setTagArg("i",1,"gold");
			
			// Send finished message to tracker
			TrackerRequest *trq = new TrackerRequest();
			trq->setPeer_id(peer_id.c_str());
			trq->setEvent("completed");
			trq->setNumwant(0);
			trq->setDownloaded(downloaded);
			trq->setUploaded(uploaded);
			trq->setLeft(pieceManager->getRemainingDownload());
			trq->setKind(Tracker_Request);
			
			seed_time = simTime();
			stop_time = simTime()+seedingTime;
			sendToTracker(trq);
			
			cout << "seed_time: " << seed_time << "\tstop_time: " << stop_time << endl;
			string name = peer_id;
			name.append(": leaving the swarm");
			cPacket *msg = new cPacket(name.c_str());
			msg->setKind(Leaving_Swarm);

			scheduleAt(simTime()+seedingTime, msg);
			
			vector<string> peerList;
			peerManager->getPeerList(&peerList);
			for(vector<string>::iterator it = peerList.begin(); it != peerList.end(); it++){
				if(peerManager->getStatus((*it).c_str()) == 2){
					if(pieceManager->getInterestedInPeer((*it).c_str()) == 0){
						NotInterested *nint = new NotInterested();
						nint->setKind(UnInterested_Peer);
						nint->setByteLength(MessageHeader_Length);
						nint->setPeer_id(peer_id.c_str());
						if(queuePacket((*it).c_str(), nint, 0) == 0) {}
					}
				}
			}
		}
	}
	else{
		//cout << "Re-request piece " << pce->getIndex() << " at peer " << pce->getPeer_id() << endl;
		PieceRequest *prq = new PieceRequest();
		prq->setKind(Piece_Request);
		prq->setByteLength(MessageHeader_Length+Request_Lenght);
		prq->setPeer_id(peer_id.c_str());
		prq->setIndex(pce->getIndex());
		prq->setBegin(pce->getBegin());
		prq->setBitLength(pce->getLength());
		if(queuePacket(pce->getPeer_id(), prq, 0) == 0) {}
	}		
	delete pce;
}

void BTClient::handleMessage(cMessage* Kmsg){
	cPacket *msg = check_and_cast<cPacket*>(Kmsg);
	if (msg->isSelfMessage()){
		handleSelfMessage(msg);
	}
	else{
		// Check if the connection is still up
		if(msg->getSenderGate()->isConnected() == true){
			session_size_in += msg->getByteLength();
			switch(msg->getKind()){
				case New_Piece : {
						Piece *pce = check_and_cast<Piece*>(msg);
						if(leecher) handlePiece(pce);
						else delete pce;
						//pieceManager->displayPiece();
						break;
					}
				case Piece_Request : {
						PieceRequest *prq = check_and_cast<PieceRequest*>(msg);
						//cout << simTime() << " piece request " << 	prq->getIndex() <<" - " << prq->getBegin() << endl;
						handlePieceRequest(prq);
						break;
					}
				case Tracker_Response : {
						TrackerResponse *trq = check_and_cast<TrackerResponse*>(msg);
						handleTrackerResponse(trq);
						break;
					}
				case HandShake : {
						Handshake *hds = check_and_cast<Handshake*>(msg);
						handleHandshake(hds);
						break;
					}
				case BITField : {
						BitField *btf = check_and_cast<BitField*>(msg);
						handleBitfield(btf);
						break;
					}
				case Have_Piece : {
						HavePiece *hpc = check_and_cast<HavePiece*>(msg);
						handleHavePiece(hpc);
						break;
					}
				case Interested_Peer : {
						Interested *hpc = check_and_cast<Interested*>(msg);
						handleInterested(hpc);
						break;
					}
				case UnInterested_Peer : {
						NotInterested *hpc = check_and_cast<NotInterested*>(msg);
						handleNotInterested(hpc);
						break;
					}
				case KeepAlive_Peer : {
						KeepAlive *kal = check_and_cast<KeepAlive*>(msg);
						delete kal;
						break;
					}
				case UnChoke_Peer : {
						UnChoke *uch = check_and_cast<UnChoke*>(msg);
						handleUnChoke(uch);
						break;
					}
				case Choke_Peer : {
						Choke *uch = check_and_cast<Choke*>(msg);
						handleChoke(uch);
						break;
					}
				case Cancel_Piece : {
						Cancel *cnl = check_and_cast<Cancel*>(msg);
						handleCancel(cnl);
						break;
					}
				case Default : {
					}
			}
		}
		else{
			// If channel is down delete all the information of the sending client
			delete msg;
		}
	}
}

void BTClient::finish(){
	
	// Sets the stop time for the seeders that were seeding from the beginning
	if(initialSeeder) stop_time = simTime();
	
	recordScalar("Start time", arrivalTime);
	recordScalar("Seed time", seed_time);
	recordScalar("Stop time", stop_time);
	recordScalar("Session duration", stop_time - arrivalTime);
	recordScalar("Session size upstream", session_size_out);
	recordScalar("Session size dowstream", session_size_in);
	
	delete transmissionPacket;
}

void BTClient::sendToTracker(cPacket *msg){
	send(msg,"out",trackerGate);
	session_size_out += msg->getByteLength();
}

int BTClient::queuePacket(const char *client, cPacket *msg, simtime_t delay){
	PacketQueueElement newMessage;
	newMessage.packet = msg;
	newMessage.delay = delay;
	newMessage.client = (string)client;
	packetQueue.push_back(newMessage);
	if(!busySending && packetQueue.size()>0){
		sendMessage();
	}
	return 0;
}

void BTClient::sendMessage(){
	do_sendToClient(packetQueue.at(0).client.c_str(), packetQueue.at(0).packet, packetQueue.at(0).delay);
	packetQueue.erase(packetQueue.begin());
}

int BTClient::do_sendToClient(const char *client, cPacket *msg, simtime_t delay){
	busySending = true;
	bool send = false;
	cGate *outGate;
	cGate *gateDestination;
	BitTorrent* otherSide;
	// If the clients data is erased (client == NULL) then the client left the 
	//   swarm. So this message does not have to be send.
	if(client != NULL){
		int gateNum = peerManager->getGate(client);		
		
		// If the gate to client is not created yet, look it up.
		if(gateNum == -1){
			//cout << simTime() << " gate not found to client " << client << "!" << endl;
			int i = 0;
			bool found = false;
			while(i < gateSize("out") && !found){
				outGate = gate("out",i);
				gateDestination = outGate->getPathEndGate();
				otherSide = check_and_cast<BitTorrent*>(gateDestination->getOwnerModule());
				// Checking for the right connection
				if(otherSide->getPeerID().compare(string(client)) == 0){
					peerManager->setGate(client, i);
					gateNum = i;
					found = true;
				}
				i++;
			}
			//cout << simTime() << " gatesize is " << gateNum << endl;
		}
	
		outGate = gate("out", gateNum);
		
		// This is to check if the connection is still up or a conenction is available
		if(outGate->isConnected() == true && outGate->isBusy() == false){
			string msgName = getFullName();
			msgName.append(" => ");
			msgName.append(stringify(simTime()));
			msg->setName(msgName.c_str());
			//simtime_t finish = outGate.getTransmissionFinishTime();
			sendDelayed(msg, delay, "out", gateNum);
			send = true;
			session_size_out += msg->getByteLength();
			scheduleAt(outGate->getTransmissionFinishTime(), transmissionPacket);
		}
	}
	
	if(send)	return 0;
	//delete msg;
	return -1;
}
