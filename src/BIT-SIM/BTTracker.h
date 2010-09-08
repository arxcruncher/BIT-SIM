#ifndef _BTTracker_H_
#define _BTTracker_H_

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

#include <ctype.h>
#include <iostream>
#include <vector>
#include <string>

#include "BitTorrent.h"
#include "BTClient.h"

/*!
    \brief Unifies data of each BT client registred at the traker.
*/

struct userID{
        /*!
          \brief Peer ID of the client.
          */
	string peer_id;

        /*!
          \brief Gate through which the client is reachable.
	
		If the gate is equal to -1 the client is not reachable.
          */
	int gate;

        /*!
          \brief The amount of bytes downloaded in the swarm.
          */
	unsigned int downloaded;

        /*!
          \brief The amount of bytes uploaded in the swarm.
          */
	unsigned int uploaded;

	/*!
          \brief The amount of bytes still left to download.
          */
	unsigned int left;
};

/*!
    \brief The BTTracker class implements the tracker functionalities.
    \date 2009-06-09
    \version 1.2
    \author Blekinge Institute of Technology TEK-ATS
*/
class BTTracker : public BitTorrent{
	private :
		/*!
			\brief Amount of seeders in the swarm.
		*/
		short complete;
		/*!
			\brief Amount of leechers in the swarm.
		*/
		short incomplete;
		/*!
			\brief Amount of clients that will connect in the future to the swarm.
		*/
		unsigned int left;
		
		/*!
			\brief Total amount of clients that will connect in the simulation.
		*/
		int amountOfClients;
		/*!
			\brief Amount of peers in the swarm.
		*/
		int peersinswarm;
		
		cOutVector completedpeers; 	// Output vector for completed downloads.
		cOutVector totpeers;  		// Output vector for number of total peers.
		cOutVector totseeds;		// Output vector for number of seeds.
		/*!
			\brief Defines the time in which each BT client should report himself back.
		*/
		// TODO: make this interval work at the side of the client.
		int interval;
		/*!
			\brief Array containing every registred BT client at the tracker.
		*/
		vector<userID> users;
		/*!
			\brief Returns a pointer to the information of \e peer.
		*/
		vector<userID>::iterator peerLookUp(const char *peer);
		/*!
			\brief Sends message \e msg to \e peer.
		*/
		void sentToClient(const char *peer, cMessage *msg);
		/*!
			\brief Returns the gate at which \e peer is connected to.
		*/
		int getGate(const char *peer);
		/*!
			\brief Registers the gate at which \e peer is connected to.
		*/
		int setGate(const char *peer, int gateNum);
		/*!
			\brief Adds \e numWanted random peers to the tracker response message \e tr.
		*/
		void getRandomPeers(TrackerResponse *tr, unsigned int numWanted, const char *peer);
	
    public :
        BTTracker();
		/*!
			\brief Frees the gate to which \e peer is connected to.
		*/
		void freeGate(const char *peer);
		/*!
			\brief displays the content of the \e users array.
		*/
		void displayUser();
		/*!
			\brief Sets the amount of clients that will connect to the swarm during the simulation.

			this function is called by the ClientAssigner module at the start up of the simualtion.
		*/
		void setAmountOfClients(int clients);
		/*!
			\brief Sets up the tracker.

			Must be called upon creation of the simulation. This is done by OMNeT++ itself.
		*/
		void initialize();
		/*!
			\brief Handles incomming mesasges.
			
			This function is called by OMNeT++.
		*/
		void handleMessage(cMessage* msg);
		/*!
			\brief Called upon finalisiation of the simulation.

			This is done by OMNeT++ itself.
		*/
		void finish();
};

#endif
