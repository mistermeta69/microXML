//////////////////////////////////////////////////////////////////////
// myxml.cpp
// implemetation of a simple xml "parser"
// requires an input text stream (file)
//  does not interpret any XML escapes for < > or &..
//  Stream based.

/*
For Hosted Application Services PoC demo

Rev		Date		Author		Comments
----------------------------------------
0.1		Feb 23,2000	TB

*/

#include "myxml.h"

#include <ctype.h>
#include <string.h>

// Constructor/destructor
// requires an input stream
//
MyXML::MyXML(FILE *inp, int m_bufsize) : m_buf(0)
{
	m_inp = inp;
	m_inpType = FILEBUF;

	// m_stack = new Stack;
	m_buf = new char[m_bufsize];
	ResetParser();
}

// read input from a buffer
MyXML::MyXML(char *buf, int m_bufsize) : m_buf(0)
{
	m_cinp = buf;
	m_inpType = CHARBUF;
	m_inpBuf = buf;

	// m_stack = new Stack;
	m_buf = new char[m_bufsize];
	ResetParser();
}

MyXML::~MyXML()
{
	// TODO: destroy stack..
	if (m_buf)
		delete[] m_buf;
}

void MyXML::ResetParser()
{
	m_pbuf = m_buf;
	m_state = START;
	m_nesting = 0;
}

// go back to the start of the stream
//
void MyXML::Rewind()
{
	ResetParser();
	switch (m_inpType)
	{
	case CHARBUF:
		m_cinp = m_inpBuf;
		break;
	case FILEBUF:
		fseek(m_inp, 0L, SEEK_SET);
		break;
	default:;
	}
}

int MyXML::NextChar()
{
	switch (m_inpType)
	{
	case CHARBUF:
		if (!m_cinp)
			return EOF;
		if (*m_cinp)
			return *m_cinp++;
		else
			return EOF;
	case FILEBUF:
		if (!feof(m_inp))
			return fgetc(m_inp);
	default:;
	}
	return EOF;
}

/////////////////////////////////////
// take up the parsing where left off
// returns the next token, and state
// m_buf, and m_state
// or NULL if EOF
//

char *
MyXML::Parse()
{
	int c;
	m_pbuf = m_buf; // reset buffer

	while (EOF != (c = NextChar()))
	{
		switch (m_state)
		{

		case START:
			if (c == '<')
				m_state = BEGIN_TAG;
			break;

		case BEGIN_TAG:
			if (isspace(c))
				break;
			if (c == '/')
			{
				m_state = TERM_TAG;
			}
			else if (c == '>')
			{
				m_state = END_TAG; // empty tag...
			}
			else
			{
				m_state = TAG; // begin storing the tag...
				*m_pbuf++ = (char)c;
			}
			break;

		case TAG:
			if (isspace(c))
				break;
			if (c == '>')
			{
				m_state = END_TAG;
				// m_stack.push ( m_buf ); //for validation...
				m_nesting++;
				*m_pbuf = NULL;
				//////////////////////
				return m_buf; // tag value m_state = END_TAG
			}
			else
			{
				*m_pbuf++ = (char)c; // save the character
			}
			break;

		case END_TAG:
			if (c == '<')
			{
				m_state = BEGIN_TAG;

				*m_pbuf = NULL;
				//////////////////////
				return m_buf; // BEGIN_TAG means we have a field!
			}
			else
			{
				*m_pbuf++ = (char)c; // save field character incl spaces
			}
			break;

		case TERM_TAG:
			if (isspace(c))
				break; // ignore spaces in tag
			if (c == '>')
			{
				m_state = END_TERM_TAG;
				m_nesting--;
				// TODO: do a compare to the stack
				*m_pbuf = NULL;
				//////////////////////
				return m_buf; // return the END_TERM_TAG without the slash...
			}
			else
			{
				*m_pbuf++ = (char)c; // save tag char to buffer
			}
			break;

		case END_TERM_TAG:
			if (c == '<')
			{ // ignore everthing except <
				m_state = BEGIN_TAG;
			}
			break;

		default:;
		} // switch
	} // while
	m_state = EOFSTATE;
	return NULL;
}

int MyXML::GetNesting()
{
	return m_nesting;
}

////////////////////////////
// return the next tag -
// starting tags only..<TAG>
// ignore terminating tags </TAG>
// white space is ignored
char *
MyXML::GetNextTag()
{
	char *result;

	do
	{
		result = Parse();
	} while (m_state != END_TAG && result != NULL);

	return result;
}

/////////////////////////////////////////
// return the next data field (not a tag)
// white space is NOT ignored
char *
MyXML::GetNextDataField()
{
	char *result;

	do
	{
		result = Parse();
	} while (m_state != BEGIN_TAG && result != NULL);

	return result;
}

////////////////////////////
// return the next field that matches
//	the tag parameter from the stream
//	or NULL if not found
//
char *
MyXML::GetNextTaggedField(char *tag)
{
	char *result;

	// find the tag we need
	result = GetNextTag(tag);

	// fetch the field
	if (m_state == END_TAG && result != NULL)
		result = GetNextDataField();

	return result;
}

////////////////////////////
// return the next tag that matches
//	the tag parameter
//	or NULL if not found
// used to position the stream pointer
//
char *
MyXML::GetNextTag(char *tag)
{
	char *result;
	if (!m_buf || (*m_buf == 0))
		return NULL;

	while (strcmp(m_buf, tag) != 0)
	{
		result = GetNextTag();
		if (result == NULL)
			break;
	}
	return result;
}
