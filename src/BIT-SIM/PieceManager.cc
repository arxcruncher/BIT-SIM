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

#include <ctype.h>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <assert.h>

#include "PieceManager.h"

using namespace std;


void PieceManager::init(double lengthFile, double pieceLengthIn, int algorithmIn){
	// The firest two arguments are bytes, convert to bits
	algorithm = algorithmIn;
	pieceLength = (int)(pieceLengthIn*8);
	lengthOfFile = lengthFile*8;
	amountOfPieces = (int)(lengthFile/pieceLengthIn);
	if (((lengthFile/pieceLengthIn)-amountOfPieces) > 0) amountOfPieces++;

	lengthOfPieceArray = amountOfPieces/8;
	if (amountOfPieces%8 > 0) lengthOfPieceArray++;

	double rest = lengthFile/pieceLengthIn;
	int restInt = (int)rest;
	lastPieceLength = (int)((rest-(double)restInt)*(double)pieceLength);

	myPieces = new unsigned char[lengthOfPieceArray];
	rarestPieceIndex = new short[amountOfPieces];

	for(int i = 0; i < lengthOfPieceArray; i++) myPieces[i]=0;
	for(int i = 0; i < amountOfPieces; i++) rarestPieceIndex[i]=0;

	// cout << "PieceLengt: " << pieceLength << ", amountOfPiece " << amountOfPieces << ", lengthOfPieceArray: " << lengthOfPieceArray << endl;
}

int PieceManager::getPieceArrayCompleteOfPeer(const char *peer){
	int sum = 0;
	std::map<string, bool>::iterator pos = completedPeers.find(string(peer));
	if(pos != completedPeers.end() && pos->second == true)
		return 0;
	
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
		
	if(it != piecesOfPeers.end()){
		for(int i = 0; i < amountOfPieces ; i++ ){
			if((it->piece[i>>3] & power[i%8]) > 0) sum++;
		}
		if(sum == amountOfPieces) {
			// Peer is now a seed, make a note of this.
			completedPeers.insert(make_pair(string(peer),true));
			return 0;
		}
	}
	return -1;
}

int PieceManager::getPiecePresentOfPeer(unsigned int piece, const char *peer){
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	//assert(piece < amountOfPieces);
	//cout << "Piece of peer present? " << piece << ", peer " << peer << " : " << (int)(it->piece[piece>>3] & power[piece&7]) << endl;
	if(it != piecesOfPeers.end() ) {if((it->piece[piece>>3] & power[piece&7]) > 0) return 0;}
	return -1;
}

void PieceManager::setBlockLength(int blockSize){
	if(lengthOfBlockArray == 0){
		// blockSize is specified in bytes, convert to bits
		blockLength = blockSize*8;
		
		amountOfBlocks = pieceLength/blockLength;
		if( pieceLength%blockLength > 0 ) amountOfBlocks++;
		
		lengthOfBlockArray = amountOfBlocks/8;
		if (amountOfBlocks%8 > 0) lengthOfBlockArray++;
		
		lastBlockLength = pieceLength%blockLength;
		
		if(name.compare("PEERID=0") == 0) cout << "BlockLengt: " << blockLength << ", amountOfBlocks: " << amountOfBlocks << ", lengthOfBlockArray: " << lengthOfBlockArray << endl;
	}
}

PieceManager::PieceManager(){
	lengthOfBlockArray = 0;
	lengthOfFile = 0;
	pieceLength = 0;
	amountOfPieces = 0;
	amountOfBlocks= 0;
	lastPieceLength = 0;
	lastBlockLength = 0;
	acces = 0;
	seeder = false;
	myPieces = NULL;
	
	completed_pieces = 0;
	
	last_piece = -1;
	
	// Look-up table for, this is faster than calculating every time the values
	power = new unsigned char[8];
	power[7]=1;  power[6]=2;  power[5]=4;  power[4]=8;
	power[3]=16; power[2]=32; power[1]=64; power[0]=128;
}

PieceManager::~PieceManager(){
	//delete piece;
}

void PieceManager::writePiecesToMessage(BitField *msg){
	msg->setPieceArraySize(lengthOfPieceArray);
	for(unsigned int index = 0; index < msg->getPieceArraySize(); index++) msg->setPiece(index, myPieces[index]);
}

