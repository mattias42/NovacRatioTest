#pragma once
#include "Common/XMLFileReader.h"
#include "Configuration/NovacPPPConfiguration.h"

#include <PPPLib/CString.h>
#include <PPPLib/CStdioFile.h>

namespace FileHandler {

    /** The class <b>CSetupFileReader</b> is used to read and write the setup
        files for NovacPPP. The setup files contains the setup data for
        the instruments, such as site-name, lat&long of the position
        and the cone-angle of the instrument. */

    class CSetupFileReader : public CXMLFileReader
    {
    public:
        CSetupFileReader(void);
        ~CSetupFileReader(void);

        /** This reads in the contents of a setup-file into the supplied data-structure.
            @param fileName - the name of the file to read from. This must be a .txt file
                in the correct format!
            @param setup - will on successful parsing of the file contain the setup-information
                found in the file.
            @return SUCCESS - if successful.
        */
        RETURN_CODE ReadSetupFile(const novac::CString &fileName, Configuration::CNovacPPPConfiguration &setup);

        /** This takes care of writing the contents of a setup data-structure to file
            Only the part regarding the instrument's location will be written to the file */
        RETURN_CODE WriteSetupFile(const novac::CString &fileName, const Configuration::CNovacPPPConfiguration &setup);

    private:
        /** Parses an individual location section */
        void Parse_Location(Configuration::CLocationConfiguration &loc);

        /** Parses an individual instrument section */
        void Parse_Instrument(Configuration::CInstrumentConfiguration &instr);

        /** Parses a custom spectrometer model */
        void Parse_CustomSpectrometer(SpectrometerModel& model);
    };
}