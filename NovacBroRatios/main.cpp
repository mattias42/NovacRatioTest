#include <iostream>
#include <PPPLib/CString.h>
#include "Configuration/NovacPPPConfiguration.h"
#include "SetupFileReader.h"
#include "Common/EvaluationConfigurationParser.h"
#include "Common/ProcessingFileReader.h"

// The global configuration object
Configuration::CNovacPPPConfiguration g_setup;

void LoadConfigurations()
{
    // Declaration of variables and objects
    novac::CString setupPath = "D:\\Development\\NovacRatioTest\\Configuration\\setup.xml";
    novac::CString processingPath = "D:\\Development\\NovacRatioTest\\Configuration\\processing.xml";

    FileHandler::CSetupFileReader reader;
    FileHandler::CEvaluationConfigurationParser eval_reader;
    FileHandler::CProcessingFileReader processing_reader;

    // Read configuration from file setup.xml */	
    // setupPath.Format("%sconfiguration%csetup.xml", (const char*)common.m_exePath, Poco::Path::separator());
    if (SUCCESS != reader.ReadSetupFile(setupPath, g_setup))
    {
        throw std::logic_error("Could not read setup.xml. Setup not complete. Please fix and try again");
    }
    ShowMessage(novac::CString::FormatString(" Parsed %s, %d instruments found.", setupPath.c_str(), g_setup.m_instrumentNum));


    // Read the users options from file processing.xml
    // processingPath.Format("%sconfiguration%cprocessing.xml", (const char*)common.m_exePath, Poco::Path::separator());
    if (SUCCESS != processing_reader.ReadProcessingFile(processingPath, g_userSettings)) {
        throw std::logic_error("Could not read processing.xml. Setup not complete. Please fix and try again");
    }

    // Check if there is a configuration file for every spectrometer serial number
    for (unsigned int k = 0; k < g_setup.m_instrumentNum; ++k) {
        evalConfPath.Format("%sconfiguration%c%s.exml", (const char*)common.m_exePath, Poco::Path::separator(), (const char*)g_setup.m_instrument[k].m_serial);

        if (IsExistingFile(evalConfPath))
            eval_reader.ReadConfigurationFile(evalConfPath, &g_setup.m_instrument[k].m_eval, &g_setup.m_instrument[k].m_darkCurrentCorrection);
        else {
            throw std::logic_error("Could not find configuration file: " + evalConfPath);
        }
    }
}

int main()
{
    std::cout << "This is the test program for novac BrO/SO2 ratio evaluation " << std::endl;

    // Read the configuration files
    std::cout << " Loading configuration" << std::endl;
    LoadConfigurations();

}