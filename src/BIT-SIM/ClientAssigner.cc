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

#include "ClientAssigner.h"
#include "Convert.h"
#include "BTClient.h"

#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

Define_Module(ClientAssigner);

ClientAssigner::ClientAssigner(){
	seeders = 0;
	clients = 0;
	maxNumberOfConnections = 0;
}

void ClientAssigner::initialize(){
	seeders = par("seeders");
	clients = par("clients");
	maxNumberOfConnections =  par("maxNumberOfConnections");
	WATCH(seeders);
	WATCH(clients);
	WATCH(maxNumberOfConnections);
	seederArray = new bool[clients];
	int random = 0;
	
	tracker = check_and_cast<BTTracker*>(getParentModule()->getSubmodule("tracker"));
	tracker->setGateSize("in", clients);
	tracker->setGateSize("out", clients);
	tracker->setAmountOfClients(clients);
	
	clientArray = new BTClient*[clients];
 	for(int i = 0; i < clients ;i++){
		string temp = "client";
		temp.append(stringify((int)i));
  		cModuleType *moduleType = cModuleType::get("BTClient");
		cModule	*module = moduleType->create(temp.c_str(), getParentModule());
		BTClient *bTClient = check_and_cast<BTClient*>(module);
		bTClient->setGateSize("in", maxNumberOfConnections);
		bTClient->setGateSize("out", maxNumberOfConnections);
		bTClient->scheduleStart(simTime());
		clientArray[i] = bTClient;
		//	module->finalizeParamters();
		module->buildInside();
		bTClient->getDisplayString().setTagArg("i", 0, "device/laptop");
		cout << "allocating client "<< i << "/" << clients << "\r";
	}
	cout << endl;
	
	for(int i = 0; i < clients ;i++){
		seederArray[i] = false;
	}
	
 	if(seeders <= clients){
 		for(int i = 0; i < seeders ;i++){
 			bool found = false;
 			while(!found){
 				random = intuniform(0, clients-1);
 				if(!seederArray[random]){
 					seederArray[random] = true;
 					found = true;
 				}	
 			}
 		}
 	}
	//else ev << "Smart Client Assigner: to many seeders, check *.ini-file!" << endl; //omnetpp 4.0

	simtime_t baseTime = 0;
	cout << "---------------------- SmartClientAssigner Array --clients: " << clients << " - seeders: " << seeders << " -------------------" << endl;
	for(int i = 0; i < clients ;i++){
		if(seederArray[i]){
			clientArray[i]->setSeeder();
			cout << "Peer " << i << " seeder" << endl;
		}
		else{
			baseTime += (double)par("sessionInterArrivalTime");
			// Set the client as leecher and send it it's session arrival time and seeding time
			clientArray[i]->setLeecher(baseTime, (simtime_t)par("seedingTime"));
			cout << "Peer " << i << " leecher, arrival time: " << baseTime << endl;
		}
	}
	cout << "----------------------------- SmartClientAssigner Array -----------------------------------" << endl << endl;	
}

void ClientAssigner::peerIsLeaving(const char *peer){
	for(int i = 0; i < clients ;i++){
		clientArray[i]->handlePeerIsGone(peer);

	}
}

void ClientAssigner::peerIsSeeder(int ID, bool seeder){
	if(ID < clients) seederArray[ID] = seeder;
}

int ClientAssigner::getMode(int ID){
	if(ID < clients){
		if(seederArray[ID]) return 0;
		return -1;
	}
	else return -2;
}
