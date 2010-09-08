#ifndef _PieceManager_H_
#define _PieceManager_H_

/*
 * Defines a piece managment class.
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
#include <string>

#include "BitField_m.h"
#include "PieceRequest_m.h"

using namespace std;

/*!
	\brief Defines the parameters of a block as part of a piece.
*/
struct blockID{
	/*!
		\brief The sequence number of the piece.
	*/
	int index;
	/*!
		\brief The sequence number of the block.
	*/
	int offset;
	/*!
		\brief The length of the block.
	*/
	int length;
};

/*!
	\brief Defines the pieces that a peer has.
*/
struct pieceOfPeer{
	/*!
		\brief The ID of the peer.
	*/
	string peer_id;
	/*!
		\brief Defines if the client owning this pieceOfPeer is intereted in this peer or not.
	*/
	bool interested;
	/*!
		\brief Defines if the client owning this pieceOfPeer is choked by this peer or not.
	*/
	bool choked;
	/*!
		\brief Pointer to an array of bits telling which pieces this peer owns.

		Every bit represents one piece.
	*/
	unsigned char *piece;
};

/*!
	\brief Defines an incomplete piece.
*/
struct incompletePiece{
	/*!
		\brief The ID of the peer from which this piece is being download.

		If this piece is assigned to a peer, then the peer ID is stored. If this piece is
		is not assigned then the peer ID is set to "XXX".
	*/
	string peer_id;
	/*!
		\brief The number of the piece.
	*/
	unsigned int piece;
	/*!
		\brief Pointer to an array of bits representing the blocks of \e piece that are downloaded.

		Every bit represents one block.
	*/
	unsigned char *block;
};


/*!
    \brief The PieceManager class manages all the information concerning pieces and blocks for a BT client.
    \date 2009-03-06
    \version 1.2
    \author Blekinge Institute of Technology TEK-ATS
*/
class PieceManager{
	private :
	/*!
		\brief Defines if the client owning this class is a seeder or not.
	*/
		bool seeder;
	/*!
		\brief Defines the piece selection algorithm used.
	*/
		int algorithm;
	/*!
		\brief Pointer to an array of bits representing the pieces that this client owns.

		Every bit represents one piece.
	*/
		unsigned char *myPieces;
	/*!
		\brief Pointer to an array which stores how many duplicates of a piece is present in the swarm.

		The piecenumber must be used to index this array.
	*/
		short *rarestPieceIndex;
	/*!
		\brief Defines the name of the client owning this class.

		This is only used in case debug messages want to be printed.
	*/
		string name;

	/*!
		\brief Defines the length of a piece.
	*/
		int pieceLength;
	/*!
		\brief Defines the length of a block.
	*/
		int blockLength;
	/*!
		\brief Defines the length of the last block.
	*/
		int lastBlockLength;
	/*!
		\brief Defines the length of the last piece.
	*/
		int lastPieceLength;
	/*!
		\brief Defines the length of the file being downloaded.
	*/
		double lengthOfFile;

	/*!
		\brief Defines the amount of pieces in which the file is divided.
	*/
		int amountOfPieces;
	/*!
		\brief Defines the amount of blocks in which a peice is divided.
	*/
		int amountOfBlocks;

	/*!
		\brief Defines the length of the piece array.
	*/
		int lengthOfPieceArray;
	/*!
		\brief Defines the length of the block array.
	*/
		int lengthOfBlockArray;

		int last_piece;
		unsigned int completed_pieces;

	/*!
		\brief Pointer to an array storing piece selection information.
	*/
		unsigned char *power;
	/*!
		\brief Array containing information about incomplete pieces.
	*/
		vector<incompletePiece> incompletePieces;
	/*!
		\brief Array containing piece information of peers.
	*/
		vector<pieceOfPeer> piecesOfPeers;

	/*!
		\brief Map that maps seeder/leecher state upon peers.
	*/
		std::map<string, bool> completedPeers;

	/*!
		\brief Returns pointer to in incomplete piece.
		\retval vector.end() If the piece was not found.
	*/
		inline vector<incompletePiece>::iterator incPieceLookUp(unsigned int pieceLU);
	/*!
		\brief Returns pointer to the piece information of \e peer.
		\retval vector.end() If no information of peer could be found.
	*/
		inline vector<pieceOfPeer>::iterator pieceOfPeerLookUp(const char *peer);

