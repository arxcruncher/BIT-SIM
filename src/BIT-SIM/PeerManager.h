#ifndef _PeerManager_H_
#define _PeerManager_H_

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

#include <omnetpp.h>

#include <sstream>
#include <vector>
#include <string>

#define CLOSED 0
#define PENDING 1
#define OPEN 2

using namespace std;

/*!
    \brief Unifies information of other clients in the swarm.
*/
struct Peer{
	/*!
		\brief Peer ID of the client, owning the other fields in this struct.
	*/
	string peer_id;
	/*!
		\brief The time when the choked value was last changed.
	*/
	simtime_t time;
	/*!
		\brief Defines whether \e peer_id is interested in the PeerManager class.
	*/
	bool interested;
	/*!
		\brief Defines whether \e peer_id is choked by the peer owning the PeerManager class.
	*/
	bool choked;
	/*!
		\brief Defines wheter the \e peer_id is a seeder or not.
	*/
	bool seeder;
	/*!
		\brief Defines the status of the connection.

		0 = conenction closed, 1 = conenction pending, 2 = conenction established.
	*/
	int status;
	/*!
		\brief Tells when the connection status was last changed.
	*/
	simtime_t statusChange;
	/*!
		\brief Defines the amount of bytes that were downloaded since the last time reference.
	*/
	int downloaded;
	/*!
		\brief Defines the gate at which \e peer_id is connected to.
	*/
	int gate;
};

/*!
    \brief The PeerManager class manages client infornation for a BT client.
    \date 2009-03-06
    \version 1.2
    \author Blekinge Institute of Technology TEK-ATS
*/
class PeerManager{
	private :
		/*!
			\brief Defines the Peer ID of the peer that owns this class.
		*/
		string peer_id;
		/*!
			\brief Array of peer information of which \e peer_id has notice of.
		*/
		vector<Peer> peers;
		/*!
			\brief Looks up the pointer to a \e Peer struct containing information of \e peer.
		*/
		vector<Peer>::iterator peerLookUp(const char *peer);
	
	public :
		/*!
			\brief Returns whether \e peer_id is choked by this client.
		*/
		int  getStatus(const char *peer);
		/*!
			\brief Sets the conenction to \e peer to established.
		*/
		void conOpenend(const char *peer, simtime_t time);
		/*!
			\brief Sets the conenction to \e peer to pending.
		*/
		void conPending(const char *peer, simtime_t time);
		
		/*!
			\brief Adds an empty entry for \e peer in the database.
		*/
		void addPeer(const char *peer);
		/*!
			\brief Returns if \e peer has an entry in the database.
			\retval 0 \e peer is present.
			\retval -1 \e peer is not present
		*/
		int  registredPeer(const char *peer);
		/*!
			\brief Returns whether \e peer is choked by this client.
			\retval 0 \e peer is unchoked.
			\retval -1 \e peer is choked
		*/
		int  getPeerChoked(const char *peer);
		/*!
			\brief Sets the status of \e peer to seeder in the database.
		*/
		void setPeerAsSeeder(const char *peer);
		/*!
			\brief Returns the seeder/leecher status of \e peer.
			\retval true \e peer is a seeder
			\retval false \e peer is a leecher
			\retval -1 \e peer is not found in the database
		*/
		int  isPeerASeeder(const char *peer);
		/*!
			\brief Returns the amount of established conenctions.
		*/
		int  getAmountOfConnections();
		/*!
			\brief Pushes back all peers that are not connected in the \e peer vector.
		*/
		void getNewPeers(vector<string> *peer);
		/*!
			\brief Pushes back all peers that can be found in the database in the \e peerList vector.
		*/
		void getPeerList(vector<string> *peerList);
		/*!
			\brief Deletes the entry in the database correspending to \e peer.
		*/
		void deletePeer(const char *peer);
		/*!
			\brief Sets all the PENDING connections to CLOSED that are pending for more then 1 minute.
		*/
		int resetPendingPeers(simtime_t time);
		/*!
			\brief Returns the gate to which \e peer is conencted to.
		*/
		int getGate(const char *peer);
		/*!
			\brief Sets the gate to which \e peer is conencted to.
		*/
		void setGate(const char *peer, int gate);

		/*!
			\brief Sets the download tracks off every peer entry to 0.
		*/
		void resetDownload();
		/*!
			\brief Adds the download to the \e peers information space.
		*/
		void registerDownload(const char *peer, int byte);
		/*!
			\brief Pushes back the three fastest downloaders in the \e peerList vector.
		*/
		void getFastestDownloader(vector<string> *peerList);
		/*!
			\brief Pushes back the three most recently unchoked peers in the \e peerList vector.
		*/
		void getRecentlyUnchoked(vector<string> *peerList);
		/*!
			\brief Sets the choke state of \e peer and records the timestamp.
		*/
		void setChokePeer(const char *peer, bool choke, simtime_t time);
		/*!
			\brief Pushes back all the peers in the \e choked vector that are choked and not in the \e unChoked vector.
		*/
		void isPeerChoked(vector<string> *unChoked, vector<string> *choked);
		/*!
			\brief Returns a random interested and choked peer.
		*/
		const char* getRandomInterested();
		/*!
			\brief Returns a random choked and interested peer.
		*/
		const char* getRandomChoked();
		/*!
			\brief Sets the interested parameter of \e peer.
			\param interest the state to which \e peer will be set
		*/
		void setPeerInterested(const char* peer, bool interest);
		/*!
			\brief Returns if there are interested peers or not.
			\retval 0 There are interested peers
			\retval -1 There are no interested peers
		*/
		int  getIntrests();
		/*!
			\brief Returns if there are choked peers or not.
			\retval 0 There are choked peers
			\retval -1 There are no choked peers
		*/
		int  getChoked();

		/*!
			\brief Registers the ID of the peer that owns this class.

			This is only used in case debug messages want to be printed.
		*/
		void setPeerID(string name);

		/*!
			\brief Writes the content of the peer database to \e cout.
		*/
		void writePeers();
		/*!
			\brief Destroys this PeerManager.
		*/
		~PeerManager();
};

#endif
