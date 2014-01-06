#include <stdint.h>
#include <Windows.h>

class CommModule
{
public:
	//declare variables here
	
	//declare functions here
	int amekaCommunication();					//handle Communication module
	int amekaLoadEEG();							//load and communicate data from EEG
	int amekaLoadPhoticScript();				//load photic script data
	int amekaWriteRawData(uint8_t channel);		//write raw data to specified channel
private:
	//declare variables here

	//declare variables here
};