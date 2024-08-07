#if !defined(AFX_MYXML__INCLUDED_)
#define AFX_MYXML__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MyXML.h
// A simple state machine based non-validating XML "parser"
// optional: may later use string and stack class - for now just counts
// nesting levels of tags
//////////////////////////////////////////////////////////////////////
//
/*
For Hosted Application Services PoC demo

Rev		Date		Author		Comments
----------------------------------------
0.1		Feb 23,2000	TB
*/

#include <stdio.h>

class MyXML
{
public:
	MyXML(FILE *inp, int m_bufsize = 4096); // requires an input stream
	MyXML(char *buf, int m_bufsize = 4096); // zero terminated buffer not UNICODE..
	~MyXML();

public:
	const char *GetCurrentBuf() { return m_buf; }; // whatever was fetched last...
	char *GetNextTag();							   // next tag in stream or null
	char *GetNextTag(char *tag);				   // next matching tag in stream or null
	char *GetNextDataField();					   // next data field in stream or null
	char *GetNextTaggedField(char *tag);		   // search for first occurence of tag, get data or null
	int GetNesting();							   // get current nesting level for what it's worth
	void Rewind();								   // rewind the buffer to the start to re-parse
protected:
	int NextChar();
	void ResetParser();
	char *Parse();
	FILE *m_inp;
	char *m_cinp;
	char *m_inpBuf;
	enum State
	{
		START,
		BEGIN_TAG,
		TERM_TAG,
		TAG,
		END_TERM_TAG,
		END_TAG,
		EOFSTATE
	};
	State m_state;
	char *m_buf;
	char *m_pbuf;
	int m_nesting;

private:
	enum InpType
	{
		CHARBUF,
		FILEBUF
	};
	InpType m_inpType;
	int m_bufsize;

}; // MyXML

#endif
