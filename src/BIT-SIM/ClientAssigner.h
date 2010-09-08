#ifndef _ClientAssigner_H_
#define _ClientAssigner_H_

/*
 * Defines a client managment class.
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

#include <ctype.h>
#include <map>
#include <iostream>
#include <vector>
#include <string>

#include "BTClient.h"
#include "BTTracker.h"

class BTClient;
class BTTracker;

using namespace std;

/*!
    \brief Manages the peers in the swarm from the simulators side.
	
	ClientManager creates and deletes peers in the swarm during the simulation. Connections are created automatically along with the peers.
	
    \date 2009-03-06
    \version 1.2
    \author Blekinge Institute of Technology TEK-ATS
*/
class ClientAssigner : public cSimpleModule{
	protected:
        /*!
          \brief Stores the amount of seeders in the simualtion.
          */
		int seeders;
        /*!
          \brief Stores the amount of peers in the simulation.
          */
		int clients;
        /*!
          \brief Stores the maximum amount of connections that a client may handle.
		
		This corresponds to the maximum amount of TCP connections a client will maintain.
          */
		int maxNumberOfConnections;
        /*!
          \brief Pointer pointing to an array that stores the seeder states of all BTClients.
          */
		bool *seederArray;

        /*!
          \brief Pointer to the BTTracker in the simulator.
          */
		BTTracker *tracker;
        /*!
          \brief Pointer to an array of pointers pointing to every BTClient in the simualtion.
          */
		BTClient **clientArray;
	
	public:
        /*!
          \brief Initialises all variables.
          */
		ClientAssigner();
        /*!
            \brief Initializes the simulation, must be called before the start of the simulation.

		Automagically loads every BTClient and the BTTracker. Assigns leecher/seeder states to the clients.
 
         */
		void initialize();
        /*!
          \brief Tells the ClientAssigner that client with ID is a seeder now.
		  
		  ClientAssigner keeps track of statistics and terminates the simulation upon seeder domination, hence the ClientAssigner must know that leechers turn into seeders.

            \param ID The identifier of the client.
            \param seeder The state of the client, true = seeder, false = leecher.
          */
		void peerIsSeeder(int ID, bool seeder);
        /*!
          \brief Notifies the ClientAssigner that a client is leaving the swarm.

		The ClientAssigner then notifies every other module in the swarm to delete the records of that client. This is done for memory optimisation.

            \param peer The identifier of the client that is leaving.
          */
		void peerIsLeaving(const char *peer);
        /*!
          \brief Request the state of a client.

            \param ID The identifier of the client.
            \return The state of client \e ID.
            \retval 0 Client is a seeder
            \retval 1 Client is a leecher
            \retval 2 Client not found
          */
		int getMode(int ID);
};

#endif
