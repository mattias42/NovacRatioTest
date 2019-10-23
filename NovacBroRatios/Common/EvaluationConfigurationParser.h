#pragma once

#include "XMLFileReader.h"
#include "../Configuration/EvaluationConfiguration.h"
#include "../Configuration/DarkCorrectionConfiguration.h"

namespace FileHandler {
    class CEvaluationConfigurationParser : public CXMLFileReader
    {
    public:
        CEvaluationConfigurationParser();

        /** Reads in an evaluation-configuration file.
            In the format specified for the NovacPostProcessingProgram (NPPP)
            @return 0 on sucess */
        int ReadConfigurationFile(const novac::CString &fileName, Configuration::CEvaluationConfiguration *settings, Configuration::CDarkCorrectionConfiguration *darkSettings);

        /** Writes an evaluation configuration file in the NPPP-format
            @return 0 on success */
        int WriteConfigurationFile(const novac::CString &fileName, const Configuration::CEvaluationConfiguration *settings, const Configuration::CDarkCorrectionConfiguration *darkSettings);

    private:

        /** Reads a 'fitWindow' section */
        int Parse_FitWindow(Evaluation::CFitWindow &window, CDateTime &validFrom, CDateTime &validTo);

        /** Reads a 'Reference' section */
        int Parse_Reference(Evaluation::CFitWindow &window);

        /** Reads a 'dark-correction' section */
        int Parse_DarkCorrection(Configuration::CDarkSettings &dSettings, CDateTime &validFrom, CDateTime &validTo);

    };
}
