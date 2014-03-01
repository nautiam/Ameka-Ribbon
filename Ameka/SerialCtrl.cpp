#include "StdAfx.h"
#include "SerialCtrl.h"
#include "easylogging++.h"
#include "DSPModule.h"
#include <atltime.h>
#include "Ameka.h"

// Set cau hinh mac dinh cua cong com
SerialCtrl::SerialCtrl(void):m_portStatus(FALSE),m_portHandle(NULL)
{
	m_portConfig.ByteSize = 8;            // Byte of the Data.
	m_portConfig.StopBits = ONESTOPBIT;   // Use one bit for stopbit.
	m_portConfig.Parity = NOPARITY;       // No parity bit
	m_portConfig.BaudRate = CBR_115200;     // Baudrate 115200 bit/sec
}

// Destructor Serial Ctrl
SerialCtrl::~SerialCtrl(void)
{
	m_portHandle = NULL;
}

// Ghi trang thai cong
void SerialCtrl::SetPortStatus(BOOL bOnOff)
{
	m_portStatus=bOnOff;
}

// Doc trang thai cong
BOOL SerialCtrl::GetPortStatus(void)
{
	return m_portStatus;
}

// Doc xu ly cong
HANDLE SerialCtrl::GetPortHandle(void)
{
	return m_portHandle;
}

// Mo cong voi cau hinh da co
BOOL SerialCtrl::OpenPort(DCB dcb, CString portName)
{

	if (m_portStatus == FALSE)  // if port is opened already, not open port again.
	{
		m_portHandle = CreateFile((LPCTSTR)portName,  // Specify port device: default "COM1"
			GENERIC_READ | GENERIC_WRITE,       // Specify mode that open device.
			0,                                  // the devide isn't shared.
			NULL,                               // the object gets a default security.
			OPEN_EXISTING,                      // Specify which action to take on file. 
			0,                                  // default.
			NULL);                              // default.

		if(m_portHandle != INVALID_HANDLE_VALUE)
		{
			// Get current configuration of serial communication port.
			if (GetCommState(m_portHandle,&m_portConfig) == 0)
			{
				//AfxMessageBox("Get configuration port has problem.");
				LOG(ERROR) << "System Error: Get configuration port has problem";
				return FALSE;
			}
			// Assign user parameter.
			m_portConfig.BaudRate = dcb.BaudRate;    // Specify baud rate of communicaiton.
			m_portConfig.StopBits = dcb.StopBits;    // Specify stopbit of communication.
			m_portConfig.Parity = dcb.Parity;        // Specify parity of communication.
			m_portConfig.ByteSize = dcb.ByteSize;    // Specify  byte of size of communication.

			// Set current configuration of serial communication port.
			if (SetCommState(m_portHandle,&m_portConfig) == 0)
			{
				//AfxMessageBox("Set configuration port has problem.");
				LOG(ERROR) << "System Error: Set configuration port has problem";
				return FALSE;
			}

			// instance an object of COMMTIMEOUTS.
			COMMTIMEOUTS comTimeOut;                   
			comTimeOut.ReadIntervalTimeout = MAXDWORD;
			comTimeOut.ReadTotalTimeoutMultiplier = 0;
			comTimeOut.ReadTotalTimeoutConstant = 0;
			comTimeOut.WriteTotalTimeoutMultiplier = 3;
			comTimeOut.WriteTotalTimeoutConstant = 2;
			// set the time-out parameter into device control.
			if (SetCommTimeouts(m_portHandle,&comTimeOut) == 0)
			{
				LOG(ERROR) << "System Error: Set timeouts parameters has problem";
				return FALSE;
			}
			m_portStatus = TRUE; 
			return TRUE;      
		}
	}
	return FALSE;
}