	/*!
		\brief Adds a piece to the local piece set.
	*/		
		void addPiece(unsigned int stuk);
	/*!
		\brief Checks if the passed piece array is complete.
	*/	
		int getPieceArrayComplete(unsigned char *piece);
	/*!
		\brief Checks if the passed block array is complete.
	*/	
		int getBlockArrayComplete(unsigned char *block);
	/*!
		\brief Pushes Back all piece numbers from pieceList in the vector.
	*/	
		void getCompressedPieceList(unsigned char *pieceList, vector<int> *compressedPieceList);

	/*!
		\brief Returns the first empty block of an incomplete piece.
	*/	
		unsigned int getNextEmptyBlock(vector<incompletePiece>::iterator it);
	/*!
		\brief Returns if all block are present in incomplete piece \e it.
	*/	
		unsigned int isLastBlock(vector<incompletePiece>::iterator it);
	/*!
		\brief Gets a random piece from the piece set of peer.
	*/	
		unsigned int getRandomPiece(const char *peer);
	/*!
		\brief Checks wether a block is present in block array.
	*/	
		int getBlockPresent(unsigned char *it, unsigned int block);
	/*!
		\brief  Throws all the pieces out of \e rarestSet that this client has
		, which are not in \e pieceList and which are in process of downloading
	*/
		void throwOutOfPieces(unsigned char *pieceList, unsigned char *rarestSet);
	/*!
		\brief Sets every incomplete piece of peer to open to other peers.
	*/	
		void freeIncompletePiece(const char* peer);

	/*!
		\brief Implementing the random first policy.
		\param blockId pointer to a blockID in wich the selected piece will be written
	*/	
		void randomFirstPolicy(const char *peer, blockID *blockId);
	/*!
		\brief Implementing the strict priority policy.
		\param blockId pointer to a blockID in wich the selected piece will be written
	*/	
		void strictPriorityPolicy(const char *peer, blockID *blockId);
	/*!
		\brief Implementing the rarest first policy.
		\param blockId pointer to a blockID in wich the selected piece will be written
	*/	
		void rarestFirstPolicy(const char *peer, blockID *blockId);

	/*!
		\brief Adds an incomplete piece to incompletePieces vector.
		\param peer The peer from which this piece will be downloaded
		\param index The piece that will be downloaded
	*/
		void addIncompletePiece(const char *peer, unsigned int index);
	/*!
		\brief Implements the simple linear piece selection policy.
	*/
		void algLinear(const char *peer, blockID *blockId);
		
		/*
			TODO Add remaining piecde selection algorithms.
				1) hybrid data-sensitive, with policies
					a: probabilistic
					b: bandwidth dependent
					c: deadline-based
				2) pure smoothing?
		*/
		
	
	public :
		PieceManager();
		~PieceManager();

	/*!
		\brief Defines the amount of time that \e myPieceArray was accesed.
	*/
		int acces;
		
	/*!
		\brief Notifies the PieceManager class that the owner of this class is a seeder.
	*/
		void setSeeder();
	/*!
		\brief Initialises this class, must first be called for proper functioning.
	*/
		void init(double lengthFile, double pieceLengthIn, int algorithmIn);
	/*!
		\brief Sets the length of the blocks, must first be called for proper functioning.
	*/
		void setBlockLength(int blockSize);
	/*!
		\brief Adds a block of a piece to the incomplete piece set.
		\return Wheter \e peer was found in the database
		\retval 0 Block was added.
		\retval -1 Block was not added.
	*/ 
		int setBlock(unsigned int piece, unsigned int block);
	/*!
		\brief Adds a piece in the piece set of \e peer.
	*/
		void addPiece(unsigned int piece, const char *peer);
	/*!
		\brief Checks whether piece \e piece is present.
		\retval 0 present
		\retval -1 not present
	*/
		int getPiecePresent(unsigned int piece);
	/*!
		\brief Checks whether \e peer has \e piece.
		\retval 0 present
		\retval -1 not present
	*/
		int getPiecePresentOfPeer(unsigned int piece, const char *peer);
	/*!
		\brief Checks if the myPieceSet is complete.
		\retval 0 complete
		\retval -1 incomplete
	*/
		int getPieceArrayComplete();
	/*!
		\brief Returns the amount of pieces that are present in myPieceSet.
	*/
		int getAmountOfPieces();
	/*!
		\brief Returns the amount of pieces that are present in \e myPieceArray.
	*/
		int getAmountOfCompletePieces();
	/*!
		\brief Returns the amount of pieces in progress of downloading.
	*/
		int getAmountOfPiecesInProgress();
	/*!
		\brief Returns the length of the piece array (in bytes).
	*/
		int getLengthOfPieceArray();
	/*!
		\brief Returns the length of the file that is being downloaded (in bytes).
	*/
		double getLengthOfFile(){return lengthOfFile;};

