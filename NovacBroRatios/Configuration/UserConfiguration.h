#pragma once

#include <SpectralEvaluation/DateTime.h>
#include <SpectralEvaluation/Configuration/SkySettings.h>

// #include "../Common/Common.h"
#include "../Misc.h"
#include "../Molecule.h"

#include <PPPLib/CString.h>

namespace Configuration {
    /** The class <b>CUserConfiguration</b> stores the settings from the user
        on how the post-processing should be performed. This means settings for
        which mode we're supossed to run in (flux, composition...) or which
        of the defined fit-windows to use for the flux calculation or
        which time period to evaluate scans from...
    */

#define ENDTAG(x) ("/" + novac::CString(x))
#define FLAG(x) ("--" + novac::CString(x) + "=")

    class CUserConfiguration
    {
    public:
        CUserConfiguration();

        /** Restores all values to their defaults */
        void Clear();

        /** Compares to set of settings */
        bool operator==(const CUserConfiguration &settings2);

        // ------------------------------------------------------------------------
        // ------------------ GENERAL SETTINGS FOR THE SOFTWARE -------------------
        // ------------------------------------------------------------------------

        /** The maximum number of threads that we can split up a task into */
        unsigned long    m_maxThreadNum;
#define str_maxThreadNum "MaxThreadNum"


        /** The working-directory, used to override the location of the software.
                This can only be overriden in command line arguments, not the config file. */
#define str_workingDirectory "WorkDir"

        // ------------------------------------------------------------------------
        // ------------- IS THIS THE CONTINUATION OF A BROKEN RUN ? ---------------
        // ------------------------------------------------------------------------

        /** this is 'true' if this run is a continuation of an old processing
            run that was interrupted for some reason.
            This parameter is never written/read to/from file, it's queried
                to the user at startup if (and only if) we detect that an output
                directory already exists with the same settings that we want
                to use.

            If this is true, then the scan-files will not be evaluated again */
        bool m_fIsContinuation;


        // ------------------------------------------------------------------------
        // ------------------ THE LOCATION OF THE OUTPUT FILES  -------------------
        // ------------------------------------------------------------------------

        /** The directory that we should use for temporary files */
        novac::CString   m_tempDirectory;
#define   str_tempDirectory "tempdirectory"

        /** The directory that we should use to store the results */
        novac::CString   m_outputDirectory;
#define str_outputDirectory "outputdirectory"

        // ------------------------------------------------------------------------
        // --------------- WHAT WE WANT TO DO WITH THE PROCESSING  ----------------
        // ------------------------------------------------------------------------

        /** This determines the processing mode of the program. */
        PROCESSING_MODE  m_processingMode;
#define str_processingMode "mode"


        /** Set to false to disable spectrum evaluations and thus only re-calculating 
            the results from already produced evaluation-logs. */
        bool m_doEvaluations = true;
#define str_doEvaluations "doEvaluations"


        /** The molecule of main interest.
            This is the one the fluxes will be calculated for if the processing mode is 'flux' */
        STANDARD_MOLECULE m_molecule;
#define str_molecule "molecule"

        // ------------------------------------------------------------------------
        // ----------------- SETTINGS FOR THE VOLCANO TO PROCESS ------------------
        // ------------------------------------------------------------------------

        /** The volcano that we're processing at the moment.
            This is an index into the global array 'g_volcanoes'.
         */
        int     m_volcano;
#define    str_volcano "Volcano"

        // ------------------------------------------------------------------------
        // -------------- SETTINGS FOR THE TIME PERIOD TO PROCESS ----------------
        // ------------------------------------------------------------------------

        /** The first day that we should look for data (inclusive) */
        CDateTime  m_fromDate;
#define   str_fromDate "FromDate"

        /** The last day that we should look for data (inclusive) */
        CDateTime  m_toDate;
#define   str_toDate "ToDate"

        // ------------------------------------------------------------------------
        // ------- SETTINGS FOR THE LOCATION OF THE .PAK-FILES TO PROCESS ---------
        // ------------------------------------------------------------------------

        /** The path to a directory on the location computer which we should scan for
            data files. */
        novac::CString   m_LocalDirectory;
#define   str_LocalDirectory "LocalDirectory"

        /** This is non-zero if we should include sub-directories to 'm_LocalDirectory'
            in our search for data */
        int    m_includeSubDirectories_Local;
#define   str_includeSubDirectories_Local "IncludeSubDirs_Local"

        /** The full path to a directory on a FTP - server where we should scan for
            data files */
        novac::CString   m_FTPDirectory;
#define   str_FTPDirectory "FTPDirectory"

        /** This is non-zero if we should include sub-directories to 'm_FTPDirectory'
            in our search for data */
        int    m_includeSubDirectories_FTP;
#define   str_includeSubDirectories_FTP "IncludeSubDirs_FTP"

        /** The username and password to log in to the FTP-server */
        novac::CString  m_FTPUsername, m_FTPPassword;
#define  str_FTPUsername "FTPUsername"
#define  str_FTPPassword "FTPPassword"


        // ------------------------------------------------------------------------
        // ---------- SETTINGS FOR WHAT TO DO WITH THE PROCESSED RESULTS ----------
        // ------------------------------------------------------------------------