int PieceManager::getBlockPresentOfIncompletePeer(unsigned int piece, unsigned int block, const char *peer){
	// If there is an empty incomplete block a request for the first block has been send, thus an additional request 
	// for that block to peer is not nessecary
	for(vector<incompletePiece>::iterator it = incompletePieces.begin() ; it != incompletePieces.end(); it++){
		if(it->peer_id.compare(peer) == 0){
			if(piece == it->piece){
				if(it->block[0] == 0 && block == 0) return 0;
				if(getBlockPresent(it->block, block) == 0) return 0;
				return -1;
			}
		}
	}
	return -1;
}

void PieceManager::deleteIncompletePieces(){
	for(vector<incompletePiece>::iterator it = incompletePieces.begin(); it != incompletePieces.end(); it++){
		delete[] it->block;
	}
	incompletePieces.clear();
}

int PieceManager::setBlock(unsigned int stuk, unsigned int block){
	vector<incompletePiece>::iterator it = incPieceLookUp(stuk);
	if(it != incompletePieces.end()){
		if( getBlockPresent(it->block, block) == -1 ) it->block[block>>3] |= power[block&7];
		//If piece is complete, remove from vector and add it to the complete-piece array
		if(getBlockArrayComplete(it->block) == 0){
			delete[] it->block;
			incompletePieces.erase(it);
			addPiece(stuk);
			return 1;
		}
	}
	return 0;
}

int PieceManager::getBlockArrayComplete(unsigned char *block){
	int som = 0;
	for(int i = 0; i < amountOfBlocks ; i++){
		if((block[i/8] & power[i&7]) > 0) som++;
	}
	if( som == amountOfBlocks ) return 0;
	else return -1;
}

int PieceManager::getPieceArrayComplete(unsigned char *block){
	int som = 0;
	for(int i = 0; i < amountOfPieces ; i++){
		if((block[i>>3] & power[i&7]) > 0) som++;
	}
	if( som == amountOfPieces ) return 0;
	else return -1;
}

int PieceManager::getPieceArrayComplete(){
	return (completed_pieces == amountOfPieces) ? 0 : -1;
}

int PieceManager::getPiecePresent(unsigned int piece){
	//assert(piece/8 < amountOfPieces);
	if((myPieces[piece>>3] & power[piece&7]) > 0) return 0;
	return -1;
}

int PieceManager::getChokedByPeer(const char *peer){
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	if(it == piecesOfPeers.end()) return -2;
	if(it->choked) return 0;
	return -1;
}

int PieceManager::getInterestedInPeer(const char *peer){
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	if(it == piecesOfPeers.end()) return -2;
	if(it->interested) return 0;
	return -1;
}

int PieceManager::isThereSomethingToBeInterestedIn(const char *peer){
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	if(it == piecesOfPeers.end()) {displayPeers();cout << "Peer " << peer << endl;}
	unsigned char mask = 0;
	for(int i = 0; i < lengthOfPieceArray; i++) mask |= (~myPieces[i] & it->piece[i]);
	if(mask > 0) return 0;
	return -1;
}

void PieceManager::addPiece(unsigned int stuk){
	if ((myPieces[stuk>>3] & power[stuk&7]) > 0) cout << name << " adding already existing piece " << stuk << endl;
	else completed_pieces++;
	myPieces[stuk>>3] |= power[stuk&7];
	acces++;
}

void PieceManager::addPiece(unsigned int stuk, const char *peer){
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	it->piece[stuk>>3] |= power[stuk&7];
	rarestPieceIndex[stuk]++;
}

int PieceManager::getBlockPresent(unsigned char *it, unsigned int block){
	//assert(block/8 < lengthOfBlockArray);
	if((it[block>>3] & power[block&7]) > 0) return 0;
	else return -1;
}

inline vector<incompletePiece>::iterator PieceManager::incPieceLookUp(unsigned int pieceLU){
	vector<incompletePiece>::iterator it = incompletePieces.begin();
	while(it != incompletePieces.end()){
		if( it->piece == pieceLU ) break;
		it++;
	}
	return it;
}

