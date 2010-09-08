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

#include "TorrentFile.h"

#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "Convert.h"

using namespace std;

Define_Module(TorrentFile);

void TorrentFile::getInt(ifstream &inFile, ostringstream &entry){
	char num;
	inFile >> num;
	for(;;){
		inFile >> num;
		if(num != 'e') entry << num;
		else break;
	}
}

void TorrentFile::getString(ifstream &inFile, ostringstream &entry){
	char num;
	// Get string length
	ostringstream lengthStr;
	inFile >> num;
	while(num != ':'){
		lengthStr << num;
		inFile >> num;					
	}
	int length = atoi(lengthStr.str().c_str());
	// Read string
	for(; length > 0;length--){
		num = inFile.get();
		entry << num;
	}
}

int TorrentFile::getNext(ifstream &inFile, ostringstream &entry){
	char num;	
	if(!inFile.eof()) {
		inFile >> num;
		if(num == 'e') return -1;
		switch(num){
			case 'd' : {
				getNext(inFile, entry);
				break;
			}
			case 'l' : {
				while(getNext(inFile, entry) != -1) {entry << ", ";}
				break;
			}
			case 'i' : {
				inFile.putback(num);
				getInt(inFile, entry);
				break;
			}
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
			case '7' :
			case '8' :
			case '9' : {
				inFile.putback(num);
				getString(inFile, entry);
				break;
			}
		}
		return 0;
	}
	return -1;
}

TorrentFile::TorrentFile(){
	fileName = "NULL";
	announce = "NULL";
	lengthOfFile = 0;
	lengthOfFile_double = 0.0;
	amountOfPieces = 0;
	amountOfBlocks = 0;
	
	// Writting a torrent file decoder is a not that easy and causes a lot of grief
	// This is a dirty and quick solution for "bendecoding"
	// Though, sufficient for our needs. (Must be replaced by a proper implementation.)
	
	ifstream indata;
	indata.open("dummy.torrent");
	//indata.open(par("torrent"));
	if(!indata) {
		cerr << "Error: torrent file could not be opened" << endl;
	}
	else cout << "Torrent file opened!" << endl;
	
	vector<char> file;
	char character;
	while(!indata.eof()){
		indata >> character;
		file.push_back(character);
	}
	// Controlled buffer overflow here
	for(int i = 0; i < file.size(); i++){
		if(memcmp("4:name", &file[i], 6) == 0){
			int j = 0;
			ostringstream temp;
			do {
				temp << file[i+6+j];
				j++;
			}
			while(file[i+6+j] != ':');
			int length_of_name = atoi(temp.str().c_str());
			fileName = string(&file[i+6+j+1], length_of_name);
			torrent.insert(pair<string, string>("name", fileName));
		}
		else if(memcmp("8:announce", &file[i], 10) == 0){
			int j = 0;
			ostringstream temp;
			do {
				temp << file[i+10+j];
				j++;
			}
			while(file[i+10+j] != ':');
			int length_of_name = atoi(temp.str().c_str());
			announce = string(&file[i+10+j+1], length_of_name);
			torrent.insert(pair<string, string>("announce", announce));
		}
		else if(memcmp(&file[i], "12:piece", 8) == 0){
			int j = 0;
			ostringstream temp;
			do {
				temp << file[i+15+j];
				j++;
			}
			while(file[i+15+j] != 'e');
			piece_length = atoi(temp.str().c_str());
			torrent.insert(pair<string, string>("piece length", temp.str()));
		}
		else if(memcmp(&file[i], "6:length", 8) == 0){
			int j = 0;
			ostringstream temp;
			do {
				temp << file[i+9+j];
				j++;
			}
			while(file[i+9+j] != 'e');
			lengthOfFile_double += atof(temp.str().c_str());
		}
	}
	char double_array[20];
	sprintf(double_array, "%f", lengthOfFile_double);
	string string_array = double_array;
	torrent.insert(pair<string, string>("length", string_array));

	indata.close();
	/*
	cout << "End-of-file reached.." << endl << endl;
	map<string,string>::iterator iter = torrent.find("name");
	if(iter != torrent.end()) fileName = iter->second;
	iter = torrent.find("length");
	if(iter != torrent.end()) lengthOfFile = atoi(iter->second.c_str());
	iter = torrent.find("piece length");
	if(iter != torrent.end()) lengthOfPieces = atoi(iter->second.c_str());
	*/
	
	
	cout << "----------------------------- Torrent File Content -----------------------------------" << endl;
	map<string,string>::iterator it;
	for ( it=torrent.begin() ; it != torrent.end(); it++ ){
    	cout << it->first << " => " << it->second << endl;
	}
	cout << "--------------------------------------------------------------------------------------" << endl;
}

void TorrentFile::initialize(){
	/*
	amountOfPieces = lengthOfFile/lengthOfPieces;
	if (lengthOfFile%lengthOfPieces > 0) amountOfPieces++;
	amountOfBlocks = lengthOfPieces/32768;
	if( lengthOfPieces%32768 > 0 ) amountOfBlocks++;

	WATCH(fileName);
	WATCH(amountOfPieces);
	WATCH(amountOfBlocks);
	*/
}

string TorrentFile::getValue(const char *key){
	map<string,string>::iterator iter = torrent.find(key);
	if( iter != torrent.end() ) {
		return iter->second;
	}
	return "";
}
