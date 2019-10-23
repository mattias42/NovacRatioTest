#pragma once

// #include "../resource.h"
// #include "../stdafx.h"
#include "ScanResult.h"

// #include "../Common/Common.h"
#include <SpectralEvaluation/File/ScanFileHandler.h>
#include <SpectralEvaluation/Evaluation/Ratio.h>
#include "../Configuration/NovacPPPConfiguration.h"

namespace Evaluation
{
    /** <b>CPostEvaluationController</b> is used to to perform the
        evaluation of the scans.

        The main function called (from the outside) is
        <b>EvaluateScan</b> which takes care of checking the spectra
        and calling the help-classes <b>CScanEvaluation</b> to perform the
        actual evaluation. This class takes care of writing the results
        to file and performing other useful things...
    */

    class CPostEvaluationController
    {
    public:
        CPostEvaluationController();

        // ----------------------------------------------------------------------
        // ---------------------- PUBLIC DATA -----------------------------------
        // ----------------------------------------------------------------------

        /** A scan-result, for sharing evaluated data with the rest of the
            program. This is updated after every evaluation of a full scan. */
        CScanResult *m_lastResult;

        // ----------------------------------------------------------------------
        // --------------------- PUBLIC METHODS ---------------------------------
        // ----------------------------------------------------------------------

        /** Evaluates the spectra of one scan and writes the results to file.
            @param pakFileName - the name of the .pak-file that should be evaluated
            @param fitWindowName - the name of the fit-window that should be used in the evaluation
                there can be more than one valid fit-window for each spectrometer at each
                given time. The evaluation will only be performed for the fit-window with the
                given name. If this is empty then the first valid fit-window will be used
            @param txtfileName - if not NULL then this novac::CString will on return be filled
                with the full path and filename of the generated txt-file containing the evaluation
                results
            @param plumeProperties - if not NULL then this will on return be filled
                with the properties of the evaluated scan.
            @return 0 on success, else non-zero value
            @return 5 if the scan was rejected due to quality problems
            @return 6 if the scan was rejected because it does not see the plume or
                        sees a too small portion of the plume
          */
        int EvaluateScan(const novac::CString& pakFileName, const novac::CString &fitWindowName, novac::CString *txtFileName = NULL, CPlumeInScanProperty *plumeProperties = NULL);

    private:
        // ----------------------------------------------------------------------
        // ---------------------- PRIVATE DATA ----------------------------------
        // ----------------------------------------------------------------------


        // ----------------------------------------------------------------------
        // --------------------- PRIVATE METHODS --------------------------------
        // ----------------------------------------------------------------------

        /** Looks in the configuration of the instruments and searches
            for a configured location and fit-window (for evaluation) which
            is valid for the spectrometer that collected the given scan and
            is also valid at the time when the scan was made.
            @return 0 if successful otherwise non-zero
        */
        int GetLocationAndFitWindow(FileHandler::CScanFileHandler *scan, const novac::CString &fitWindowName,
            Configuration::CInstrumentLocation &instrLocation,
            Evaluation::CFitWindow &window);

        /** Looks in the configuration of the instrument and searches
            for the settings on how the dark-current should be removed
            from the collected spectra.
            On successful return will the settings be stored in 'settings'
            @return 0 on successful, otherwise non-zero. */
        int GetDarkCurrentSettings(FileHandler::CScanFileHandler *scan, Configuration::CDarkSettings &settings);

        /** Checks the supplied scan if it's good enough to bother evaluating.
            @returns false if the scan is too bad and should be ignored. Else return true. */
        bool IsGoodEnoughToEvaluate(const FileHandler::CScanFileHandler *scan, const Evaluation::CFitWindow &window, Configuration::CInstrumentLocation &instrLocation);

        /** Writes the evaluation result to the appropriate log file.
            @param result - a CScanResult holding information about the result
            @param scan - the scan itself, also containing information about the evaluation and the flux.
            @param scanningInstrument - information about the scanning instrument that generated the scan.
            @param txtFileName - if not null, this will on successful writing of the file be filled
                with the full path and filename of the txt - file generated
            @return SUCCESS if operation completed sucessfully. */
        // RETURN_CODE WriteEvaluationResult(const CScanResult *result, const FileHandler::CScanFileHandler *scan, const Configuration::CInstrumentLocation *instrLocation, const Evaluation::CFitWindow *window, Meteorology::CWindField &windField, novac::CString *txtFileName = nullptr);

        /** Writes the evaluation result of one ratio calculation to the appropriate log file.
            @param result - a vector of calculated ratios.
            @param scan - the scan itself, also containing information about the evaluation and the flux.
            @param scanningInstrument - information about the scanning instrument that generated the scan.
            @param txtFileName - if not null, this will on successful writing of the file be filled
                with the full path and filename of the txt - file generated
            @return SUCCESS if operation completed sucessfully. */
        RETURN_CODE WriteRatioResult(const std::vector<Ratio>& result, const FileHandler::CScanFileHandler& scan, const Evaluation::CFitWindow& window);

        /** Appends the evaluation result to the evaluation summary log file.
            @param result - a CScanResult holding information about the result
            @param scan - the scan itself
            @param scanningInstrument - information about the scanning instrument that generated the scan.
            @return SUCCESS if operation completed sucessfully. */
        // RETURN_CODE AppendToEvaluationSummaryFile(const CScanResult *result, const FileHandler::CScanFileHandler *scan, const Configuration::CInstrumentLocation *instrLocation, const Evaluation::CFitWindow *window, Meteorology::CWindField &windField);

        /** Appends the evaluation result to the pak-file summary log file.
            @param result - a CScanResult holding information about the result
            @param scan - the scan itself
            @param scanningInstrument - information about the scanning instrument that generated the scan.
            @return SUCCESS if operation completed sucessfully. */
            // RETURN_CODE AppendToPakFileSummaryFile(const CScanResult *result, const FileHandler::CScanFileHandler *scan, const Configuration::CInstrumentLocation *instrLocation, const Evaluation::CFitWindow *window, Meteorology::CWindField &windField);

        /** Gets the filename under which the scan-file should be stored.
            @return SUCCESS if a filename is found. */
            // RETURN_CODE GetArchivingfileName(novac::CString &pakFile, novac::CString &txtFile, const novac::CString &fitWindowName, const novac::CString &temporaryScanFile, MEASUREMENT_MODE mode);

        /** This function takes as input parameter an eval-log containing the result of a flux - measurement
            and checks the quality of the measurement.
            @param evalLog - the full path and filename of the flux measurement
            @return 0 - if the measurement should be rejected.
            @return -1 - if the measurement is not a flux measurement.
            */
        int CheckQualityOfFluxMeasurement(CScanResult *result, const novac::CString &pakFileName) const;

    };

}