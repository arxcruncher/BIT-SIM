#ifndef _BTClient_H_
#define _BTClient_H_

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

#include <ctype.h>
#include <iostream>
#include <vector>
#include <string>
#include <queue>

#include <cpacketqueue.h>

#include "BitTorrent.h"
#include "ClientAssigner.h"

class ClientAssigner;

/*!
    \brief The BTClient defines the behaviour of a BitTorrent client.
    \date 2009-06-07
    \version 1.2
    \author Blekinge Institute of Technology TEK-ATS
*/
class BTClient : public BitTorrent{      
	private:
	/*!
		\brief Struct that identifies a queueing packet
	*/
	struct PacketQueueElement{
		cPacket *packet;
		simtime_t delay;
		string client;
	};
	/*!
		\brief Pointer to the class that manages the pieces.
	*/
		PieceManager *pieceManager;
	/*!
		\brief Pointer to the class that manages data related to connected peers.
	*/
		PeerManager *peerManager;
	/*!
		\brief Pointer to a message used for indication transmission inprogress.
	*/
		cPacket *transmissionPacket;
	/*!
		\brief Pointer to a message used for general purpose timers.
	*/
		cPacket *timer;
	/*!
		\brief Queue for outgoing messages.
	*/
		vector<PacketQueueElement> packetQueue;
	/*!
		\brief Output vector (see OMNeT) that tracks incoming piece requests.
	*/
		cOutVector incomingRequests;
	/*!
		\brief Output vector (see OMNeT) that tracks outgoing piece requests.
	*/
		cOutVector outgoingRequests;
	/*!
		\brief Output vector (see OMNeT) that tracks the amount of connected peers.
	*/
		cOutVector connectedPeers;
	/*!
		\brief Output vector (see OMNeT) that tracks disconnection of peers.
	*/
		cOutVector disconnectedPeers;
	/*!
		\brief Output vector (see OMNeT) that tracks have message traffic.
	*/
		cOutVector haveMessages;
	/*!
		\brief The amount of time that will be seeded once the seeder states is attained.
		
		This value is set by the ClientAssigner upon creation of the peer. This for statistical purposes.
	*/
		simtime_t seed_time;			
	/*!
		\brief The time when the peer will join the swarm.
		
		@deprecated This variable is currently not used and will be deleted in future releases.
	*/
		simtime_t start_time;			// Time when peer starts.
	/*!
		\brief The time when the peer will leave the swarm.

		@deprecated This variable is currently not used and will be deleted in future releases.
	*/
		simtime_t stop_time;			// Time when peer leaves.
	/*!
		\brief TO FILL OUT!!!!!!!!!!!!!!!!!!! Incoming session size statistic ??????
	*/
		double session_size_in;
	/*!
		\brief TO FILL OUT!!!!!!!!!!!!!!!!!!! Outgoing session size statistic ??????
	*/
		double session_size_out;
		