// Doc cau hinh tu user va goi mo cong
BOOL SerialCtrl::OpenPort(CString baudRate, CString portName)
{
	DCB configSerial;
	configSerial.ByteSize = 8;
	configSerial.StopBits = ONESTOPBIT;
	configSerial.Parity = NOPARITY;
	switch(atoi((LPCSTR)(CStringA)baudRate))
	{
	case 110:
		configSerial.BaudRate = CBR_110;
		break;
	case 300:
		configSerial.BaudRate = CBR_300;
		break;
	case 600:
		configSerial.BaudRate = CBR_600;
		break;
	case 1200:
		configSerial.BaudRate = CBR_1200;
		break;
	case 2400:
		configSerial.BaudRate = CBR_2400;
		break;
	case 4800:
		configSerial.BaudRate = CBR_4800;
		break;
	case 9600:
		configSerial.BaudRate = CBR_9600;
		break;
	case 14400:
		configSerial.BaudRate = CBR_14400;
		break;
	case 19200:
		configSerial.BaudRate = CBR_19200;
		break;
	case 38400:
		configSerial.BaudRate = CBR_38400;
		break;
	case 56000:
		configSerial.BaudRate = CBR_56000;
		break;
	case 57600:
		configSerial.BaudRate = CBR_57600;
		break;
	case 115200 :
		configSerial.BaudRate = CBR_115200;
		break;
	case 128000:
		configSerial.BaudRate = CBR_128000;
		break;
	case 256000:
		configSerial.BaudRate = CBR_256000;
		break;
	default:
		break;
	}

	return OpenPort(configSerial,portName);
}


BOOL SerialCtrl::Read(char * inputData, const unsigned int & sizeBuffer, unsigned long & length)
{
	/*COMSTAT comstat;
	DWORD dwRead, Err;
	ClearCommError(m_portHandle, &Err, &comstat);
	dwRead = comstat.cbInQue;*/
	/*if (dwRead < sizeBuffer)
		sizeBuffer = &(unsigned int)dwRead;*/
	if (ReadFile(m_portHandle,  // handle of file to read
		inputData,               // handle of file to read
		sizeBuffer,              // number of bytes to read
		&length,                 // pointer to number of bytes read
		NULL) == 0)              // pointer to structure for data
	{
		// AfxMessageBox("Reading of serial communication has problem.");
		LOG(ERROR) << "System Error: Read serial communication has problem";
		return FALSE;
	}
	/*ClearCommError(m_portHandle, &Err, &comstat);
	dwRead = comstat.cbInQue;*/

	if (length > 0)
	{
		inputData[length] = NULL; // Assign end flag of message.
		return TRUE;  
	}  
	return TRUE;
}

