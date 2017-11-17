# NavSim

This Navigation Simulator has been developed to perform a real-time simulation of navigation systems. 
In this way navigation equipment could be tested, without actually being onside. 

A large range of systems can be simulated, e.g. Trisponder, Artemis, Micro-Fix, Hyper-Fix, Decca, Sercel NR53. 
Also simultanious output of depth data from Deso-25 and Echotrac can be generated.
With multi-channel serial cards, up to 8 outputs could be used.

It would simulate the serial output of the following possible navigation devices:

* Trisponder
* Artemis   
* Micro-Fix 
* Hyper-Fix 
* Decca     
* Syledis   
* Polartrack
* Falcon    
* UCM_40    
* Echotrac  
* NMEA_0183 
* NR_53_103 
* Deso_25   
* Radar-Fix 
* Pulse8    
* TRIMBLE   

For each navigation system, if possible, the output format could be changed.

For example, for Artemis, the following output formats are possible:

* ASCII 16            
* ASCII 17            
* BCD (0.01)          
* BCD (0.001)         
* BCD (0.001) + ADB   
* Telegram (Mark IV) 

This program also took the many possible coordinate projections in account.

