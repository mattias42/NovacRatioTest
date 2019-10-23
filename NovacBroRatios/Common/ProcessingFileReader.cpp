#include "ProcessingFileReader.h"
#include <PPPLib/VolcanoInfo.h>
#include <cstring>

#include <algorithm>

extern novac::CVolcanoInfo g_volcanoes;   // <-- A list of all known volcanoes

#undef min
#undef max

using namespace FileHandler;
using namespace novac;

CProcessingFileReader::CProcessingFileReader()
{
}

RETURN_CODE CProcessingFileReader::ReadProcessingFile(const novac::CString &filename, Configuration::CUserConfiguration &settings) {

    // 1. Open the file
    if (!Open(filename)) {
        return FAIL;
    }

    this->szToken = NextToken();
    while (szToken != nullptr)
    {
        // no use to parse empty lines, don't parse lines with less than 3 characters
        if (strlen(szToken) < 3)
        {
            this->szToken = NextToken();
            continue;
        }

        // If we've found the output directory
        if (Equals(szToken, str_outputDirectory, strlen(str_outputDirectory))) {
            Parse_PathItem(ENDTAG(str_outputDirectory), settings.m_outputDirectory);
            this->szToken = NextToken();
            continue;
        }

        // If we've found the temporary directory
        if (Equals(szToken, str_tempDirectory, strlen(str_tempDirectory))) {
            Parse_PathItem(ENDTAG(str_tempDirectory), settings.m_tempDirectory);
            this->szToken = NextToken();
            continue;
        }

        // If we've found the maximum number of allowed threads
        if (Equals(szToken, str_maxThreadNum, strlen(str_maxThreadNum))) {
            int number = 1;
            Parse_IntItem(ENDTAG(str_maxThreadNum), number);
            settings.m_maxThreadNum = (unsigned long)std::max(1, number);
            this->szToken = NextToken();
            continue;
        }

        // If we've found the beginning date
        if (Equals(szToken, str_fromDate, strlen(str_fromDate))) {
            Parse_Date(ENDTAG(str_fromDate), settings.m_fromDate);
            this->szToken = NextToken();
            continue;
        }

        // If we've found the end date
        if (Equals(szToken, str_toDate, strlen(str_toDate))) {
            Parse_Date(ENDTAG(str_toDate), settings.m_toDate);
            this->szToken = NextToken();
            continue;
        }

        // Look for the volcano to parse
        if (Equals(szToken, str_volcano, strlen(str_volcano))) {
            novac::CString code;
            Parse_StringItem(ENDTAG(str_volcano), code);
            settings.m_volcano = g_volcanoes.GetVolcanoIndex(code);
            this->szToken = NextToken();
            continue;
        }

        //* Look for the xml tag 'instrument' and use Parse_Instrument and Parse_Location to read serial number and location to object 'settings' */
        if (Equals(szToken, "FitWindows", 10)) {
            this->Parse_FitWindow(settings);
            this->szToken = NextToken();
            continue;
        }

        // If we've found the mode
        // if (Equals(szToken, str_processingMode, strlen(str_processingMode))) {
        //     novac::CString modeStr;
        //     Parse_StringItem(ENDTAG(str_processingMode), modeStr);
        //     if (Equals(modeStr, "composition")) {
        //         settings.m_processingMode = PROCESSING_MODE_COMPOSITION;
        //     }
        //     else if (Equals(modeStr, "stratosphere")) {
        //         settings.m_processingMode = PROCESSING_MODE_STRATOSPHERE;
        //     }
        //     else if (Equals(modeStr, "troposphere")) {
        //         settings.m_processingMode = PROCESSING_MODE_TROPOSPHERE;
        //     }
        //     else if (Equals(modeStr, "geometry")) {
        //         settings.m_processingMode = PROCESSING_MODE_GEOMETRY;
        //     }
        //     else {
        //         settings.m_processingMode = PROCESSING_MODE_FLUX;
        //     }
        //     continue;
        // }

        if (Equals(szToken, str_doEvaluations, strlen(str_doEvaluations))) {
            novac::CString boolStr;
            Parse_StringItem(ENDTAG(str_doEvaluations), boolStr);
            settings.m_doEvaluations = !Equals(boolStr, "false"); // this is better than 'Equals(boolStr, "true") since any other string the user may have entered (wrongly) is ignored
            this->szToken = NextToken();
            continue;
        }

        // If we've found the main gas
        if (Equals(szToken, str_molecule, strlen(str_molecule))) {
            novac::CString molecStr;
            Parse_StringItem(ENDTAG(str_molecule), molecStr);
            if (Equals(molecStr, "BrO")) {
                settings.m_molecule = MOLEC_BRO;
            }
            else if (Equals(molecStr, "NO2")) {
                settings.m_molecule = MOLEC_NO2;
            }
            else if (Equals(molecStr, "O3")) {
                settings.m_molecule = MOLEC_O3;
            }
            else {
                settings.m_molecule = MOLEC_SO2;
            }

            this->szToken = NextToken();
            continue;
        }

        // If we've found the wind field file to use
        if (Equals(szToken, str_windFieldFile, strlen(str_windFieldFile))) {
            Parse_PathItem(ENDTAG(str_windFieldFile), settings.m_windFieldFile);

            this->szToken = NextToken();
            continue;
        }

        if (Equals(szToken, str_windFieldFileOption, strlen(str_windFieldFileOption))) {
            Parse_IntItem(ENDTAG(str_windFieldFileOption), settings.m_windFieldFileOption);

            this->szToken = NextToken();
            continue;
        }

        // If we've found the local directory where to search for data
        if (Equals(szToken, str_LocalDirectory, strlen(str_LocalDirectory))) {
            Parse_PathItem(ENDTAG(str_LocalDirectory), settings.m_LocalDirectory);

            this->szToken = NextToken();
            continue;
        }

        // If we've found the local directory where to search for data
        if (Equals(szToken, str_includeSubDirectories_Local, strlen(str_includeSubDirectories_Local))) {
            Parse_IntItem(ENDTAG(str_includeSubDirectories_Local), settings.m_includeSubDirectories_Local);

            this->szToken = NextToken();
            continue;
        }

        // If we've found the FTP directory where to search for data
        if (Equals(szToken, str_FTPDirectory, strlen(str_FTPDirectory))) {
            Parse_StringItem(ENDTAG(str_FTPDirectory), settings.m_FTPDirectory);

            this->szToken = NextToken();
            continue;
        }

        // If we've found the FTP username
        if (Equals(szToken, str_FTPUsername, strlen(str_FTPUsername))) {
            Parse_StringItem(ENDTAG(str_FTPUsername), settings.m_FTPUsername);

            this->szToken = NextToken();
            continue;
        }

        // If we've found the FTP password
        if (Equals(szToken, str_FTPPassword, strlen(str_FTPPassword))) {
            Parse_StringItem(ENDTAG(str_FTPPassword), settings.m_FTPPassword);

            this->szToken = NextToken();
            continue;
        }

        // If we've found the FTP password
        if (Equals(szToken, str_includeSubDirectories_FTP, strlen(str_includeSubDirectories_FTP))) {
            Parse_IntItem(ENDTAG(str_includeSubDirectories_FTP), settings.m_includeSubDirectories_FTP);

            this->szToken = NextToken();
            continue;
        }

        // If we should upload the results to the NovacFTP server at the end...
        if (Equals(szToken, str_uploadResults, strlen(str_uploadResults))) {
            Parse_IntItem(ENDTAG(str_uploadResults), settings.m_uploadResults);

            this->szToken = NextToken();
            continue;
        }

        // If we've found the settings for the geometry calculations
        if (Equals(szToken, "GeometryCalc", 12)) {
            this->Parse_GeometryCalc(settings);

            this->szToken = NextToken();
            continue;
        }

        // If we've found the settings for the dual-beam calculations
        if (Equals(szToken, "DualBeam", 12)) {
            this->Parse_DualBeam(settings);

            this->szToken = NextToken();
            continue;
        }

        // If we've found the settings for when to discard spectra
        if (Equals(szToken, "Discarding", 10)) {
            this->Parse_DiscardSettings(settings);

            this->szToken = NextToken();
            continue;
        }

        this->szToken = NextToken();
    }//end while
    Close();

    return SUCCESS;
}

