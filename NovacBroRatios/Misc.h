#pragma once

#include <string>
#include <PPPLib/CString.h>

// defining if a function has failed or succeeded
enum RETURN_CODE { FAIL, SUCCESS };

// The list of instrument types available
enum INSTRUMENT_TYPE { INSTR_GOTHENBURG, INSTR_HEIDELBERG };

// The maximum number of fit windows that can be handled at any single time
#define MAX_FIT_WINDOWS 5

// The options for how to do the post-processing
enum PROCESSING_MODE {
    PROCESSING_MODE_FLUX
};

// ---------- helper methods defined elsewhere in NovacPPP ----------

void ShowMessage(const std::string& msg);
void ShowMessage(const char message[]);
void ShowMessage(const novac::CString &message);

int Equals(const std::string &str1, const std::string &str2);
