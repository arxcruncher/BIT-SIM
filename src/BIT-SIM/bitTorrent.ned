
// Topology for a BitTorrent swarm.
//
// This file is part of a BitTorrent Simulator
//
// Copyright (c) 2007. Blekinge Institute of Technology (BTH).
// All Rights Reserved.
//
// Contact information for redistribution (binary or source code):
//
// Karel De Vogeleer
// Department of Telecommunications System
// School of Engineering
// SE-371 79 Karlskrona
// SWEDEN
//
// kdv@bth.se

channel DatarateChannel {
    @class(cDatarateChannel);
    bool disabled = false;
    double delay = 0s @unit(s);
    double datarate = 0bps @unit(bps);
    double ber = 0;
    double per = 0;
}


simple BTTracker
{
    gates:
        input in[];
        output out[];
}

simple BTClient
{
    parameters:
        volatile double pieceDistribution;
        volatile double numberOfPeersWanted;
        volatile double algorithm;
        volatile double blockLength;
        volatile double channelBandwidth;
        volatile double assymetricLinks;
        volatile double channelDelay;
        volatile double PieceRequestWaitingTime;

    gates:
        input in[];
        output out[];
}

simple TorrentFile
{
}

simple ClientAssigner
{
    parameters:
        volatile double seeders;
        volatile double maxNumberOfConnections;
        volatile double seedingTime;
        volatile double clients;
        volatile double sessionInterArrivalTime;
}

module bitTorrentNetworkMod
{
    submodules:

        SmartClientAssigner: ClientAssigner {
            parameters:
                @display("p=150,300;i=block/cogwheel_s");

// BitTorrent clients are created dynamically. Go to the //.ini file to change settings.

        }
        tracker: BTTracker {
            parameters:
                @display("p=200,225;i=device/server_l");

            gates:
                in[0];
                out[0];
        }
        torrentFile: TorrentFile {
            parameters:
                @display("p=250,300;i=block/cogwheel_s");

        }
    connections:


}

network bitTorrentNetwork extends bitTorrentNetworkMod
{
    parameters:
}
