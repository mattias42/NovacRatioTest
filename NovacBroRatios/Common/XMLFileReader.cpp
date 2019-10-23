#include "XMLFileReader.h"
// #include "../Common/Common.h"
#include "../Misc.h"

#include <cstdlib>
#include <cstring>
// #include <Poco/Path.h>

using namespace FileHandler;

CXMLFileReader::CXMLFileReader()
{
}

CXMLFileReader::~CXMLFileReader()
{
    m_tokenPt = nullptr;
    Close();
}

bool CXMLFileReader::Open(const novac::CString &fileName)
{
    novac::CFileException exceFile;

    this->m_File = new novac::CStdioFile();

    if (!m_File->Open(fileName, novac::CStdioFile::modeRead | novac::CStdioFile::typeText, &exceFile))
    {
        delete m_File;
        m_File = nullptr;
        return false;
    }

    this->m_filename = fileName;
    this->nLinesRead = 0;

    return true;
}

void CXMLFileReader::Close()
{
    if (nullptr != m_File)
    {
        m_File->Close();
        m_File = nullptr;
    }

    m_filename = "";
}


char *CXMLFileReader::NextToken() {
    char separators[] = "<>\t";
    szToken = nullptr;

    if (nLinesRead == 0) {
        // if this is the first call to this function
        m_File->ReadString(szLine, 4095);
        szToken = (char*)(LPCSTR)szLine;

        m_tokenPt = strtok(szToken, separators);
        ++nLinesRead;
        return m_tokenPt;
    }
    else {
        // this is not the first call to this function
        m_tokenPt = strtok(szToken, separators);
        if (nullptr != m_tokenPt) {
            return m_tokenPt;
        }

        szLine[0] = 0;
        while (0 == strlen(szLine)) {
            if (!m_File->ReadString(szLine, 4095)) {
                return nullptr;
            }
        }

        szToken = (char*)(LPCSTR)szLine;

        m_tokenPt = strtok(szToken, separators);
        ++nLinesRead;
    }

    if (nullptr == m_tokenPt || strlen(m_tokenPt) < 2) {
        return NextToken();
    }
    else {
        return m_tokenPt;
    }
}

const char *CXMLFileReader::GetAttributeValue(const novac::CString &label)
{
    novac::CString toSearchFor;

    if (nLinesRead == 0 || szToken == nullptr) {
        return nullptr; // we haven't started reading the file yet...
    }

    // make a copy of szToken, so that we can make changes to it...
    char copyOfToken[4096];
    sprintf(copyOfToken, "%s", szToken);

    // search for the attribute in szToken
    toSearchFor.Format("%s=\"", (const char*)label);
    char *pt_start = strstr(copyOfToken, toSearchFor);
    if (pt_start == nullptr) {
        return nullptr;
    }
    pt_start += toSearchFor.GetLength(); // point to the character after the double-quote

    // search for the ending double-quote
    char *pt_end = strstr(pt_start, "\"");
    if (pt_end == nullptr) {
        return nullptr;
    }
    *pt_end = '\0'; // make the string end at the position of the double-quote

    // now we have the value of the attribute	
    sprintf(attributeValue, "%s", pt_start);

    return attributeValue;
}

int CXMLFileReader::Parse_StringItem(const novac::CString &label, novac::CString &string) {
    string.Format("");

    while (nullptr != (szToken = NextToken())) {

        if (Equals(szToken, label))
        {
            return 1;
        }

        string.Format(szToken);
    }

    return 0;
}

int CXMLFileReader::Parse_StringItem(const novac::CString &label, std::string &string)
{
    string = "";

    while (nullptr != (szToken = NextToken())) {

        if (Equals(szToken, label))
        {
            return 1;
        }

        string = std::string(szToken);
    }

    return 0;
}