// Parse for serial tag and store in the InstrumentConfiguration object
void CProcessingFileReader::Parse_FitWindow(Configuration::CUserConfiguration &settings) {
    novac::CString fitWindowName, mainFitWindowName;
    int nFitWindowsFound = 0;

    // Parse the file
    this->szToken = NextToken();

    while (szToken != nullptr)
    {
        if (Equals(szToken, "/FitWindows", 11))
        {
            // set the index of the most important fit-window
            settings.m_mainFitWindow = 0;
            if (mainFitWindowName.GetLength() > 0) {
                for (int k = 0; k < settings.m_nFitWindowsToUse; ++k) {
                    if (Equals(settings.m_fitWindowsToUse[k], mainFitWindowName)) {
                        settings.m_mainFitWindow = k;
                        break;
                    }
                }
            }

            // set the number of fit-windows to use
            settings.m_nFitWindowsToUse = nFitWindowsFound;

            return;
        }

        if (Equals(szToken, "item", 6)) {
            // we've found another fit-window to use
            Parse_StringItem("/item", fitWindowName);
            if (settings.m_nFitWindowsToUse < MAX_FIT_WINDOWS) {
                settings.m_fitWindowsToUse[nFitWindowsFound].Format(fitWindowName);
                ++nFitWindowsFound;
            }

            szToken = NextToken();
            continue;
        }
        if (Equals(szToken, str_mainFitWindow, strlen(str_mainFitWindow))) {
            Parse_StringItem(ENDTAG(str_mainFitWindow), mainFitWindowName);
            szToken = NextToken();
            continue;
        }

        this->szToken = NextToken();
    }
}

