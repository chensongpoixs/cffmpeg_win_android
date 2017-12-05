#ifndef __test__CCycleBuffer__    
#define __test__CCycleBuffer__    

#include <iostream>    
#include <assert.h>    
class CCycleBuffer
{
public:
	bool isFull();
	bool isEmpty();
	void Empty();
	int GetLength();
	CCycleBuffer(int size);
	virtual ~CCycleBuffer();
	int Write(char* buf, int count);
	int Read(char* buf, int count);
private:
	bool m_bEmpty, m_bFull;
	char * m_pBuf;
	int m_nBufSize;
	int m_nReadPos;
	int m_nWritePos;

public:
	int GetReadPos() { return m_nReadPos; }
	int GetWritePos() { return m_nWritePos; }
};
#endif /* defined(__test__CCycleBuffer__) */  