int CXMLFileReader::Parse_PathItem(const novac::CString &label, novac::CString &path)
{
    return Parse_StringItem(label, path);

    // int r = Parse_StringItem(label, path);
    // if (r != 0)
    // {
    //     std::string p = Poco::Path::expand(path.std_str());
    // 
    //     if (!novac::EqualsIgnoringCase(p, path.ToStdString()))
    //     {
    //         novac::CString message;
    //         message.Format("Expanded path: '%s' into '%s'", path.c_str(), p.c_str());
    //         ShowMessage(message);
    // 
    //         path.SetData(p);
    //     }
    //     return r;
    // }
    // else
    // {
    //     return 0;
    // }
}

int CXMLFileReader::Parse_PathItem(const novac::CString &label, std::string &path)
{
    return Parse_StringItem(label, path);
    // int r = Parse_StringItem(label, path);
    // if (r != 0)
    // {
    //     std::string p = Poco::Path::expand(path);
    // 
    //     if (!novac::EqualsIgnoringCase(p, path))
    //     {
    //         novac::CString message;
    //         message.Format("Expanded path: '%s' into '%s'", path.c_str(), p.c_str());
    //         ShowMessage(message);
    // 
    //         path = p;
    //     }
    //     return r;
    // }
    // else
    // {
    //     return 0;
    // }
}

int CXMLFileReader::Parse_LongItem(const novac::CString &label, long &number) {

    while (nullptr != (szToken = NextToken())) {

        if (Equals(szToken, label)) {
            return 1;
        }

        number = std::atol(szToken);
    }

    return 0;
}
/** General parsing of a single, simple float item */
int CXMLFileReader::Parse_FloatItem(const novac::CString &label, double &number) {
    while (nullptr != (szToken = NextToken())) {

        if (Equals(szToken, label)) {
            return 1;
        }

        number = std::atof(szToken);
    }

    return 0;
}

/** General parsing of a single, simple integer item */
int CXMLFileReader::Parse_IntItem(const novac::CString &label, int &number) {
    while (nullptr != (szToken = NextToken())) {

        if (Equals(szToken, label)) {
            return 1;
        }


        number = std::atoi(szToken);
    }

    return 0;
}

int CXMLFileReader::Parse_IPNumber(const novac::CString &label, BYTE &ip0, BYTE &ip1, BYTE &ip2, BYTE &ip3) {
    while (nullptr != (szToken = NextToken())) {
        int i0, i1, i2, i3;

        if (Equals(szToken, label)) {
            return 1;
        }

        sscanf(szToken, "%d.%d.%d.%d", &i0, &i1, &i2, &i3);
        ip0 = (BYTE)i0;
        ip1 = (BYTE)i1;
        ip2 = (BYTE)i2;
        ip3 = (BYTE)i3;
    }

    return 0;
}
/** General parsing of a date */
int CXMLFileReader::Parse_Date(const novac::CString &label, CDateTime &datum) {
    int nFields = 0;

    while (nullptr != (szToken = NextToken())) {
        int i0 = 0;
        int i1 = 0;
        int i2 = 0;
        int i3 = 0;
        int i4 = 0;
        int i5 = 0;

        if (Equals(szToken, label)) {
            return 1;
        }

        char *pt = strstr(szToken, "T");

        if (pt == nullptr) {
            nFields = sscanf(szToken, "%d.%d.%d", &i0, &i1, &i2);
            datum.year = (unsigned short)i0;
            datum.month = (unsigned char)i1;
            datum.day = (unsigned char)i2;
        }
        else {
            nFields = sscanf(szToken, "%d.%d.%dT%d:%d:%d", &i0, &i1, &i2, &i3, &i4, &i5);
            datum.year = (unsigned short)i0;
            datum.month = (unsigned char)i1;
            datum.day = (unsigned char)i2;
            datum.hour = (unsigned char)i3;
            datum.minute = (unsigned char)i4;
            datum.second = (unsigned char)i5;
        }

        if (nFields == 0) {
            // if the normal parsing didn't work, then try also to parse functional expressions...
            CDateTime::ParseDate(szToken, datum);
        }
    }

    return 0;
}