inline vector<pieceOfPeer>::iterator PieceManager::pieceOfPeerLookUp(const char *peer){
	vector<pieceOfPeer>::iterator it = piecesOfPeers.begin();
	while(it != piecesOfPeers.end()){
		if( it->peer_id.compare(string(peer)) == 0 ) break;
		it++;
	}
	return it;
}

void PieceManager::getDownloadListOfPeers(vector<string> *peers){
	for(vector<pieceOfPeer>::iterator it = piecesOfPeers.begin(); it != piecesOfPeers.end() ;it++){
		if(!it->choked) peers->push_back(it->peer_id);
	}
}

void PieceManager::addPieceArrayOfPeer(BitField *msg){
	pieceOfPeer newP;
	newP.peer_id = msg->getPeer_id();
	newP.interested = false;
	newP.choked = true;
	newP.piece = new unsigned char[msg->getPieceArraySize()];
	for(unsigned int i = 0; i < msg->getPieceArraySize(); i++){
		newP.piece[i] = msg->getPiece(i);
	}
	piecesOfPeers.push_back(newP);
}

void PieceManager::displayPiece(){
	char temp[3];
	cout << endl << "------------------- pieceManager -- Piece: " << amountOfPieces << " -- blocks: " << amountOfBlocks << " -------------";
	if(myPieces != NULL){
		int i = 0;		
		for(; i < lengthOfPieceArray; i++){
			if(i%16 == 0) cout << endl << "Piece " << i*8 << ":\t";
			if(i%8 == 0) cout << "   ";
			snprintf(temp, 3, "%02X" ,myPieces[i] );
			cout << temp << " ";
		}
	}
	else cout << endl << "No myPiece space assigned." << endl;
	cout << endl << "---------------------------- pieceManager END ----------------------------" << endl << endl;
}

void PieceManager::displayPiece(const char *peer){
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	if(it != piecesOfPeers.end()){
		char temp[3];
		cout << endl << "------------------- pieceManager -- Piece: " << amountOfPieces << " -- peer: " << peer << " -------------";
		for(int i = 0; i < lengthOfPieceArray; i++){
			if(i%16 == 0) cout << endl << "Piece " << i*8 << ":\t";
			if(i%8 == 0) cout << "   ";
			snprintf(temp, 3, "%02X" ,it->piece[i] );
			cout << temp << " ";
		}
		cout << endl << "---------------------------- pieceManager END ----------------------------" << endl << endl;
	}
}

void PieceManager::displayPeers(){
	cout << endl << "------------------- pieceManager -- Peers: " << amountOfPieces << " -------------";
	for(vector<pieceOfPeer>::iterator it = piecesOfPeers.begin(); it != piecesOfPeers.end(); it++){
		cout << endl << "Peer " << it->peer_id << "\tchoked: " << it->choked << "\tinterested: " << it->interested;
	}
	cout << endl << "---------------------------- pieceManager END ----------------------------" << endl << endl;
}

void PieceManager::displayBlock(){
	char temp[3];
	cout << endl << "------------------- blockManager -- Piece: " << amountOfPieces << " -- blocks: " << amountOfBlocks << " -------------" << endl;
	for(vector<incompletePiece>::iterator it = incompletePieces.begin(); it != incompletePieces.end(); it++){
		cout << "Piece " << it->piece << " peer " << it->peer_id << ": ";
		for(int j = 0; j < lengthOfBlockArray; j++){
			if(j&7 == 0) cout << "  ";
			snprintf(temp, 3, "%02X" ,it->block[j] );
			cout << temp << " ";
		}
		cout << endl;
	}
	cout << "---------------------------- blockManager END ----------------------------" << endl << endl;
}

void PieceManager::getRarestPieceSet(unsigned char* rarestSet, int level){
	short rarest = SHRT_MAX;
	short bottum = 0;
	while(level >= 0){
		rarest = SHRT_MAX;
		for(int i = 0; i< amountOfPieces ;i++){
			if(rarestPieceIndex[i] < rarest && rarestPieceIndex[i] > bottum){
				rarest = rarestPieceIndex[i];
			}
		}
		bottum = rarest;
		level--;
	}
	for(int i = 0; i< amountOfPieces ;i++){
		if(rarestPieceIndex[i] == rarest){
			rarestSet[i>>3] |= power[i&7];
		}
	}
}

