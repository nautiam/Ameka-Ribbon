#include <stdint.h>
#include <Windows.h>

class DSPModule
{
public:
	//declare variables here
	
	//declare functions here
	int amekaDSPEngine();						//handle DSP module
	int amekaHPassFilter(uint8_t channel, uint8_t frequency);	//High Pass filter
	int amekaLPassFilter(uint8_t channel, uint8_t frequency);	//Low Pass filter
	int amekaSpectralTransform(uint8_t channel);//transform data to "tin hieu pho" :">
	int amekaEsdTransform(uint8_t channel);		//transform "tin hieu pho" -> "mat do pho nang luong"
private:
	//declare variables here

	//declare variables here
};