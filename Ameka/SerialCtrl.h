#include "AmekaUserConfig.h"
#ifndef _SERIAL_CTRL_H
#define _SERIAL_CTRL_H

/**************************************************************************/
/* Title: CSerialIO: a useful and simple serial control                   */
/* Author: wufengkai                                                      */
/* Email:  tojine@hotmail.com                                             */


#define MAX_SEND_BUFFER 1024
#define MAX_RECV_BUFFER 38
#define READ_TIMEOUT 100

enum SERIALIO_TYPE {
	S_UNINITTIALZED = 0,
	S_INITTIALZED,
	S_CONNECTED,
	S_NOCONNECTED
};

// TuanHA add
enum SEND_STATE {
	NO_PACKET = 0,
	SENDING_PACKET_CONFIRM,
	SENDING_PACKET_DATA,
};

enum PACKET_TYPE {
	WRONG_PACKET = 0,
	PACKET_CONFIRM,
	PACKET_DATA,
};

class SerialCtrl
{
private:
	BOOL m_portStatus;		// Trang thai mo cong (On, Off)
	HANDLE m_portHandle;	// Xu ly cong (Dong, mo buffer cong, doc, ghi trang thai cong)
	DCB m_portConfig;		// Cau hinh cong (baudrate, name)
public:
	SerialCtrl(void);		// Set cau hinh mac dinh
	~SerialCtrl(void);		// Clear xu ly cong
	void SetPortStatus(BOOL bOnOff);	// Dat trang thai cong
	BOOL GetPortStatus(void);			// Lay trang thai cong
	HANDLE GetPortHandle(void);			// Doc xu ly cong
	BOOL OpenPort(DCB dcb, CString portName=L"COM1");	// Mo cong voi tham so da co
	BOOL OpenPort(CString baudRate=L"115200", CString portName=L"COM1");	// Doc cau hinh cong voi tham so truyen tu ngoai va mo cong
	BOOL Read(char * inputData, const unsigned int & sizeBuffer, unsigned long & length);	// Khai bao mot vung nho de doc du lieu tu serial com
	BOOL Write(CString outputData, const unsigned int & sizeBuffer, unsigned long & length);	// Khai bao mot vung nho de ghi du lieu vao serial com
	BOOL ClosePort(void);	// Dong cong
    BOOL SetPortEvent(DWORD Event);	// Dat su kien cho cong, (EV_RXCHAR: Nhan duoc bat cu du lieu)
};

class CSerialIO;

class ReadThread : public CWinThread
{
public:
	// To enable objects of CObject-derived classes to be created dynamically at  run timeto enable objects of CObject-derived classes to be created 

	DECLARE_DYNCREATE(ReadThread)  
	ReadThread ();               // Constructor.
	virtual ~ReadThread();       // Destructor.

	virtual BOOL InitInstance();   // virtual function that derive from base-class.
	virtual int Run();             // virtual function that derive from base-class.
	void setOwner(CSerialIO* serialIO)	// Chi dinh cho chuong trinh dung dung class
	{
		m_serialIO = serialIO;
        //m_serialIO->m_serialCtrl = &(serialIO->m_serialCtrl);
	}
	CSerialIO*      m_serialIO;         // The pointer that pointer to object user data, such dialog,window
    //SerialCtrl*     m_serialIO->m_serialCtrl;       // The pointer that pointer to object user data, such dialog,window
	// TuanHA add
public:
	BOOL byteProcessing(char * buffer);
	int packetProcessing();
	int countPacket;
	//int confirmTimes;
	int packetType;
	//int countPacketData;
	//char tempPacket[4];
};

class WriteThread : public CWinThread
{
public:
	// To enable objects of CObject-derived classes to be created dynamically at  run timeto enable objects of CObject-derived classes to be created 

	DECLARE_DYNCREATE(WriteThread)  
	WriteThread ();               // Constructor.
	virtual ~WriteThread();       // Destructor.
	virtual BOOL InitInstance();   // virtual function that derive from base-class.
	virtual int Run();             // virtual function that derive from base-class.  
	void setOwner(CSerialIO* serialIO)
	{
		m_serialIO = serialIO;
//        m_serialIO->m_serialCtrl = &serialIO->m_serialCtrl;
	}
	CSerialIO*      m_serialIO;         // The pointer that pointer to object user data, such dialog,window
//    SerialCtrl*     m_serialIO->m_serialCtrl;       // The pointer that pointer to object user data, such dialog,window
};


class CSerialIO
{

public:
	int m_bSendPacket;	// Trang thai nhan goi tin 
	CSerialIO(CString strPortName,CString strBaudRate);	// Khoi tao CSerialIO, tao thread cho run & write
	virtual ~CSerialIO();	// Huy CSerialIO, Suspend Thread

	void Write(char *outPacket,int outLength);// write data directly 

	DCB m_DCB;	// Struct cau hinh cua serial com

    //char m_recvBuffer[MAX_RECV_BUFFER];	// Buffer nhan
	char m_sendBuffer[MAX_SEND_BUFFER];	// Buffer ghi
	amekaData<char>* m_recvBuffer;
	//AmekaData<char>* m_recvBuffer;
	unsigned int m_sendSize;	// Kich thuoc goi truyen di

	CString	m_strPortName;	// Ten cong serial com
	ReadThread*  m_readProcess;	// Tao thread xu ly doc du lieu
    WriteThread* m_writeProcess;	// Tao thread xu ly ghi du lieu

	unsigned char m_bState;	// Trang thai cong com (S_UNINIT, S_INIT, S_CONNECTED)
	BOOL m_bPacketOK;	// Trang thai goi tin, kiem tra xem dung goi tin nhan chua
	HANDLE  m_WriteEvent;	// Su kien ghi
	CString	m_strBaudRate;	// Doc baudrate tu user
    SerialCtrl m_serialCtrl;	// Goi class serialctrl voi tung serialio
	amekaData<RawDataType>* RawData;
	//AmekaData<RawDataType>* RawData;

private:
	BOOL Init();	// Khoi tao Serial Com
	void UnInit();	// Bo du lieu khoi tao
};

#endif