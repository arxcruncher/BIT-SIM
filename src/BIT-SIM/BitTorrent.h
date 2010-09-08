#ifndef _BitTorrent_H_
#define _BitTorrent_H_

/*
 * Defines basic functions for a BitTorrent entity.
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

#include "TrackerRequest_m.h"
#include "TrackerResponse_m.h"
#include "Handshake_m.h"
#include "BitField_m.h"
#include "HavePiece_m.h"
#include "PieceRequest_m.h"
#include "Choke_m.h"
#include "UnChoke_m.h"
#include "Interested_m.h"
#include "NotInterested_m.h"
#include "KeepAlive_m.h"
#include "Piece_m.h"
#include "Cancel_m.h"

#include "TorrentFile.h"
#include "PieceManager.h"
#include "PeerManager.h"

#define Default 0
#define Tracker_Request 1
#define Tracker_Response 2
#define HandShake 3
#define BITField 4
#define Timer_Message 5
#define Have_Piece 6
#define Piece_Request 7
#define New_Piece 8
#define Cancel_Piece 9
#define KeepAlive_Timer 10
#define UnChoke_Peer 11
#define Choke_Peer 12
#define Interested_Peer 13
#define UnInterested_Peer 14
#define KeepAlive_Peer 15
#define Leaving_Swarm 16
#define Transmission_Ready 17


#define HandShake_Length 28
#define MessageHeader_Length 5
#define Request_Lenght 12
#define Have_Length 4
#define PieceHeader_Length 8

#define CLOSED 0
#define PENDING 1
#define OPEN 2

using namespace std;

/*!
    \brief The BitTorrent class defines basic operations of a module in a BT swarm.
    \date 2009-03-06
    \version 1.2
    \author Blekinge Institute of Technology TEK-ATS
*/
class BitTorrent : public cSimpleModule{
	protected:
        /*!
          \brief Pointer to the torrentFile module.
          */
		TorrentFile *torrentFile;
        /*!
          \brief Returns the amount of additional connections that can be established.
          */
		int getAmountOfEmptyGates();
        /*!
          \brief Defines the URL of the module.
          */
		string URL;
        /*!
          \brief Defines the Peer ID of the module.
          */
		string peer_id;

	public:
        /*!
          \brief Free the gate that is conencted to \e client..

            \param peer The identifier of the client.
          */
		virtual void freeGate(const char *peer) = 0;
        /*!
          \brief Returns a gate to connect \e peer to.

            \param peer The identifier of the client.
            \return The gate number where \e peer can be connected to.
            \retval >= 0 Gate number
            \retval -1 If no gate is available
          */
		int getFreeGate();
	/*!
          \brief Returns the Peer Id of the module.

	Only used by trackers.

            \return Peer ID.
	*/
		string getPeerID();
	/*!
          \brief Returns the URL of the module.

	Only used by BT clients not by the tracker.

            \return Peer ID.
	*/
		string getURL();
		
};

#endif
