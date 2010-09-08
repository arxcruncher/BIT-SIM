#ifndef _TorrentFile_H_
#define _TorrentFile_H_

/*
 * Defines read operations from a .torrent file.
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


//#include "bencode.h"
#include <ctype.h>
#include <map>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

//TODO: this class should completely be rewritten!!!
//      It is hidious! And cannot just read any torrent.

class TorrentFile : public cSimpleModule {
	protected:
		int getNext(ifstream &inFile, ostringstream &entry);
		map<string,string> torrent;
	
		string fileName;
		string announce;
	
		int lengthOfFile;
		double lengthOfFile_double;
		int piece_length;
		int lengthOfPieces;
		
		int amountOfPieces;
		int amountOfBlocks;

	public:
		TorrentFile();
		void initialize();
		string getValue(const char *key);
		void getInt(ifstream &inFile, ostringstream &entry);
		void getString(ifstream &inFile, ostringstream &entry);		
};

#endif