	/*!
		\brief Stores the URL of the tracker.
	*/
		string tracker;
	/*!
		\brief Denotes if module is busy with sending data.
	*/
		bool busySending;
	/*!
		\brief Set if peer is leecher.
	*/
		bool leecher;
	/*!
		\brief Set if peer is seeder.
	*/
		bool seeder;
	/*!
		\brief Defines if peer is in end game mode.
	*/
		bool endGameMode;
	/*!
		\brief Sets all conenctions open for traffic.

		@deprecated This variable is currently not used and will be deleted in future releases.
	*/
		bool gatesOpen;
	/*!
		\brief Defines if the peer is a seeder from the moment it joined the swarm.
	*/
		bool initialSeeder;
	/*!
		\brief Defines if there are assymetrical links between the peers in the P2P network.
	*/
		int  assymetricLinks;
	/*!
		\brief Defines the gate through which the BTTracker is reachable.
	*/
		int  trackerGate;
	/*!
		\brief Defines the arrival time.

		This value is set by the ClientAssigner upon creation of the peer. This for statistical purposes.
	*/
		simtime_t arrivalTime;
	/*!
		\brief Defines the seeding time.

		This value is set by the ClientAssigner upon creation of the peer. This for statistical purposes.
	*/
		simtime_t seedingTime;
	/*!
		\brief Stores the ID of the client.
	*/
		int myID;
	/*!
		\brief Defines the algorithm used for piece selection.
		
		This value is laoded from the initialisation file at the startup of the simulation (see OMNeT).
	*/
		int algorithm;
	/*!
		\brief Counts the amount of times the choke timer has been triggered.
	*/
		unsigned int chokeRound;
	/*!
		\brief Tracks the amount of bytes that has been downloaded.
	*/
		unsigned int downloaded;
	/*!
		\brief Tracks the amount of bytes that has been uploaded.
	*/
		unsigned int uploaded;
	/*!
		\brief Stores the peers that are participating in the end-game mode.
	*/
		vector<string> endGameModePeers;
	/*!
		\brief Stores the messages that are waiting to be send.
		
		Messages are placed in a queue because measurements have shown that piece messages are processed according to a specific distribution. Queuing these message gives the abillity to control this behaviour. This is necassery because OMNeT is an event based simulator.
	*/
		vector<Piece*> pendingMessages;
	/*!
		\brief Pointer to the ClientAssigner module in the simulator.
	*/
		ClientAssigner *ca;
	/*!
		\brief Handles incomming tracker responses.
	*/
		void handleTrackerResponse(TrackerResponse *trq);
	/*!
		\brief Handles incomming handshakes.
	*/
		void handleHandshake(Handshake *hds);
	/*!
		\brief Handles incomming bitfields.
	*/
		void handleBitfield(BitField *hds);
	/*!
		\brief Handles incomming have-piece messages.
	*/
		void handleHavePiece(HavePiece *hpc);
	/*!
		\brief Handles timer events.
	*/
		void handleKeepAliveTimer(cPacket *timer);
	/*!
		\brief Handles incomming interested messages.
	*/
		void handleInterested(Interested *inr);
	/*!
		\brief Handles incomming not-interested messages.
	*/
		void handleNotInterested(NotInterested *inr);
	/*!
		\brief Handles self messages.
	*/
		void handleSelfMessage(cPacket *msg);
	/*!
		\brief Handles incomming unChoke messages.
	*/
		void handleUnChoke(UnChoke *msg);
	/*!
		\brief Handles incomming piece requests.
	*/
		void handlePieceRequest(PieceRequest *prq);
	/*!
		\brief Handles incomming pieces.
	*/
		void handlePiece(Piece *pce);
	/*!
		\brief Handles incomming choke messages.
	*/
		void handleChoke(Choke *uch);
	/*!
		\brief Handles incomming cancel messages.
	*/
		void handleCancel(Cancel *cnl);
	/*!
		\brief Establishes a connection with \e peer.
		\return Whether the connection could be established
		\retval 0 succesfull
		\retval -1 not succesfull
	*/
		int  makeConnectionToPeer(const char* peer);
	/*!
		\brief Deletes all the connections to other identities in the swarm.
	*/
		void deleteAllConnections();
	/*!
		\brief Generates a new piece request and sends it to \e peer.

		If no piece could be found to download then a not interested message will be send instead.
	*/		
		void requestNewPiece(const char *peer_id);
	/*!
		\brief Chokes a peer and stores the information accordingly.
		\param client Peer to be choked\unChoked
		\param choke Choke or unChoke the peer
	*/
		void chokePeer(const char *client, bool choke);
	/*!
		\brief Sends a message to the traker.
	*/
		void sendToTracker(cPacket *msg);
	/*!
		\brief Disconnects the gates (link) to another peer.
		
		This operation also disconnects the gates (see OMNeT) at the other side of the connection.
	*/
		void freeBothGates(const char *peer);
	/*!
		\brief Enqueues a packet waiting to be send.
		\param msg Pointer to the message to be send
		\param client The ID of the peer to which message will be send
		\param delay The waiting time before the message will be send
	*/
		int queuePacket(const char *client, cPacket *msg, simtime_t delay);
	/*!
		\brief Transmits the first message in the packetQueue.
	*/
		void sendMessage();
	/*!
		\brief Sends a message to a peer.
		
		Is called through queuePacket, don't use this call directly!
		\param msg Pointer to the message to be send
		\param client The ID of the peer to which message will be send
		\param delay The waiting time before the message will be send
	*/
		int  do_sendToClient(const char *client, cPacket *msg, simtime_t delay);
	
    public :
        BTClient();
	/*!
		\brief Does the nessecary things when a peer leaves the swarm.
		\param peer The Peer ID of the peer that leaves the swarm
	*/
		void handlePeerIsGone(const char *peer);
	/*!
		\brief Disconnects the gate through which \e peer is reachable.
	*/
		void freeGate(const char *peer);
	/*!
		\brief Sets the seeder status.
		
		When this function is called the peer will behave as a seeder.
	*/
		void setSeeder();
	/*!
		\brief Sets the leecher state.
		\param aTime The arrival time in the swarm
		\param sTime The time the leecher will be seeding once it is turned into a seeder
	*/
		void setLeecher(simtime_t aTime, simtime_t sTime);
	/*!
		\brief Initialises the BTClient class, this is called by the OMNeT++ simulation core.
	*/
		void initialize();
	/*!
		\brief Handles incomming messages, this is called by the OMNeT++ simulation core.
	*/
		void handleMessage(cMessage* msg);
	/*!
		\brief Collects the nessecary statistics, this is called by the OMNeT++ simulation core at the end of the simulation.
	*/
		void finish();
};

#endif