/** Parses an individual geometry-calculation section */
void CProcessingFileReader::Parse_SkySpectrum(Configuration::CUserConfiguration &settings) {
    novac::CString option = novac::CString("option");
    novac::CString value = novac::CString("value");
    novac::CString parsedValueStr;
    novac::CString tmpString;

    // Parse the file
    this->szToken = NextToken();
    while (szToken != nullptr)
    {
        if (Equals(szToken, "/SkySpectrum", 12)) {
            if (settings.sky.skyOption == Configuration::SKY_OPTION::SPECTRUM_INDEX_IN_SCAN) {
                settings.sky.indexInScan = atoi(parsedValueStr);
            }
            else if (settings.sky.skyOption == Configuration::SKY_OPTION::USER_SUPPLIED) {
                settings.sky.skySpectrumFile = parsedValueStr.std_str();
            }

            return;
        }

        // we've found the minimum distance between two instruments that can be used to calculate
        //	a plume height
        if (Equals(szToken, option, option.GetLength())) {
            this->Parse_StringItem("/" + option, tmpString);
            if (Equals(tmpString, "SCAN")) {
                settings.sky.skyOption = Configuration::SKY_OPTION::MEASURED_IN_SCAN;
            }
            else if (Equals(tmpString, "AverageOfGood")) {
                settings.sky.skyOption = Configuration::SKY_OPTION::AVERAGE_OF_GOOD_SPECTRA_IN_SCAN;
            }
            else if (Equals(tmpString, "Index")) {
                settings.sky.skyOption = Configuration::SKY_OPTION::SPECTRUM_INDEX_IN_SCAN;
            }
            else if (Equals(tmpString, "User")) {
                settings.sky.skyOption = Configuration::SKY_OPTION::USER_SUPPLIED;
            }
            else {
                settings.sky.skyOption = Configuration::SKY_OPTION::MEASURED_IN_SCAN;
            }
 
            this->szToken = NextToken();
            continue;
        }

        // we've found the maximum distance between two instruments that can be used to calculate
        //	a plume height
        if (Equals(szToken, value, value.GetLength())) {
            this->Parse_StringItem("/" + value, parsedValueStr);
        }

        this->szToken = NextToken();
        continue;
    }
}