void PieceManager::buildPieceIndex(){
	//Reset existing rarestPiece array
	for(int j = 0; j<amountOfPieces; j++){
		rarestPieceIndex[j] = 0;
	}
	//Build new rarestPiece array
	for(vector<pieceOfPeer>::iterator it = piecesOfPeers.begin(); it!=piecesOfPeers.end() ;it++){
		for(int j = 0; j < amountOfPieces; j++){
			if((it->piece[j>>3] & power[j&7]) > 0 ) rarestPieceIndex[j]++;
		}
	}
}

void PieceManager::setSeeder(){
	seeder = true;
}

int PieceManager::getAmountOfCompletePieces(){
	return completed_pieces;
}

int PieceManager::getAmountOfPiecesInProgress(){
	return incompletePieces.size();
}

int PieceManager::getAmountOfPieces(){
	return amountOfPieces;
}

void PieceManager::getLastBlocks(vector<blockID> *blocks){
	for(vector<incompletePiece>::iterator it = incompletePieces.begin(); it != incompletePieces.end(); it++){
		for(int k = 0; k < amountOfBlocks; k++){
			if(getBlockPresent(it->block, k)){
				blockID temp;
				temp.index = it->piece;
				temp.offset = k;
				if(k == (amountOfBlocks -1)) temp.length = lastBlockLength;
				else temp.length = blockLength;
				blocks->push_back(temp);
			}
		}
	}
}

unsigned int PieceManager::getNextEmptyBlock(vector<incompletePiece>::iterator it){
	int i = 0;
	while(i < amountOfBlocks){
		if((it->block[i>>3] & power[i&7]) == 0) return i;
		i++;
	}
	cout << endl << "  Error: block index out of bound: " << i/8 << endl;
	abort();
	return -1;
}

void PieceManager::getCompressedPieceList(unsigned char *pieceList, vector<int> *compressedPieceList){
	for(int i = 0; i < amountOfPieces; i++){
		if((pieceList[i>>3] & power[i&7]) > 0) compressedPieceList->push_back(i);
	}
}

unsigned int PieceManager::isLastBlock(vector<incompletePiece>::iterator it){
	int i = 0;
	bool found = false;
	while(!found && i < amountOfBlocks){
		if((it->block[i>>3] & power[i&7]) == 0) found = true;
		i++;
	}
	if(i == amountOfBlocks) return 0;
	return -1;
}

unsigned int PieceManager::getRandomPiece(const char *peer){
	vector<pieceOfPeer>::iterator itPeer = pieceOfPeerLookUp(peer);
	vector<int> compressedPieceList;
	
	if(itPeer != piecesOfPeers.end()){
		unsigned char temp[lengthOfPieceArray];
		for(int i = 0; i < lengthOfPieceArray; i++) temp[i] = ~0;
		throwOutOfPieces(itPeer->piece, temp);
		getCompressedPieceList(temp, &compressedPieceList);
	}
	
	if(compressedPieceList.size() == 0) return -1;
	else return (unsigned int)compressedPieceList.at(intuniform(0, compressedPieceList.size()-1));
}

void PieceManager::strictPriorityPolicy(const char *peer, blockID *blockId){
	vector<unsigned int> abandonedPiece;
	
	for(vector<incompletePiece>::iterator wildCard = incompletePieces.begin(); 
		wildCard != incompletePieces.end(); wildCard++)
	{
		if(wildCard->peer_id.compare("XXX") == 0) 
			abandonedPiece.push_back(wildCard->piece);
	}
	
	if(abandonedPiece.size() > 0){
		for(unsigned int i = 0; i < abandonedPiece.size(); i++){
			if(getPiecePresentOfPeer(abandonedPiece.at(i), peer) == 0){
				vector<incompletePiece>::iterator it = incPieceLookUp(abandonedPiece.at(i));
				it->peer_id = peer;
				blockId->index = abandonedPiece.at(i);
				blockId->offset = getNextEmptyBlock(it);
				// assert(blockId->offset < lengthOfFile);
				if(isLastBlock(it) != 0) blockId->length = blockLength;
				else blockId->length = lastBlockLength;
				i = abandonedPiece.size();
			}
		}
	}
}


