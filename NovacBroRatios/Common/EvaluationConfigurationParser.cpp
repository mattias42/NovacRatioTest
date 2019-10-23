#include "EvaluationConfigurationParser.h"
// #include "Common.h"
#include "../Misc.h"
#include <memory>
#include <cstring>
#include <SpectralEvaluation/Configuration/DarkSettings.h>

using namespace FileHandler;
using namespace novac;

CEvaluationConfigurationParser::CEvaluationConfigurationParser()
{
}

int CEvaluationConfigurationParser::ReadConfigurationFile(const novac::CString &fileName, Configuration::CEvaluationConfiguration *settings, Configuration::CDarkCorrectionConfiguration *darkSettings) {

    // 1. Open the file
    if (!Open(fileName))
    {
        return FAIL;
    }

    // Parse the file
    this->szToken = NextToken();
    while (szToken != nullptr)
    {
        // no use to parse empty lines
        if (strlen(szToken) < 3)
        {
            this->szToken = NextToken();
            continue;
        }

        if (novac::Equals(szToken, "serial", 6)) {
            this->Parse_StringItem("/serial", settings->m_serial);

            this->szToken = NextToken();
            continue;
        }

        if (novac::Equals(szToken, "fitWindow", 9)) {
            Evaluation::CFitWindow tmpWindow;
            CDateTime validFrom, validTo;

            // Parse the fit window
            Parse_FitWindow(tmpWindow, validFrom, validTo);

            // Insert the fit-window into the list
            settings->InsertFitWindow(tmpWindow, &validFrom, &validTo);
        }

        if (novac::Equals(szToken, "DarkCorrection", 14)) {
            Configuration::CDarkSettings dSettings;
            CDateTime validFrom, validTo;

            // Parse the fit window
            Parse_DarkCorrection(dSettings, validFrom, validTo);

            // Insert the fit-window into the list
            darkSettings->InsertDarkCurrentCorrectionSettings(dSettings, &validFrom, &validTo);
        }

        this->szToken = NextToken();
    }
    Close();

    return 0;
}