/** Parses an individual geometry-calculation section */
void CProcessingFileReader::Parse_GeometryCalc(Configuration::CUserConfiguration &settings) {

    // Parse the file
    this->szToken = NextToken();

    while (szToken != nullptr)
    {
        if (Equals(szToken, "/GeometryCalc", 13)) {
            return;
        }

        // we've found the completeness limit for the scans that can be used to calculate
        //	a plume height
        if (Equals(szToken, str_calcGeometry_CompletenessLimit, strlen(str_calcGeometry_CompletenessLimit))) {
            this->Parse_FloatItem(ENDTAG(str_calcGeometry_CompletenessLimit), settings.m_calcGeometry_CompletenessLimit);

            this->szToken = NextToken();
            continue;
        }

        // we've found the time each geometry-calculation is valid for (in seconds)
        if (Equals(szToken, str_calcGeometryValidTime, strlen(str_calcGeometryValidTime))) {
            this->Parse_IntItem(ENDTAG(str_calcGeometryValidTime), settings.m_calcGeometryValidTime);

            this->szToken = NextToken();
            continue;
        }

        // we've found the maximum difference in start-time between any two scans that can be combined
        if (Equals(szToken, str_calcGeometry_MaxTimeDifference, strlen(str_calcGeometry_MaxTimeDifference))) {
            this->Parse_IntItem(ENDTAG(str_calcGeometry_MaxTimeDifference), settings.m_calcGeometry_MaxTimeDifference);

            this->szToken = NextToken();
            continue;
        }

        // we've found the minimum distance between two instruments that can be used to calculate
        //	a plume height
        if (Equals(szToken, str_calcGeometry_MaxDistance, strlen(str_calcGeometry_MaxDistance))) {
            this->Parse_IntItem(ENDTAG(str_calcGeometry_MaxDistance), settings.m_calcGeometry_MaxDistance);

            this->szToken = NextToken();
            continue;
        }

        // we've found the minimum distance between two instruments that can be used to calculate
        //	a plume height
        if (Equals(szToken, str_calcGeometry_MinDistance, strlen(str_calcGeometry_MinDistance))) {
            this->Parse_IntItem(ENDTAG(str_calcGeometry_MinDistance), settings.m_calcGeometry_MinDistance);

            this->szToken = NextToken();
            continue;
        }

        // we've found the maximum tolerable error in the calculated plume altitude
        if (Equals(szToken, str_calcGeometry_MaxPlumeAltError, strlen(str_calcGeometry_MaxPlumeAltError))) {
            this->Parse_FloatItem(ENDTAG(str_calcGeometry_MaxPlumeAltError), settings.m_calcGeometry_MaxPlumeAltError);

            this->szToken = NextToken();
            continue;
        }

        // we've found the maximum tolerable error in the calculated plume altitude
        if (Equals(szToken, str_calcGeometry_MaxWindDirectionError, strlen(str_calcGeometry_MaxWindDirectionError))) {
            this->Parse_FloatItem(ENDTAG(str_calcGeometry_MaxWindDirectionError), settings.m_calcGeometry_MaxWindDirectionError);

            this->szToken = NextToken();
            continue;
        }

        this->szToken = NextToken();
    }
}

/** Parses an individual dual-beam section */
void CProcessingFileReader::Parse_DualBeam(Configuration::CUserConfiguration &settings) {

    // Parse the file
    this->szToken = NextToken();

    while (szToken != nullptr)
    {
        if (Equals(szToken, "/DualBeam", 13))
        {
            return;
        }

        // we've found the time each dual-beam is valid for (in seconds)
        if (Equals(szToken, str_dualBeam_ValidTime, strlen(str_dualBeam_ValidTime))) {
            this->Parse_IntItem(ENDTAG(str_dualBeam_ValidTime), settings.m_dualBeam_ValidTime);

            this->szToken = NextToken();
            continue;
        }

        // we've found the flag whether we should use the maximum test-length possible (or not)
        if (Equals(szToken, str_fUseMaxTestLength_DualBeam, strlen(str_fUseMaxTestLength_DualBeam))) {
            int tmpInt;
            this->Parse_IntItem(ENDTAG(str_fUseMaxTestLength_DualBeam), tmpInt);
            if (tmpInt)
                settings.m_fUseMaxTestLength_DualBeam = true;
            else
                settings.m_fUseMaxTestLength_DualBeam = false;

            this->szToken = NextToken();
            continue;
        }

        // we've found the maximum tolerable error in the calculated wind-speed
        if (Equals(szToken, str_dualBeam_MaxWindSpeedError, strlen(str_dualBeam_MaxWindSpeedError))) {
            this->Parse_FloatItem(ENDTAG(str_dualBeam_MaxWindSpeedError), settings.m_dualBeam_MaxWindSpeedError);

            this->szToken = NextToken();
            continue;
        }

        this->szToken = NextToken();
    }
}

