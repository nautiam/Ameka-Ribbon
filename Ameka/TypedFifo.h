#pragma once

#include <afxmt.h>

//////////////////////////////////////////////////////////////////////////
// AmekaData - this class pretends to be thread safe
template <typename T>
class AmekaData
{
private:
	T * m_pW; // write pointer
	T * m_pR; // read pointer
	T * m_pBuf; // start address of the buffer
	T * m_pEnd; // end address of the buffer
	CCriticalSection m_cs; // sinchronicity
	CEvent m_eNewData; // pulsed when a write operation was performed
public:
	AmekaData();
	AmekaData(const UINT nCnt);
	~AmekaData();

	BOOL Init(const UINT nCnt);
	BOOL Realloc(const UINT nCnt);
	void Free();

	// returns the count of the elements in the fifo
	int GetCount(); 

	// returns the maximum number of elements that can fit into the buffer
	int GetCapacity();

	int popData(T * pBuf, UINT nCnt);	
	void pushData(const T * pBuf, UINT nCnt);
	void Drain(const UINT nCnt);
	DWORD WaitForNewData(DWORD dwMilliseconds = INFINITE);
	HANDLE GetNewDataEventHandle() const {return m_eNewData.m_hObject;}
private:
	int _popData(T * pBuf, UINT nCnt);
	int _GetCount() const;
	void _Drain(const UINT nCnt);
};

//////////////////////////////////////////////////////////////////////////
// AmekaData

template <typename T> 
AmekaData<T>::AmekaData(void)
{
	m_pBuf = NULL;
	m_eNewData.ResetEvent();
}

template <typename T>
AmekaData<T>::AmekaData( const UINT nCnt )
{
	m_pBuf = NULL;
	m_eNewData.ResetEvent();
	Init(nCnt);
}

template <typename T> 
AmekaData<T>::~AmekaData(void)
{
	Free();
}

template <typename T> 
inline BOOL AmekaData<T>::Init(const UINT nCnt)
{	
	CSingleLock lock(&m_cs, TRUE);
	if (m_pBuf) // already initialized
		return FALSE;

	m_pW = m_pR = m_pBuf = new T[nCnt];
	m_pEnd = m_pBuf + nCnt;

	if (!m_pBuf) // if couldn't initialize
		return FALSE;

	return TRUE;
}

template <typename T> 
inline void AmekaData<T>::Free()
{
	CSingleLock lock(&m_cs, TRUE);

	if (m_pBuf)
		delete [] m_pBuf;

	m_pBuf = NULL;
}

template <typename T>
BOOL AmekaData<T>::Realloc( const UINT nCnt )
{
	CSingleLock lock(&m_cs, TRUE);

	const UINT nCurCnt = GetCount();
	if ((nCurCnt >= nCnt) || nCnt <= 0)
		return FALSE;

	T * const pNewBuf = new T[nCnt]; // allocate new buffer

	if (_popData(pNewBuf, nCurCnt) < 0) // read all the data
	{
		delete [] pNewBuf;
		return FALSE;
	}
	delete [] m_pBuf; // free the old buffer

	m_pBuf = pNewBuf;
	m_pR = m_pBuf;
	m_pW = m_pBuf + nCurCnt;
	m_pEnd = m_pBuf + nCnt;

	return TRUE;
}

template <typename T>
int AmekaData<T>::GetCapacity()
{
	CSingleLock lock(&m_cs, TRUE);
	const int nTotalCnt = m_pEnd - m_pBuf;
	return nTotalCnt;
}

template <typename T> 
inline int AmekaData<T>::GetCount()
{
	CSingleLock lock(&m_cs, TRUE);
	return _GetCount();
}

template <typename T>
inline int AmekaData<T>::_GetCount() const
{
	int size = m_pW - m_pR; //f->wptr - f->rptr;
	if (size < 0)
		size += m_pEnd - m_pBuf;
	return size;
}

template <typename T> 
inline int AmekaData<T>::popData(T * pBuf, UINT nCnt)
{	
	CSingleLock lock(&m_cs, TRUE);
	return _popData(pBuf, nCnt);
}

template <typename T>
int AmekaData<T>::_popData( T * pBuf, UINT nCnt )
{
	const UINT nSize = _GetCount();
	const UINT nRead = nCnt;
	if (nSize < nCnt)
		return 0;
	do 
	{
		const int nLen = min((UINT) (m_pEnd - m_pR), nCnt);
		memcpy(pBuf, m_pR, nLen*sizeof(T));
		pBuf = pBuf + nLen;

		_Drain(nLen);
		nCnt -= nLen;

	} while (nCnt > 0);

	return nRead - nCnt;
}

template <typename T> 
void AmekaData<T>::pushData(const T * pBuf, UINT nCnt)
{
	CSingleLock lock(&m_cs, TRUE);

	const UINT nCurSize = _GetCount();
	const UINT nMaxSize = m_pEnd - m_pBuf;
	if (nCurSize >= nMaxSize ) // Do not allow new data if fifo is full
		return;
	do 
	{
		const UINT nLen = min((UINT) (m_pEnd - m_pW), nCnt);

		memcpy(m_pW, pBuf, nCnt*sizeof(T));
		m_pW += nLen;

		if (m_pW >= m_pEnd)
			m_pW = m_pBuf;

		pBuf += nLen;
		nCnt -= nLen;
	} while (nCnt > 0);

	m_eNewData.PulseEvent();
}

template <typename T> 
_inline void AmekaData<T>::Drain(const UINT nCnt)
{
	CSingleLock lock(&m_cs, TRUE);
	_Drain(nCnt);
}

template <typename T>
inline void AmekaData<T>::_Drain(const UINT nCnt)
{
	m_pR += nCnt;
	if (m_pR >= m_pEnd)
		m_pR -= m_pEnd - m_pBuf;
}

template <typename T>
inline DWORD AmekaData<T>::WaitForNewData( DWORD dwMilliseconds )
{
	return ::WaitForSingleObject(m_eNewData.m_hObject, dwMilliseconds);
}