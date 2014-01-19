#include "AmekaUserConfig.h"
#include "dsp_filters.h"

#define SAMPLE_RATE 256
#define LEAD_NUMBER 16
#define ARRAY_LENGTH 10

class DSPModule : public CWinThread
{
private:
	
	int sampleRate;
	float HighFre;
	float LowFre;
	float CenterFre;
	float BandWidth;
	int Type_design;
	int numSamples;
	float* audioData[LEAD_NUMBER];
	RawDataType* dataBuffer;

	//uint16_t* audioData[LEAD_NUMBER];
public:
	//declare variables here
	DECLARE_DYNCREATE(DSPModule)
	
	DSPModule(void);
	~DSPModule(void);
	
	virtual BOOL InitInstance();   // virtual function that derive from base-class.
	virtual int Run();
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