/** Parses an individual quality-judgement section */
void CProcessingFileReader::Parse_DiscardSettings(Configuration::CUserConfiguration &settings) {

    // Parse the file
    this->szToken = NextToken();
    while (szToken != nullptr)
    {
        if (Equals(szToken, "/Discarding", 11)) {
            return;
        }

        // we've found the limit for the completeness of each scan
        if (Equals(szToken, str_completenessLimitFlux, strlen(str_completenessLimitFlux))) {
            this->Parse_FloatItem(ENDTAG(str_completenessLimitFlux), settings.m_completenessLimitFlux);

            this->szToken = NextToken();
            continue;
        }

        // we've found the minimum saturation ratio that we can have in the fit-region and
        //	still consider the spectrum worth evaluating
        if (Equals(szToken, str_minimumSaturationInFitRegion, strlen(str_minimumSaturationInFitRegion))) {
            this->Parse_FloatItem(ENDTAG(str_minimumSaturationInFitRegion), settings.m_minimumSaturationInFitRegion);

            this->szToken = NextToken();
            continue;
        }

        // we've found the maximum exposure time that we can have of the spectrum and
        //	still consider the spectrum worth evaluating
        if (Equals(szToken, str_maxExposureTime_got, strlen(str_maxExposureTime_got))) {
            this->Parse_IntItem(ENDTAG(str_maxExposureTime_got), settings.m_maxExposureTime_got);
        }
        if (Equals(szToken, str_maxExposureTime_hei, strlen(str_maxExposureTime_hei))) {
            this->Parse_IntItem(ENDTAG(str_maxExposureTime_hei), settings.m_maxExposureTime_hei);
        }

        this->szToken = NextToken();
    }
}