/** \brief algLinear

Implements the simple linear piece selection algorithm.

\author David Erman, TEK-ATS, BTH
\date 2007-10-23
\param  peer - peer to download piece from.
\param  blockId - piece do download. 
		fill in blockId->index   : the piece number
		        blockId->offset  : the block number
		        blockId->length  : the length of the block
\return void
\sa
**/

void PieceManager::algLinear(const char *peer, blockID *blockId) {
	
	// This method assumes that Strict Priority is performed outside, i.e., blocks
	// of partially downloaded pieces are already downloaded.
	if (last_piece < amountOfPieces) {
		blockId->index = ++last_piece;
		blockId->offset = 0;
		blockId->length = blockLength;

	// cerr << "requests piece " << last_piece << " from " << peer << endl;
		addIncompletePiece(peer,last_piece);
	} else {
		cerr << "algLinear: All pieces already requested! " << endl;
	}
}

/** \brief addIncompletePiece - add incomplete piece to incompletePieces vector.
	
	\author  TEK-ATS, BTH
	\date 2007-10-23
	\param  peer - peer to download piece from
	\param  index - piece number
	\return void
	\sa
**/
void PieceManager::addIncompletePiece(const char *peer, unsigned int index){
	incompletePiece newPiece;
	newPiece.peer_id = string(peer);
	newPiece.piece = index;
	newPiece.block = new unsigned char[amountOfBlocks];
	for(int k = 0; k < amountOfBlocks ;k++){
		newPiece.block[k] = 0;
	}

	incompletePieces.push_back(newPiece);
}

void PieceManager::randomFirstPolicy(const char *peer, blockID *blockId){
	int amount = getAmountOfCompletePieces();

	if(amount < 4){
		blockId->index = getRandomPiece(peer);
		blockId->offset = 0;
		blockId->length = blockLength;
		
		incompletePiece newPiece;
		newPiece.peer_id = string(peer);
		newPiece.piece = (unsigned int)blockId->index;
		newPiece.block = new unsigned char[amountOfBlocks];
		for(int k = 0; k < amountOfBlocks ;k++){
			newPiece.block[k] = 0;
		}
		incompletePieces.push_back(newPiece);
	}
}

void PieceManager::rarestFirstPolicy(const char *peer, blockID *blockId){
	vector<int> rarestSetC;
	int level = 0;
	vector<pieceOfPeer>::iterator itPeer = pieceOfPeerLookUp(peer);
	
	do {
		unsigned char rarestSet[lengthOfPieceArray];
		for(int i = 0; i < lengthOfPieceArray ; i++) rarestSet[i] = 0;
		getRarestPieceSet(rarestSet, level);
		level++;
		throwOutOfPieces(itPeer->piece, rarestSet);
		getCompressedPieceList(rarestSet, &rarestSetC);
	}
	//level < 15 is just to prevent the possibility of an infinite loop
	while(rarestSetC.size() == 0 && level < 15);
	if(rarestSetC.size() == 0){
		blockId->index = getRandomPiece(peer);
	}
	else{
		blockId->index = rarestSetC.at(intuniform(0, rarestSetC.size()-1));
	}
	
	if(blockId->index != -1){
		blockId->offset = 0;
		blockId->length = blockLength;
		incompletePiece newPiece;
		newPiece.peer_id = string(peer);
		newPiece.piece = (unsigned int)blockId->index;
		newPiece.block = new unsigned char[amountOfBlocks];
		for(int k = 0; k < amountOfBlocks ;k++){
			newPiece.block[k] = 0;
		}
		incompletePieces.push_back(newPiece);
	}
}

