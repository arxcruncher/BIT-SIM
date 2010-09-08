Welcome to BIT-SIM, a bittorrent simulator! 

This file provides instruction on how to run the BIT-SIM included in the distribution.

------------------------
0. About this distribution
------------------------

This distribution includes both the BIT-SIM source code. This simulator is supposed to run on top of OMNeT++ 4.0, which can be downloaded for free on www.omnetpp.org. Make sure that OMNeT++ 4.0 is installed prior and the paths to OMNeT are configured correctly.

The bittorrent clients are dynamically generated during runtime. The torrent "dummy.torrent" will be automatically loaded into the simulator at startup.

We would like to provide a quality product therefore we want your input. Feedback and bug reporting are highly appreciated. 


------------------------
1. Running BIT-SIM
------------------------
Run BIT-SIM as you would run a normal set of modules on top of OMNeT++:

$ opp_makemake
$ make
$ ./BIT-SIM

BIT-SIM settings can be altered from within the omnetpp.ini file. See this file for comment on the different settings.


------------------------
2. Technical Contact
------------------------
Karel De Vogeleer
Dept. of Telecommunication Systems,
Blekinge Institute of Tehnology
Karlskrona Sweden
karel.de.vogeleer@bth.se