RETURN_CODE CProcessingFileReader::WriteProcessingFile(const novac::CString &fileName, const Configuration::CUserConfiguration &settings) {

    // try to open the file
    FILE *f = fopen(fileName, "w");
    if (f == NULL) {
        return FAIL;
    }

    fprintf(f, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    fprintf(f, "<!-- This is the configuration file for the processing of scans in the NOVAC Post Processing Program -->\n");

    fprintf(f, "<NovacPostProcessing>\n");

    PrintParameter(f, 1, str_maxThreadNum, settings.m_maxThreadNum);

    // the output and temp directories
    PrintParameter(f, 1, str_outputDirectory, settings.m_outputDirectory);
    PrintParameter(f, 1, str_tempDirectory, settings.m_tempDirectory);

    // the mode
    // switch (settings.m_processingMode) {
    // case PROCESSING_MODE_FLUX:			PrintParameter(f, 1, str_processingMode, "Flux"); break;
    // case PROCESSING_MODE_COMPOSITION:	PrintParameter(f, 1, str_processingMode, "Composition"); break;
    // case PROCESSING_MODE_STRATOSPHERE:	PrintParameter(f, 1, str_processingMode, "Stratosphere"); break;
    // case PROCESSING_MODE_TROPOSPHERE:	PrintParameter(f, 1, str_processingMode, "Troposphere"); break;
    // case PROCESSING_MODE_GEOMETRY:		PrintParameter(f, 1, str_processingMode, "Geometry"); break;
    // default:							PrintParameter(f, 1, str_processingMode, "Unknown"); break;
    // }

    // the most important molecule
    switch (settings.m_molecule) {
    case MOLEC_SO2:		fprintf(f, "\t<molecule>SO2</molecule>\n"); break;
    case MOLEC_O3:		fprintf(f, "\t<molecule>O3</molecule>\n"); break;
    case MOLEC_BRO:		fprintf(f, "\t<molecule>BRO</molecule>\n"); break;
    case MOLEC_NO2:		fprintf(f, "\t<molecule>NO2</molecule>\n"); break;
    case MOLEC_HCHO:	fprintf(f, "\t<molecule>HCHO</molecule>\n"); break;
    default: fprintf(f, "\t<molecule>Unknown</molecule>\n"); break;
    }

    // the volcano
    PrintParameter(f, 1, str_volcano, g_volcanoes.GetVolcanoCode(settings.m_volcano));

    // the time frame that we are looking for scans
    PrintParameter(f, 1, str_fromDate, settings.m_fromDate);
    PrintParameter(f, 1, str_toDate, settings.m_toDate);

    // the location of the .pak-files to use
    PrintParameter(f, 1, str_LocalDirectory, settings.m_LocalDirectory);
    PrintParameter(f, 1, str_includeSubDirectories_Local, settings.m_includeSubDirectories_Local);
    PrintParameter(f, 1, str_FTPDirectory, settings.m_FTPDirectory);
    PrintParameter(f, 1, str_includeSubDirectories_FTP, settings.m_includeSubDirectories_FTP);
    PrintParameter(f, 1, str_FTPUsername, settings.m_FTPUsername);
    PrintParameter(f, 1, str_FTPPassword, settings.m_FTPPassword);

    // Uploading of the results?
    PrintParameter(f, 1, str_uploadResults, settings.m_uploadResults);

    // the wind-field file
    PrintParameter(f, 1, str_windFieldFile, settings.m_windFieldFile);
    PrintParameter(f, 1, str_windFieldFileOption, settings.m_windFieldFileOption);

    // the settings for the geometry calculations
    fprintf(f, "\t<GeometryCalc>\n");
    PrintParameter(f, 2, str_calcGeometry_CompletenessLimit, settings.m_calcGeometry_CompletenessLimit);
    PrintParameter(f, 2, str_calcGeometryValidTime, settings.m_calcGeometryValidTime);
    PrintParameter(f, 2, str_calcGeometry_MaxTimeDifference, settings.m_calcGeometry_MaxTimeDifference);
    PrintParameter(f, 2, str_calcGeometry_MinDistance, settings.m_calcGeometry_MinDistance);
    PrintParameter(f, 2, str_calcGeometry_MaxDistance, settings.m_calcGeometry_MaxDistance);
    PrintParameter(f, 2, str_calcGeometry_MaxPlumeAltError, settings.m_calcGeometry_MaxPlumeAltError);
    PrintParameter(f, 2, str_calcGeometry_MaxWindDirectionError, settings.m_calcGeometry_MaxWindDirectionError);
    fprintf(f, "\t</GeometryCalc>\n");

    // the settings for the dual-beam wind speed calculations
    fprintf(f, "\t<DualBeam>\n");
    PrintParameter(f, 2, str_fUseMaxTestLength_DualBeam, settings.m_fUseMaxTestLength_DualBeam);
    PrintParameter(f, 2, str_dualBeam_MaxWindSpeedError, settings.m_dualBeam_MaxWindSpeedError);
    PrintParameter(f, 2, str_dualBeam_ValidTime, settings.m_dualBeam_ValidTime);
    fprintf(f, "\t</DualBeam>\n");

    // the fit fit windows to use
    fprintf(f, "\t<FitWindows>\n");
    for (int k = 0; k < settings.m_nFitWindowsToUse; ++k) {
        fprintf(f, "\t\t<item>%s</item>\n", (const char*)settings.m_fitWindowsToUse[k]);
    }
    fprintf(f, "\t\t<main>%s</main>\n", (const char*)settings.m_fitWindowsToUse[settings.m_mainFitWindow]);
    fprintf(f, "\t</FitWindows>\n");

    // the sky-spectrum to use
    fprintf(f, "\t<SkySpectrum>\n");
    if (settings.sky.skyOption == Configuration::SKY_OPTION::MEASURED_IN_SCAN) {
        fprintf(f, "\t\t<option>SCAN</option>\n");
    }
    else if (settings.sky.skyOption == Configuration::SKY_OPTION::AVERAGE_OF_GOOD_SPECTRA_IN_SCAN) {
        fprintf(f, "\t\t<option>AverageOfGood</option>\n");
    }
    else if (settings.sky.skyOption == Configuration::SKY_OPTION::SPECTRUM_INDEX_IN_SCAN) {
        fprintf(f, "\t\t<option>Index</option>\n");
        fprintf(f, "\t\t<value>%ld</value>\n", settings.sky.indexInScan);
    }
    else if (settings.sky.skyOption == Configuration::SKY_OPTION::USER_SUPPLIED) {
        fprintf(f, "\t\t<option>User</option>\n");
        fprintf(f, "\t\t<value>%s</value>\n", settings.sky.skySpectrumFile.c_str());
    }
    fprintf(f, "\t</SkySpectrum>\n");


    // settings for when to discard spectra/scans
    fprintf(f, "\t<Discarding>\n");
    PrintParameter(f, 2, str_completenessLimitFlux, settings.m_completenessLimitFlux);
    PrintParameter(f, 2, str_minimumSaturationInFitRegion, settings.m_minimumSaturationInFitRegion);
    PrintParameter(f, 2, str_maxExposureTime_got, settings.m_maxExposureTime_got);
    PrintParameter(f, 2, str_maxExposureTime_hei, settings.m_maxExposureTime_hei);
    fprintf(f, "\t</Discarding>\n");

    // finishing up
    fprintf(f, "</NovacPostProcessing>\n");

    // remember to close the file!
    fclose(f);

    return SUCCESS;
}

inline void PrintTabs(FILE *f, int nTabs) {
    // print the starting tabs
    if (nTabs == 1) {
        fprintf(f, "\t");
    }
    else if (nTabs == 2) {
        fprintf(f, "\t\t");
    }
    else {
        for (int k = 0; k < nTabs; ++k) {
            fprintf(f, "\t");
        }
    }
}

void CProcessingFileReader::PrintParameter(FILE *f, int nTabs, const novac::CString &tag, const novac::CString &value) {
    PrintTabs(f, nTabs);
    fprintf(f, "<%s>%s</%s>\n", (const char*)tag, (const char*)value, (const char*)tag);
    return;
}
void CProcessingFileReader::PrintParameter(FILE *f, int nTabs, const novac::CString &tag, const int &value) {
    PrintTabs(f, nTabs);
    fprintf(f, "<%s>%d</%s>\n", (const char*)tag, value, (const char*)tag);
    return;
}
void CProcessingFileReader::PrintParameter(FILE *f, int nTabs, const novac::CString &tag, const unsigned int &value) {
    PrintTabs(f, nTabs);
    fprintf(f, "<%s>%u</%s>\n", (const char*)tag, value, (const char*)tag);
    return;
}
void CProcessingFileReader::PrintParameter(FILE *f, int nTabs, const novac::CString &tag, const unsigned long &value) {
    PrintTabs(f, nTabs);
    fprintf(f, "<%s>%u</%s>\n", (const char*)tag, value, (const char*)tag);
    return;
}
void CProcessingFileReader::PrintParameter(FILE *f, int nTabs, const novac::CString &tag, const double &value) {
    PrintTabs(f, nTabs);
    fprintf(f, "<%s>%.2lf</%s>\n", (const char*)tag, value, (const char*)tag);
    return;
}
void CProcessingFileReader::PrintParameter(FILE *f, int nTabs, const novac::CString &tag, const CDateTime &value) {
    PrintTabs(f, nTabs);
    fprintf(f, "<%s>%04d.%02d.%02d</%s>\n", (const char*)tag, value.year, value.month, value.day, (const char*)tag);
    return;
}