blockID PieceManager::pieceRequest(const char *peer){
	blockID blockId;
	blockId.index = -1;
	blockId.offset = -1;
	blockId.length = -1;
	
	//Look if a piece is being downloaded from that peer
	vector<incompletePiece>::iterator it = incompletePieces.begin();
	for(;it != incompletePieces.end(); it++){
		if(it->peer_id.compare(string(peer)) == 0) break;
	}
	
	if(it != incompletePieces.end()){
		//Download next block from peer
		blockId.index = it->piece;
		blockId.offset = getNextEmptyBlock(it);
		if(isLastBlock(it) != 0) 
			blockId.length = blockLength;
		else 
			blockId.length = lastBlockLength;
	}
	//Start to download next piece from peer
	else{
		switch(algorithm){
			case 1 : // Naive linear selection.
			if(blockId.index == -1) {
				strictPriorityPolicy(peer, &blockId);
			}
			if(blockId.index == -1) {
				algLinear(peer, &blockId);
			}
			break; 
			case 0 : // Default rarest-first.
			if(blockId.index == -1) strictPriorityPolicy(peer, &blockId);
			if(blockId.index == -1) randomFirstPolicy(peer, &blockId);
			if(blockId.index == -1) rarestFirstPolicy(peer, &blockId);
			break;
		}
	}
	return blockId;
}


//This class throws all the pieces out of rarestSet that this client has
//    and which are not in pieceList and which are in process of downloading
void PieceManager::throwOutOfPieces(unsigned char *pieceList, unsigned char *rarestSet){
	for(int i = 0; i < lengthOfPieceArray; i++){
		rarestSet[i] &= (~myPieces[i]);
		rarestSet[i] &= pieceList[i];
	}
 	
	for(vector<incompletePiece>::iterator it = incompletePieces.begin(); it != incompletePieces.end(); it++){
		//assert((it->piece)>>3 < lengthOfPieceArray);
 		rarestSet[(it->piece)>>3] &= ~power[(it->piece)&7];
 	}	
}

int PieceManager::getPieceArrayComplete(const char *peer){
	int sum = 0;
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	for(int i = 0; i < amountOfPieces ; i++ ){
		if((it->piece[i>>3] & power[i&7]) > 0) sum++;
	}
	if(sum == amountOfPieces) return 0;
	return -1;
}

int PieceManager::getPieceArrayEmpty(const char *peer){
	unsigned char sum = 0;
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	for(int i = 0; i < lengthOfPieceArray; i++ ){
		sum |= it->piece[i];
	}
	if(sum == 0) return 0;
	return -1;
}

void PieceManager::setInterestedInPeer(const char *peer, bool interest){
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	if(it != piecesOfPeers.end()){
		it->interested = interest;
	}
}

void PieceManager::setPeerID(string naam){
	name = naam;
}

void PieceManager::setChokedByPeer(const char *peer, bool choked){
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	if(it != piecesOfPeers.end()){
		it->choked = choked;
	}
	
	if(choked) freeIncompletePiece(peer);
}

void PieceManager::freeIncompletePiece(const char* peer){
	vector<incompletePiece>::iterator iter = incompletePieces.begin();
	while(iter != incompletePieces.end()){
		if(iter->peer_id.compare(string(peer)) == 0) iter->peer_id = "XXX";
		iter++;
	}
}

void PieceManager::freePeer(const char *peer){
	freeIncompletePiece(peer);
	vector<pieceOfPeer>::iterator it = pieceOfPeerLookUp(peer);
	if(it != piecesOfPeers.end()){
		delete[] it->piece;
		piecesOfPeers.erase(it);
	}
	buildPieceIndex();
}

void PieceManager::initialisePieceArray(double distri){
	int piecesToLoad = (int)(distri*amountOfPieces);
	int random = 0;
	while(piecesToLoad > 0){
		random = intuniform(0, (amountOfPieces-1));
		if(getPiecePresent((unsigned int)random) != 0){
			addPiece((unsigned int)random);
			piecesToLoad--;
		}
	}
}

int PieceManager::getLengthOfPieceArray(){
	return lengthOfPieceArray;
}

unsigned int PieceManager::getRemainingDownload(){
	unsigned int remain = 0;
	for(int i = 0; i < (amountOfPieces-1) ;i++){
		if(getPiecePresent(i) == -1) remain += pieceLength;
	}
	if(getPiecePresent(amountOfPieces-1) == -1) remain += lastPieceLength;
	return remain;
}