int CEvaluationConfigurationParser::WriteConfigurationFile(const novac::CString &fileName, const Configuration::CEvaluationConfiguration *settings, const Configuration::CDarkCorrectionConfiguration *darkSettings) {
    novac::CString indent, str;
    Evaluation::CFitWindow window;
    Configuration::CDarkSettings dSettings;
    CDateTime from, to;

    // open the file
    FILE *f = fopen(fileName, "w");
    if (f == NULL)
        return 1;

    // write the header lines and the start of the file
    fprintf(f, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    fprintf(f, "<!-- This is the configuration file for the evaluation of spectra in the NOVAC Post Processing Program -->\n\n");
    fprintf(f, "<EvaluationConfiguration>\n");
    indent.Format("\t");

    // Write the serial-number of the spectrometer for which this configuration is valid
    fprintf(f, "\t<serial>%s</serial>\n", (const char*)settings->m_serial);

    // ------ loop through each of the fit windows and write them to file --------
    unsigned long nWindows = settings->GetFitWindowNum();
    for (unsigned int k = 0; k < nWindows; ++k) {
        settings->GetFitWindow(k, window, from, to);

        fprintf(f, "\t<fitWindow>\n");

        // the channel of the spectrometer
        fprintf(f, "\t\t<channel>%d</channel>\n", window.channel);

        // the name of the fit-window
        fprintf(f, "\t\t<name>%s</name>\n", window.name.c_str());

        // the time-range when the fit-window is valid
        fprintf(f, "\t\t<validFrom>%04d.%02d.%02d</validFrom>\n", from.year, from.month, from.day);
        fprintf(f, "\t\t<validTo>%04d.%02d.%02d</validTo>\n", to.year, to.month, to.day);

        // The size of the spectra and the interlace-steps
        fprintf(f, "\t\t<specLength>%d</specLength>\n", window.specLength);
        fprintf(f, "\t\t<interlaceStep>%d</interlaceStep>\n", window.interlaceStep);

        // the option for the polynomial to use
        fprintf(f, "\t\t<polyOrder>%d</polyOrder>\n", window.polyOrder);

        // the type of fit to use
        fprintf(f, "\t\t<fitType>%d</fitType>\n", window.fitType);

        // the boundaries of the fit (in pixels)
        fprintf(f, "\t\t<fitLow>%d</fitLow>\n", window.fitLow);
        fprintf(f, "\t\t<fitHigh>%d</fitHigh>\n", window.fitHigh);

        // If we should use a pre-calibrated solar-spectrum to calibrate
        //  the shift & squeeze of the spectra
        if (window.fraunhoferRef.m_path.size() > 3) {
            fprintf(f, "\t\t<wavelengthCalibration>\n");
            fprintf(f, "\t\t<fraunhoferSpec>%s</fraunhoferSpec>\n", window.fraunhoferRef.m_path.c_str());
            fprintf(f, "\t\t</wavelengthCalibration>\n");
        }

        // Each of the references...
        for (int j = 0; j < window.nRef; ++j) {
            fprintf(f, "\t\t<Reference>\n");
            fprintf(f, "\t\t\t<name>%s</name>\n", window.ref[j].m_specieName.c_str());
            fprintf(f, "\t\t\t<path>%s</path>\n", window.ref[j].m_path.c_str());

            // The value for the shift
            fprintf(f, "\t\t\t<shiftOption>%d</shiftOption>\n", window.ref[j].m_shiftOption);
            if (window.ref[j].m_shiftOption != Evaluation::SHIFT_FREE)
                fprintf(f, "\t\t\t<shiftValue>%lf</shiftValue>\n", window.ref[j].m_shiftValue);

            // The value for the squeeze
            fprintf(f, "\t\t\t<squeezeOption>%d</squeezeOption>\n", window.ref[j].m_shiftOption);
            if (window.ref[j].m_shiftOption != Evaluation::SHIFT_FREE)
                fprintf(f, "\t\t\t<squeezeValue>%lf</squeezeValue>\n", window.ref[j].m_shiftValue);

            // The value for the column
            fprintf(f, "\t\t\t<columnOption>%d</columnOption>\n", window.ref[j].m_columnOption);
            if (window.ref[j].m_columnOption != Evaluation::SHIFT_FREE)
                fprintf(f, "\t\t\t<columnValue>%lf</columnValue>\n", window.ref[j].m_columnValue);

            fprintf(f, "\t\t</Reference>\n");
        }

        fprintf(f, "\t</fitWindow>\n");
    }

    // ------ loop through each of the dark-current settings and write them to file --------
    for (unsigned int k = 0; k < nWindows; ++k) {
        darkSettings->GetDarkSettings(k, dSettings, from, to);

        fprintf(f, "\t<DarkCorrection>\n");

        // the time-range when the dark-current settings is valid
        fprintf(f, "\t\t<validFrom>%04d.%02d.%02d</validFrom>\n", from.year, from.month, from.day);
        fprintf(f, "\t\t<validTo>%04d.%02d.%02d</validTo>\n", to.year, to.month, to.day);

        if (dSettings.m_darkSpecOption == Configuration::DARK_SPEC_OPTION::MEASURED_IN_SCAN) {
            // only use a dark-spectrum with the same exp.-time
            fprintf(f, "\t\t<dark>SCAN</dark>\n");
        }
        else if (dSettings.m_darkSpecOption == Configuration::DARK_SPEC_OPTION::MODEL_ALWAYS) {
            // always model the dark-spectrum
            fprintf(f, "\t\t<dark>MODEL</dark>\n");

            // dark-current
            if (dSettings.m_darkCurrentOption == Configuration::DARK_MODEL_OPTION::MEASURED_IN_SCAN) {
                fprintf(f, "\t\t<darkCurrent>SCAN</darkCurrent>\n");
            }
            else {
                fprintf(f, "\t\t<darkCurrent>USER</darkCurrent>\n");
                fprintf(f, "\t\t<darkCurrentSpec>%s</darkCurrentSpec>\n", dSettings.m_darkCurrentSpec.c_str());
            }

            // offset
            if (dSettings.m_offsetOption == Configuration::DARK_MODEL_OPTION::MEASURED_IN_SCAN) {
                fprintf(f, "\t\t<offset>SCAN</offset>\n");
            }
            else {
                fprintf(f, "\t\t<offset>USER</offset>\n");
                fprintf(f, "\t\t<offsetSpec>%s</offsetSpec>\n", dSettings.m_offsetSpec.c_str());
            }

        }
        else if (dSettings.m_darkSpecOption == Configuration::DARK_SPEC_OPTION::USER_SUPPLIED) {
            fprintf(f, "\t\t<dark>USER</dark>\n");
            fprintf(f, "\t\t<darkCurrentSpec>%s</darkCurrentSpec>\n", dSettings.m_darkCurrentSpec.c_str());
            fprintf(f, "\t\t<offsetSpec>%s</offsetSpec>\n", dSettings.m_offsetSpec.c_str());
        }

        fprintf(f, "\t</DarkCorrection>\n");
    }

    fprintf(f, "</EvaluationConfiguration>\n");

    // remember to close the file when we're done
    fclose(f);

    return 0;
}

void SaveSlitFunctionAndWavelengthCalibration(Evaluation::CFitWindow &window, novac::CString& slitfunctionFile, novac::CString& wavelengthCalibFile)
{
    if (slitfunctionFile.GetLength() > 0)
    {
        for (int ii = 0; ii < window.nRef; ++ii)
        {
            window.ref[ii].m_slitFunctionFile = slitfunctionFile.ToStdString();
        }
    }
    if (wavelengthCalibFile.GetLength() > 0)
    {
        for (int ii = 0; ii < window.nRef; ++ii)
        {
            window.ref[ii].m_wavelengthCalibrationFile = wavelengthCalibFile.ToStdString();
        }
    }
}

int CEvaluationConfigurationParser::Parse_FitWindow(Evaluation::CFitWindow &window, CDateTime &validFrom, CDateTime &validTo) {
    window.Clear();
    novac::CString slitfunctionFile, wavelengthCalibFile;

    // Parse the file
    this->szToken = NextToken();
    while (szToken != nullptr)
    {
        // no use to parse empty lines
        if (strlen(szToken) < 2)
        {
            this->szToken = NextToken();
            continue;
        }

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            this->szToken = NextToken();
            continue;
        }

        // end of fit-window section
        if (Equals(szToken, "/fitWindow")) {
            SaveSlitFunctionAndWavelengthCalibration(window, slitfunctionFile, wavelengthCalibFile);
            return 0;
        }

        if (Equals(szToken, "fitWindow"))
        {
            Evaluation::CFitWindow child;
            CDateTime childValidFrom, childValidTo;
            Parse_FitWindow(child, childValidFrom, childValidTo);
            window.child.push_back(child);

            this->szToken = NextToken();
            continue;
        }


        if (Equals(szToken, "name")) {
            Parse_StringItem("/name", window.name);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "validFrom")) {
            Parse_Date("/validFrom", validFrom);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "validTo")) {
            Parse_Date("/validTo", validTo);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "fitLow")) {
            Parse_IntItem("/fitLow", window.fitLow);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "fitHigh")) {
            Parse_IntItem("/fitHigh", window.fitHigh);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "polyOrder")) {
            Parse_IntItem("/polyOrder", window.polyOrder);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "fitType")) {
            Parse_IntItem("/fitType", (int&)window.fitType); // TODO: Will this be ok????

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "channel")) {
            Parse_IntItem("/channel", window.channel);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "specLength")) {
            Parse_IntItem("/specLength", window.specLength);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "fOptShift")) {
            int flagToParse = 0;
            Parse_IntItem("/fOptShift", flagToParse);
            window.findOptimalShift = (flagToParse > 0);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "shiftSky")) {
            int flagToParse = 0;
            Parse_IntItem("/shiftSky", flagToParse);
            window.shiftSky = (flagToParse > 0);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "interlaceStep")) {
            Parse_IntItem("/interlaceStep", window.interlaceStep);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "interlaced")) {
            Parse_IntItem("/interlaced", window.interlaceStep);
            window.interlaceStep += 1;

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "fraunhoferSpec", 14)) {
            // Parse the settings for the wavelength calibration
            this->Parse_PathItem("/fraunhoferSpec", window.fraunhoferRef.m_path);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "slitFunction")) {
            // This is the path to a reference which needs to be convolved before we can continue.
            Parse_PathItem("/slitFunction", slitfunctionFile);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "wavlengthCalibration")) {
            // This is the path to a reference which needs to be convolved before we can continue.
            Parse_PathItem("/wavlengthCalibration", wavelengthCalibFile);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "Reference", 9)) {
            Parse_Reference(window);

            this->szToken = NextToken();
            continue;
        }

        this->szToken = NextToken();
    }

    return 1;
}