        /** This is true if we should upload the results (FluxLogs etc) to the
            NovacFTP server.*/
        int   m_uploadResults;
#define  str_uploadResults "UploadResults"

        // ------------------------------------------------------------------------
        // -------------------- SETTINGS FOR THE WIND FIELD -----------------------
        // ------------------------------------------------------------------------

        /** The file where to search for the wind field */
        novac::CString   m_windFieldFile;
#define   str_windFieldFile "WindFieldFile"

        /** How to interpret the m_windFieldFile
            0 <=> m_windFieldFile is an ordinary .wxml file
            1 <=> m_windFieldFile is a directory containing .wxml files
                    with the name "VOLCANO_analysis_YYYYMMDD.wxml"
        */
        int    m_windFieldFileOption;
#define   str_windFieldFileOption "WindFileOption"

        // ------------------------------------------------------------------------
        // ------------- SETTINGS FOR THE GEOMETRY CALCULATIONS  ------------------
        // ------------------------------------------------------------------------

        /** Only scans with calculated completeness higher than this
                given value will be used to calculate the geometries. */
        double   m_calcGeometry_CompletenessLimit;
#define   str_calcGeometry_CompletenessLimit "completenessLimit"

        /** The time a geometry measurement is valid. In seconds.
            Half of this time is before the measurement is made and half is after */
        int    m_calcGeometryValidTime;
#define   str_calcGeometryValidTime "validTime"

        /** The maximum time difference (in seconds) between the start-time
            of two scans that can be combined to make a plume altitude
            calculation */
        int    m_calcGeometry_MaxTimeDifference;
#define   str_calcGeometry_MaxTimeDifference "maxStartTimeDifference"

        /** The minimum distance between two instruments that can be used
            to make a geometry calculation. In meters */
        int    m_calcGeometry_MinDistance;
#define   str_calcGeometry_MinDistance "minInstrumentDistance"

        /** The maximum distance between two instruments that can be used
            to make a geometry calculation. In meters */
        int    m_calcGeometry_MaxDistance;
#define   str_calcGeometry_MaxDistance "maxInstrumentDistance"

        /** The maximum error in the plume altitude calculation that
            we can tolerate */
        double   m_calcGeometry_MaxPlumeAltError;
#define   str_calcGeometry_MaxPlumeAltError "maxPlumeAltitudeError"

        /** The maximum error in the wind direction calculation that
            we can tolerate */
        double   m_calcGeometry_MaxWindDirectionError;
#define   str_calcGeometry_MaxWindDirectionError "maxWindDirectionError"

        // ------------------------------------------------------------------------
        // ------------- SETTINGS FOR THE DUAL BEAM CALCULATIONS  -----------------
        // ------------------------------------------------------------------------

        /** true if we should use the maximum test length possible */
        bool   m_fUseMaxTestLength_DualBeam;
#define   str_fUseMaxTestLength_DualBeam "useMaximumTestLength"

        /** The maximum acceptable error in the wind-speed as determined
            from the dual-beam measurements */
        double   m_dualBeam_MaxWindSpeedError;
#define   str_dualBeam_MaxWindSpeedError "maxWindSpeedError"

        /** The time a geometry measurement is valid. In seconds.
            Half of this time is before the measurement is made and half is after */
        int    m_dualBeam_ValidTime;
#define   str_dualBeam_ValidTime "validTime"

        // ------------------------------------------------------------------------
        // ------------------- SETTINGS FOR THE FIT WINDOWS  -----------------------
        // ------------------------------------------------------------------------

        /** The names of the fit-windows that we should evaluate for */
        novac::CString   m_fitWindowsToUse[MAX_FIT_WINDOWS];
        long   m_nFitWindowsToUse;
#define   m_str_fitWindowToUse "FitWindow_Item"

        /** The name of the most important fit-window
            In processing for fluxes, this is the window that will be used
                to calculate the flux.
        */
        int    m_mainFitWindow;
#define   str_mainFitWindow "main"

        /** The settings for the sky spectrum to use */
        CSkySettings sky;
#define   str_skyOption ""
#define   str_skyIndex ""
#define   str_skySpectrumFromUser ""

// ------------------------------------------------------------------------
// ---------------- SETTINGS FOR THE QUALITY CONTROL  ---------------------
// ------------------------------------------------------------------------

/** Only flux measurements with a calculated completeness higher than this
        given value will be used to calculate a flux. */
        double   m_completenessLimitFlux;
#define   str_completenessLimitFlux "completenessLimit"

        /** All spectra with so little light that the pixel with the highest
            intensity in the fit-region has a saturation level less than this
            limit will be ignored in the evaluation.
            This judgement is done after the dark-current & offset has been removed.

            Range is 0.0 (reject none) to 1.0 (reject all spectra) */
        double   m_minimumSaturationInFitRegion;
#define   str_minimumSaturationInFitRegion "minimumSaturationInFitRegion"

        /** The maximum exposure-time for a spectrum for us to consider it good
            and to evaluate it */
        int    m_maxExposureTime_got;
#define   str_maxExposureTime_got "MaxExpTime_Got"

        int    m_maxExposureTime_hei;
#define   str_maxExposureTime_hei "MaxExpTime_Hei"

    };
}