	/*!
		\brief Fills \e myPieceArray with and uniform distribution.

			The left bound is zero.
		\param distri Is the right bound of the uniform distribution.
	*/
		void initialisePieceArray(double distri);
	/*!
		\brief Checks whether the peice array of \e peer is complete.
		\param peer The peer to be checked
		\retval 0 If array of \e peer is complete
		\retval -1 If array of \e peer is not complete 
	*/
		int getPieceArrayCompleteOfPeer(const char *peer);
	/*!
		\brief Checks wether \e block is present in the block array of an incomplete piece being downloaded from peer.
		\param piece The piece to be checked
		\param block The block to be checked
		\param peer The peer to be checked
		\retval 0 Block is present
		\retval -1 Block is not present
	*/
		int getBlockPresentOfIncompletePeer(unsigned int piece, unsigned int block, const char *peer);
	/*!
		\brief Returns all the blocks from incompletePieces that are not yet downloaded.
		\param blocks A pointer to a vector on which all the IDs of the blocks will be pushed back.
	*/
		void getLastBlocks(vector<blockID> *blocks);
	/*!
		\brief Returns a list of peers that currently is being downloaded from.
		\param peers Pointer to a vector on which all the ID of the peers will be pushed.
	*/
		void getDownloadListOfPeers(vector<string> *peers);
	/*!
		\brief Returns the ID of a block that can be downloaded from a peer.

			This function calls the proper piece selection algorithms set in the initialisation file.
	*/
		blockID pieceRequest(const char *peer);

	/*!
		\brief Returns the rarest piece set.
		\param rarestSet Pointer to an array in which the rarest pieces will be stored.
		\param level The order of the rarest pieces to be requested. For example level 0 are the rarest pieces,
				level 1 are the second rarest pieces and so on.
	*/
		void getRarestPieceSet(unsigned char *rarestSet, int level);
	/*!
		\brief Recalculates the \e rarestPieceIndex array.
	*/ 
		void buildPieceIndex();

	/*!
		\brief Adds a new entry for \e peer to the piecesOfPeer array.
	*/
		void addPieceArrayOfPeer(BitField *msg);
	/*!
		\brief Returns if the piece array of \e peer is empty.
		\retval 0 piece set is empty
		\retval -1 piece set is not empty
	*/
		int getPieceArrayEmpty(const char *peer);
	/*!
		\brief Returns if the piece array of \e peer is complete.
		\retval 0 piece set complete
		\retval -1 piece set incomplete
	*/
		int getPieceArrayComplete(const char *peer);
	/*!
		\brief Copies the content of \e myPieceSet in the bitfield message.
	*/
		void writePiecesToMessage(BitField *msg);

	/*!
		\brief Deletes every track of \e peer.

		Incomplete pieces will be freed and all user information will be deleted from the database.
	*/	
		void freePeer(const char *peer);
	/*!
		\brief Sets if the owner is interested in \e peer.
	*/
		void setInterestedInPeer(const char *peer, bool interest);
	/*!
		\brief Returns if the owner is interested in \e peer.
		\retval 0 interested
		\retval -1 not interested
		\retval -2 \e peer not found
	*/
		int getInterestedInPeer(const char *peer);
	/*!
		\brief Sets if the owner is choked by \e peer.
		\param peer The peer of which the choke value should be set
		\param choked choked or unchoked by that \e peer.
	*/
		void setChokedByPeer(const char *peer, bool choked);
	/*!
		\brief Returns if the owner is choked by \e peer.
		\retval 0 choked
		\retval -1 unchoked
	*/
		int getChokedByPeer(const char *peer);
	/*!
		\brief Returns if there is something to be interested in from \e peer.
		\param 0 There are pieces to be interested in.
		\param -1 There are no pieces to be interested in.
	*/
		int isThereSomethingToBeInterestedIn(const char *peer);
	/*!
		\brief Returns the amount of bytes that is still to be downloaded.
	*/
		unsigned int getRemainingDownload();

	/*!
		\brief Displays the piece set to cout.
	*/
		void displayPiece();
	/*!
		\brief Displays the piece set of \e peer to cout.
	*/
		void displayPiece(const char *peer);
	/*!
		\brief Displays the incomplete piece set to cout.
	*/
		void displayBlock();
	/*!
		\brief Displays the peers of which a piece list is avialable to cout.
	*/
		void displayPeers();

	/*!
		\brief Sets the name of the owner of this class.
	*/	
		void setPeerID(string naam);
	
	/*!
		\brief Removes every enry in the incomplete piece set.
	*/
		void deleteIncompletePieces();
};

#endif