int CEvaluationConfigurationParser::Parse_Reference(Evaluation::CFitWindow &window) {
    int nRef = window.nRef;

    // Parse the file
    this->szToken = NextToken();
    while (szToken != nullptr)
    {
        // no use to parse empty lines
        if (strlen(szToken) < 3)
        {
            this->szToken = NextToken();
            continue;
        }

        // ignore comments
        if (Equals(szToken, "!--", 3))
        {
            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "/Reference")) {
            ++window.nRef;
            return 0;
        }

        if (Equals(szToken, "name")) {
            Parse_StringItem("/name", window.ref[nRef].m_specieName);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "filtered")) {
            novac::CString str;
            Parse_StringItem("/filtered", str);
            if (Equals(str, "HP500")) {
                window.ref[nRef].m_isFiltered = true;
            }

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "path")) {
            // This is the path to a pre-convolved reference. Just read the path and read the reference from there.
            Parse_PathItem("/path", window.ref[nRef].m_path);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "crossSection")) {
            // This is the path to a reference which needs to be convolved before we can continue.
            Parse_PathItem("/crossSection", window.ref[nRef].m_crossSectionFile);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "shiftOption")) {
            int tmpInt;
            Parse_IntItem("/shiftOption", tmpInt);
            switch (tmpInt) {
            case 0: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_FREE; break;
            case 1: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_FIX; break;
            case 2: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_LINK; break;
            case 3: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_LIMIT; break;
            }

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "shiftValue")) {
            Parse_FloatItem("/shiftValue", window.ref[nRef].m_shiftValue);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "squeezeOption")) {
            int tmpInt;
            Parse_IntItem("/squeezeOption", tmpInt);
            switch (tmpInt) {
            case 0: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_FREE; break;
            case 1: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_FIX; break;
            case 2: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_LINK; break;
            case 3: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_LIMIT; break;
            }

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "squeezeValue")) {
            Parse_FloatItem("/squeezeValue", window.ref[nRef].m_squeezeValue);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "columnOption")) {
            int tmpInt;
            Parse_IntItem("/columnOption", tmpInt);
            switch (tmpInt) {
            case 0: window.ref[nRef].m_columnOption = Evaluation::SHIFT_FREE; break;
            case 1: window.ref[nRef].m_columnOption = Evaluation::SHIFT_FIX; break;
            case 2: window.ref[nRef].m_columnOption = Evaluation::SHIFT_LINK; break;
            case 3: window.ref[nRef].m_columnOption = Evaluation::SHIFT_LIMIT; break;
            }

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "columnValue")) {
            Parse_FloatItem("/columnValue", window.ref[nRef].m_columnValue);

            this->szToken = NextToken();
            continue;
        }

        this->szToken = NextToken();
    }

    return FAIL;
}

