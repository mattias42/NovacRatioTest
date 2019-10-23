#include <PPPLib/CFileUtils.h>
#include <algorithm>
#include <cstring>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

namespace novac
{
	void CFileUtils::GetFileName(CString& fileName)
	{
		// look for slashes in the path
		int position = std::max(fileName.ReverseFind('\\'), fileName.ReverseFind('/'));
		int length = (int)fileName.GetLength();
		fileName = fileName.Right(length - position - 1);
	}

	void CFileUtils::GetDirectory(CString &fileName) {
		int position = fileName.ReverseFind('\\');
		if (position >= 0)
		{
			fileName = fileName.Left(position + 1);
		}
	}

	bool CFileUtils::GetInfoFromFileName(const CString fileName, CDateTime &start, CString &serial, int &channel, MEASUREMENT_MODE &mode)
	{
		CString name, sDate, sTime, resToken;
		int iDate, iTime;
		int curPos = 0;

		// set to default values
		start = CDateTime();
		serial = "";
		channel = 0;
		mode = MODE_FLUX;

		// make a local copy of the filename
		name.Format(fileName);

		// remove the name of the path
		GetFileName(name);

		// Tokenize the file-name using the underscores as separators
		resToken = name.Tokenize("_", curPos);
		if (resToken == "")
			return false;
		serial.Format(resToken);

		if (curPos == -1)
			return false;

		// The second part is the date
		resToken = name.Tokenize("_", curPos);
		if (resToken == "")
			return false;
		sscanf(resToken, "%d", &iDate);
		start.year = (unsigned char)(iDate / 10000);
		start.month = (unsigned char)((iDate - start.year * 10000) / 100);
		start.day = (unsigned char)(iDate % 100);
		start.year += 2000;

		if (curPos == -1)
			return false;

		// The third part is the time
		resToken = name.Tokenize("_", curPos);
		if (resToken == "")
			return false;
		sscanf(resToken, "%d", &iTime);
		start.hour = (unsigned char)(iTime / 100);
		start.minute = (unsigned char)((iTime - start.hour * 100));
		start.second = 0;

		if (curPos == -1)
			return false;

		// The fourth part is the channel
		resToken = name.Tokenize("_", curPos);
		if (resToken == "")
			return false;
		sscanf(resToken, "%d", &channel);

		if (curPos == -1)
			return true;

		// The fifth part is the measurement mode. This is however not always available...
		resToken = name.Tokenize("_", curPos);
		if (resToken == "")
			return false;
		if (Equals(resToken, "flux", 4)) {
			mode = MODE_FLUX;
		}
		else if (Equals(resToken, "wind", 4)) {
			mode = MODE_WINDSPEED;
		}
		else if (Equals(resToken, "stra", 4)) {
			mode = MODE_STRATOSPHERE;
		}
		else if (Equals(resToken, "dsun", 4)) {
			mode = MODE_DIRECT_SUN;
		}
		else if (Equals(resToken, "comp", 4)) {
			mode = MODE_COMPOSITION;
		}
		else if (Equals(resToken, "luna", 4)) {
			mode = MODE_LUNAR;
		}
		else if (Equals(resToken, "trop", 4)) {
			mode = MODE_TROPOSPHERE;
		}
		else if (Equals(resToken, "maxd", 4)) {
			mode = MODE_MAXDOAS;
		}
		else if (Equals(resToken, "unkn", 4)) {
			mode = MODE_UNKNOWN;
		}
		else {
			mode = MODE_UNKNOWN;
		}

		return true;
	}

	bool CFileUtils::IsIncompleteFile(const novac::CString& fileName)
	{
		if (strstr((const char*)fileName, "Incomplete")) {
			return true;
		}
		if (novac::Equals(fileName, "Upload.pak")) {
			return true;
		}
		return false;
	}
}  // namespace novac

#ifdef _MSC_VER
#pragma warning(pop)
#endif
