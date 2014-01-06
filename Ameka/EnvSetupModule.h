#include <stdint.h>
#include <Windows.h>

class EnvSetupModule
{
public:
	//declare variables here
	
	//declare functions here
	int amekaSetupEnv();						//setup the program setting after parse from file
	void amekaCheckLicense(LPCTSTR fPath);		//check program liscense
	int amekaLoadSetting(LPCTSTR fPath);		//load setting from file
	void amekaLoadDefaultSetting();				//load program default setting, used in case amekaLoadSetting fails
	int amekaSaveSetting(LPCTSTR fPath);		//save setting midified by the user
private:
	//declare variables here

	//declare variables here
};