int CEvaluationConfigurationParser::Parse_DarkCorrection(Configuration::CDarkSettings &dSettings, CDateTime &validFrom, CDateTime &validTo) {
    dSettings.Clear();
    novac::CString str;

    // Parse the file
    this->szToken = NextToken();
    while (szToken != nullptr)
    {
        // no use to parse empty lines
        if (strlen(szToken) < 2)
        {
            this->szToken = NextToken();
            continue;
        }

        // ignore comments
        if (Equals(szToken, "!--", 3))
        {
            this->szToken = NextToken();
            continue;
        }

        // end of dark-correction section
        if (Equals(szToken, "/DarkCorrection")) {
            return 0;
        }

        // valid interval
        if (Equals(szToken, "validFrom")) {
            Parse_Date("/validFrom", validFrom);

            this->szToken = NextToken();
            continue;
        }
        if (Equals(szToken, "validTo")) {
            Parse_Date("/validTo", validTo);

            this->szToken = NextToken();
            continue;
        }

        // the option for the dark
        if (Equals(szToken, "dark")) {
            Parse_StringItem("/dark", str);

            if (Equals(str, "MODEL")) {
                dSettings.m_darkSpecOption = Configuration::DARK_SPEC_OPTION::MODEL_ALWAYS;
            }
            else if (Equals(str, "USER")) {
                dSettings.m_darkSpecOption = Configuration::DARK_SPEC_OPTION::USER_SUPPLIED;
            }
            else {
                dSettings.m_darkSpecOption = Configuration::DARK_SPEC_OPTION::MEASURED_IN_SCAN;
            }

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "darkCurrentSpec")) {
            Parse_StringItem("/darkCurrentSpec", dSettings.m_darkCurrentSpec);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "darkCurrent")) {
            Parse_StringItem("/darkCurrent", str);

            if (Equals(str, "USER")) {
                dSettings.m_darkCurrentOption = Configuration::DARK_MODEL_OPTION::USER_SUPPLIED;
            }
            else {
                dSettings.m_darkCurrentOption = Configuration::DARK_MODEL_OPTION::MEASURED_IN_SCAN;
            }

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "offsetSpec")) {
            Parse_StringItem("/offsetSpec", dSettings.m_offsetSpec);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, "offset")) {
            Parse_StringItem("/offset", str);

            if (Equals(str, "USER")) {
                dSettings.m_offsetOption = Configuration::DARK_MODEL_OPTION::USER_SUPPLIED;
            }
            else {
                dSettings.m_offsetOption = Configuration::DARK_MODEL_OPTION::MEASURED_IN_SCAN;
            }

            this->szToken = NextToken();
            continue;
        }

        this->szToken = NextToken();
    }

    return 1;
}