BOOL SerialCtrl::Write(CString outputData, const unsigned int & sizeBuffer, unsigned long & length)
{
	if (length > 0)
	{
		if (WriteFile(m_portHandle, // handle to file to write to
			outputData,              // pointer to data to write to file
			sizeBuffer,              // number of bytes to write
			&length,NULL) == 0)      // pointer to number of bytes written
		{
			//AfxMessageBox("Reading of serial communication has problem.");
			LOG(ERROR) << "System Error: Write serial communication has problem";
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL SerialCtrl::ClosePort(void)
{
	if (m_portStatus == TRUE)               // Port need to be open before.
	{
		m_portStatus = FALSE;                 // Update status
		if(CloseHandle(m_portHandle) == 0)    // Call this function to close port.
		{
			/* TODO add code here: Ghi log file System error */
			LOG(ERROR) << "System Error: Close Port has problem";
			return FALSE;
		}    
		return TRUE;
	}
	return FALSE;
}

BOOL SerialCtrl::SetPortEvent(DWORD Event)
{
    return SetCommMask(m_portHandle, Event);
}


const unsigned short MAX_MESSAGE = 4096;

IMPLEMENT_DYNCREATE(ReadThread,CWinThread)
ReadThread::ReadThread() :m_serialIO(NULL)
{
	//confirmTimes = 0;
}
ReadThread::~ReadThread()
{
	m_serialIO = NULL;
}


BOOL ReadThread::InitInstance()
{
	return TRUE;
}

int ReadThread::Run()
{
    /* A. Viet add new */
    //DWORD	EventMask;
	char buf[MAX_MESSAGE+100] = {0};
	unsigned long lenMessage;
	int counter = 0;

	// Check signal controlling and status to open serial communication.
    /* A. Viet replace */
	while(1)
	{
        if(m_serialIO->m_bState == S_UNINITTIALZED)
        {
            if(m_serialIO->m_serialCtrl.OpenPort(m_serialIO->m_DCB,(m_serialIO->m_strPortName))==TRUE)
            {
                /* Chuyen trang thai State */
                m_serialIO->m_bState = S_NOCONNECTED;
				//AfxMessageBox("Open serial communication successfully.");
				LOG(INFO) << "Open serial communication successfully";
                /* Su dung su kien EV_RXCHAR cua Comport*/
                if (m_serialIO->m_serialCtrl.SetPortEvent(EV_RXCHAR) == 0)
				{
					LOG(ERROR) << "System Error: Set Event for Serial Com has problem";
				}
            }
            else /* Cannot open com, so change to Error State */
            {
                m_serialIO->m_bState = S_INITTIALZED;
				//LOG(INFO) << "Open Port has problem";
            }
        }
        else if(m_serialIO->m_bState == S_INITTIALZED)
        {
            /* Re-open com port every 1000ms */
            if(m_serialIO->m_serialCtrl.OpenPort(m_serialIO->m_DCB,m_serialIO->m_strPortName)==TRUE)
            {
                /* Chuyen trang thai State */
                m_serialIO->m_bState = S_NOCONNECTED;
                /* Su dung su kien EV_RXCHAR cua Comport*/
				if (m_serialIO->m_serialCtrl.SetPortEvent(EV_RXCHAR) == 0)
				{
					LOG(ERROR) << "System Error: Set Event for Serial Com has problem";
				}
            }
            else /* Cannot open com, so change to Error State */
            {
                Sleep(200);
            }
        }
        else if(m_serialIO->m_bState == S_CONNECTED)
        {
			counter++;
			//LOG(INFO) << counter;
			if (counter >= READ_TIMEOUT)
			{
				m_serialIO->m_bState = S_NOCONNECTED;
				counter = 0;
				LOG(INFO) << "Change State";
			}
			else
			{
				Sleep(5);
   //         if (WaitCommEvent(m_serialIO->m_serialCtrl.GetPortHandle(), &EventMask, NULL) == 0)
			//{
			//	LOG(ERROR) << "System Error: Wait com event has problem";
			//}
   //         /* Neu co ky tu trong COM Port, thi doc tung ky tu */
			//if ((EventMask & EV_RXCHAR) == EV_RXCHAR)
			//{
				unsigned int lenBuff = MAX_MESSAGE;
				if (m_serialIO->m_serialCtrl.Read(buf,lenBuff,lenMessage)==TRUE)
				{
					// TODO add code here : Xu ly tung byte nhan duoc o day
					// TODO add code here : Neu nhan duoc la goi tin dung thi set co m_bPacketOK
					
					if (lenMessage > 0)
					{
						//LOG(INFO) << "DataRead: ";
						//LOG(INFO) << lenMessage;
						for (unsigned int i=0; i<lenMessage; i++)
						{
							char temp_buff = buf[i];
							if (byteProcessing(&temp_buff) == TRUE)
							{
								m_serialIO->m_bPacketOK = TRUE;
								counter = 0;
							};
							if(m_serialIO->m_bPacketOK == TRUE)
							{
								// TODO add code here : Xu ly goi tin o day
								/* Sau khi xu ly xong thi clear co m_bPacketOK */
								if (packetProcessing() != 0)
								{
									LOG(ERROR) << "Error: Packet Processing has problem";
								}
								m_serialIO->m_bPacketOK = FALSE;
							}
						}
					}
				}
				else
				{
					m_serialIO->m_bState = S_UNINITTIALZED;
					m_serialIO->m_serialCtrl.ClosePort();
					AfxMessageBox(L"The program has problem. Please close and reopen the program.");
					LOG(INFO) << "Close port and reopen port";
				}

			}
        }
		else if(m_serialIO->m_bState == S_NOCONNECTED)
        {
			char firstConfirm[4] = {0xb1, 0x02, 0xff, 0x01};
			
			PurgeComm(m_serialIO->m_serialCtrl.GetPortHandle(), PURGE_RXCLEAR);
			m_serialIO->Write(firstConfirm, 4);
			Sleep(10);
			//if (WaitCommEvent(m_serialIO->m_serialCtrl.GetPortHandle(), &EventMask, NULL) == 0)
			//{
			//	LOG(ERROR) << "System Error: Wait com event has problem";
			//}
   //         /* Neu co ky tu trong COM Port, thi doc tung ky tu */
			//if ((EventMask & EV_RXCHAR) == EV_RXCHAR)
			//{
			unsigned int lenBuff = MAX_MESSAGE;
			if (m_serialIO->m_serialCtrl.Read(buf,lenBuff,lenMessage)==TRUE)
			{
				// TODO add code here : Xu ly tung byte nhan duoc o day
				// TODO add code here : Neu nhan duoc la goi tin dung thi set co m_bPacketOK
				if (lenMessage > 0)
				{
					for (unsigned int i=0; i<lenMessage; i++)
					{
						char temp_buff = buf[i];
						if (byteProcessing(&temp_buff) == TRUE)
						{
							m_serialIO->m_bPacketOK = TRUE;
						};
						if(m_serialIO->m_bPacketOK == TRUE)
						{
							// TODO add code here : Xu ly goi tin o day
							/* Sau khi xu ly xong thi clear co m_bPacketOK */
							if (packetProcessing() != 0)
							{
								LOG(ERROR) << "Error: Packet Processing has problem";
							}
							m_serialIO->m_bPacketOK = FALSE;
						};
					};
				}
			}
			else
			{
				m_serialIO->m_bState = S_UNINITTIALZED;
				m_serialIO->m_serialCtrl.ClosePort();
				AfxMessageBox(L"The program has problem. Please close and reopen the program.");
				LOG(INFO) << "Close port and reopen port";
			}           
        }
	}
	return 0;
}

//Kiem tra goi tin dung ring buffer
BOOL ReadThread::byteProcessing(char * buffer)
{
	char checksum = 0;
	const char stdCfmPacket[27] = {0xff, 0xff, 0xff, 0xb1, 0x16, 0x00, 0x81, 0x12,
		0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x14,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x31, 0x32, 0x33, 0x05, 0x35, 0x83};
	const char stdCfmData[5] = {0xb1, 0x24, 0x10, 0x0, 0x00};

	packetType = PACKET_CONFIRM;

	m_serialIO->m_recvBuffer->pushData(*buffer);
	
	for (int i=0; i<27;i++)
	{
		if (m_serialIO->m_recvBuffer->get(37-i) != stdCfmPacket[26-i])
		{
			packetType = WRONG_PACKET;
			break;
		}
	}

	if (packetType == PACKET_CONFIRM)
	{
		//AfxMessageBox("Receive a packet confirm!");
		LOG(DEBUG) << "Receive a packet confirm";
		//confirmTimes++;
		return TRUE;
	}
	/*char temp0 = m_serialIO->m_recvBuffer->get(0);
	char temp1 = m_serialIO->m_recvBuffer->get(1);
	char temp2 = m_serialIO->m_recvBuffer->get(2);
	char temp4 = m_serialIO->m_recvBuffer->get(4);*/
	if ((m_serialIO->m_recvBuffer->get(0) != stdCfmData[0]) || (m_serialIO->m_recvBuffer->get(1) != stdCfmData[1])
		|| (m_serialIO->m_recvBuffer->get(2) != stdCfmData[2]) || (m_serialIO->m_recvBuffer->get(4) != stdCfmData[4]))
	{
		packetType = WRONG_PACKET;
		return FALSE;
	}
	for (int i=1; i<(MAX_RECV_BUFFER - 1); i++)
	{
		checksum += m_serialIO->m_recvBuffer->get(i);
	}
	char temp37 = m_serialIO->m_recvBuffer->get(37);
	if (checksum == temp37)
	{
		LOG(DEBUG) << "Receive a packet data";
		packetType = PACKET_DATA;
		return TRUE;
	}
	//checksum += m_serialIO->m_recvBuffer->get(37);
	return FALSE;
}

int ReadThread::packetProcessing()
{
	char firstConfirm[4] = {0xb1, 0x02, 0xff, 0x01};
	char secondConfirm[4] = {0xb1, 0x02, 0x91, 0x93};
	switch (packetType)
	{
	case PACKET_CONFIRM:
		{
			/*if (confirmTimes == 1)
			{
				m_serialIO->Write(firstConfirm, 4);
			}
			else if (confirmTimes >= 2)
			{*/
			if (m_serialIO->m_bState == S_NOCONNECTED)
			{
				m_serialIO->Write(secondConfirm, 4);
				//confirmTimes = 0;
				m_serialIO->m_bState = S_CONNECTED;
			}
			//}
			break;
		}
	case PACKET_DATA:
		{
			//AfxMessageBox("Packet Data received");
			RawDataType temp;
			temp.time = 0;
			CTime t = GetCurrentTime();
			temp.time = t.GetTime();
			for (int i=0; i<16; i++)
			{
				uint16_t tempBuffer=0;
				tempBuffer = (m_serialIO->m_recvBuffer->get(i*2 + 5)) << 8;
				tempBuffer |=  (m_serialIO->m_recvBuffer->get(i*2 + 6) & 0x00FF);
				temp.value[i] = tempBuffer;
			};
			//int i = m_serialIO->RawData->pushData(temp);
			if (m_serialIO->m_bState == S_CONNECTED)
			{
				//LOG(INFO) << temp.value[0];
				//RawDataType* data = new RawDataType[1];
				//memcpy(data, &temp, 1*sizeof(RawDataType));
				POSITION pos =  theApp.docList.GetHeadPosition();
				while(pos) 
				{ 
					CAmekaDoc* curr = theApp.docList.GetNext(pos);
					int count = curr->dataBuffer->pushData(temp);
				}
				//int count = m_serialIO->RawData->pushData(temp);
				//delete [] data;
				//if (count != 0)
				//{
				//	//AfxMessageBox("Error");
				//	LOG(WARNING) << "Raw data buffer is full";
				//}
			}
			break;
		}
	default:
		break;
	}

	return 0;
}
IMPLEMENT_DYNCREATE(WriteThread,CWinThread)
WriteThread::WriteThread()
{

}

WriteThread::~WriteThread()
{
	m_serialIO = NULL;
}

BOOL WriteThread::InitInstance()
{
	return TRUE;
}

int WriteThread::Run()
{
    DWORD nWritten;
	while (1)
	{
		WaitForSingleObject(m_serialIO->m_WriteEvent, INFINITE);	// Doi su kien Write Event vo han
        if((m_serialIO->m_bState == S_CONNECTED) || (m_serialIO->m_bState == S_NOCONNECTED))
        {
            if(m_serialIO->m_serialCtrl.Write((LPCTSTR)m_serialIO->m_sendBuffer,m_serialIO->m_sendSize,nWritten) == FALSE)
            {
                // TODO add code here : Ghi log file system error
				LOG(ERROR) << "System Error: Write data has problem";
            }
        }
        if (ResetEvent(m_serialIO->m_WriteEvent) == 0)
		{
			LOG(ERROR) << "System Error: Reset com event has problem";
		}
	}
	return 1;
}

CSerialIO::CSerialIO(CString strPortName,CString strBaudRate)
{
    m_readProcess  = NULL;
    m_writeProcess = NULL;

    m_strPortName = strPortName;
    m_strBaudRate = strBaudRate;
	
    m_bState = S_UNINITTIALZED;
    m_bPacketOK = FALSE;
    m_bSendPacket = FALSE;
	RawData = new amekaData<RawDataType>(1000);
	m_recvBuffer = new amekaData<char>(MAX_RECV_BUFFER);
	Init();	
}
CSerialIO:: ~CSerialIO()
{
	UnInit();
}
BOOL CSerialIO::Init()
{
    BOOL retVal = TRUE;

    m_readProcess = (ReadThread*)AfxBeginThread(RUNTIME_CLASS(ReadThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_readProcess->setOwner(this);

    m_writeProcess = (WriteThread*)AfxBeginThread(RUNTIME_CLASS(WriteThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_writeProcess->setOwner(this);

    if((m_readProcess == NULL) || (m_writeProcess == NULL))
    {
        /* TODO Code add here : Ghi Log File la System Error */
		LOG(ERROR) << "System Error: Create thread has problem";
        retVal = FALSE;
    }

    /* Create Event for Write thread */
    m_WriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if((m_WriteEvent == NULL) || (m_WriteEvent == INVALID_HANDLE_VALUE))
    {
        /* TODO Code add here : Ghi Log File la System Error */
		LOG(ERROR) << "System Error: Create com event has problem";
        retVal = FALSE;
    }
	if (ResetEvent(m_WriteEvent) == 0)
	{
		LOG(ERROR) << "System Error: Reset com event has problem";
	}

	m_DCB.ByteSize = 8;
	m_DCB.StopBits = ONESTOPBIT;
	m_DCB.Parity = NOPARITY;
	switch(atoi((LPCSTR)(CStringA)m_strBaudRate))
	{
	case 110:
		m_DCB.BaudRate = CBR_110;
		break;
	case 300:
		m_DCB.BaudRate = CBR_300;
		break;
	case 600:
		m_DCB.BaudRate = CBR_600;
		break;
	case 1200:
		m_DCB.BaudRate = CBR_1200;
		break;
	case 2400:
		m_DCB.BaudRate = CBR_2400;
		break;
	case 4800:
		m_DCB.BaudRate = CBR_4800;
		break;
	case 9600:
		m_DCB.BaudRate = CBR_9600;
		break;
	case 14400:
		m_DCB.BaudRate = CBR_14400;
		break;
	case 19200:
		m_DCB.BaudRate = CBR_19200;
		break;
	case 38400:
		m_DCB.BaudRate = CBR_38400;
		break;
	case 56000:
		m_DCB.BaudRate = CBR_56000;
		break;
	case 57600:
		m_DCB.BaudRate = CBR_57600;
		break;
	case 115200 :
		m_DCB.BaudRate = CBR_115200;
		break;
	case 128000:
		m_DCB.BaudRate = CBR_128000;
		break;
	case 256000:
		m_DCB.BaudRate = CBR_256000;
		break;
	default:
		break;
	}
	m_readProcess->ResumeThread();
    m_writeProcess->ResumeThread();

	/*DSPModule*  m_dspProcess;
	m_dspProcess = (DSPModule*)AfxBeginThread(RUNTIME_CLASS(DSPModule), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_dspProcess->ResumeThread();*/

	return retVal;
}
void CSerialIO::UnInit()
{
	if(m_readProcess)
		m_readProcess->SuspendThread();
    if(m_writeProcess)
		m_writeProcess->SuspendThread();
	if (ResetEvent(m_WriteEvent) == 0)
	{
		LOG(ERROR) << "System Error: Reset com event has problem";
	}
    m_serialCtrl.ClosePort();
}

void CSerialIO::Write(char *outPacket,int outLength)
{
	if(outLength<=MAX_SEND_BUFFER)
	{
		memcpy(m_sendBuffer,outPacket,outLength);
		m_sendSize = outLength;
		if (SetEvent(m_WriteEvent) == 0)
		{
			LOG(ERROR) << "System Error: Reset com event has problem";
		}
		//SetEvent(m_WriteEvent);
	}
    else
	{
		// TODO add code here : Ghi log file, system error
		LOG(ERROR) << "System Error: Write Data is too large";
	}
	return ;
}