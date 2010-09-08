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

#include "BitTorrent.h"

#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

//Define_Module(BitTorrent);

string BitTorrent::getURL(){ return URL; }

string BitTorrent::getPeerID(){ return peer_id; }

int BitTorrent::getAmountOfEmptyGates(){
	int empty = 0;
	for(int i = 0; i < gateSize("out"); i++){
		if(gate("out", i)->isConnected() == false) empty++;
	}
	return empty;
}

int BitTorrent::getFreeGate(){
	for(int i = 0; i < gateSize("out"); i++){
		if(gate("out", i)->isConnected() == false) return i;
	}
	return -1;
}
