#include <stdint.h>
#include <Windows.h>

class RecordingModule
{
public:
	//declare variables here
	
	//declare functions here
	int amekaWrite2File(LPCTSTR fPath);			//write recording data to file
	int amekaPrintData(uint8_t channel);		//print recorded data
	int amekaReadFromFile(LPCTSTR fPath);		//load recorded data from file
private:
	//declare variables here

	//declare variables here
};