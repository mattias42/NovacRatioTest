#include "ScanEvaluation.h"

#include <SpectralEvaluation/Evaluation/EvaluationBase.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>
#include <SpectralEvaluation/File/SpectrumIO.h>
#include <SpectralEvaluation/File/STDFile.h>
#include <SpectralEvaluation/File/TXTFile.h>

// we want to make some statistics on the processing
// #include "../PostProcessingStatistics.h"

// This is the settings for how to do the procesing
#include "../Configuration/UserConfiguration.h"

#include <cstdint>

// extern CPostProcessingStatistics					g_processingStats; // <-- The statistics of the processing itself
extern Configuration::CUserConfiguration			g_userSettings;// <-- The settings of the user

using namespace Evaluation;

CScanEvaluation::CScanEvaluation()
    : ScanEvaluationBase()
{
}

CScanEvaluation::~CScanEvaluation(void)
{
    if (m_result != NULL) {
        delete(m_result);
        m_result = NULL;
    }
}

long CScanEvaluation::EvaluateScan(FileHandler::CScanFileHandler *scan, const CFitWindow &fitWindow, const Configuration::CDarkSettings *darkSettings) {
    CEvaluationBase *eval = NULL; // the evaluator
    CFitWindow adjustedFitWindow = fitWindow; // we may need to make some small adjustments to the fit-window. This is a modified copy

    // Adjust the fit-low and fit-high parameters according to the spectra
    m_fitLow = adjustedFitWindow.fitLow;
    m_fitHigh = adjustedFitWindow.fitHigh;

    // sometimes the length of the spectra is not what we expect, 
    //	we need to be able to handle this.
    adjustedFitWindow.interlaceStep = scan->GetInterlaceSteps();
    adjustedFitWindow.specLength = scan->GetSpectrumLength() * adjustedFitWindow.interlaceStep;
    adjustedFitWindow.startChannel = scan->GetStartChannel();

    // Now choose what we should do before the real evaluation. Should we;
    //	1) find the shift & squeeze from the Fraunhofer spectrum
    //	2) find the optimal shift & squeeze from the spectrum with the highest column
    //  3) do none of the above

    if (adjustedFitWindow.fraunhoferRef.m_path.size() > 4)
    {
        ShowMessage("  Determining shift from FraunhoferReference");
        this->m_lastErrorMessage.clear();

        adjustedFitWindow.fraunhoferRef.ReadCrossSectionDataFromFile();

        // If we have a solar-spectrum that we can use to determine the shift
        //	& squeeze then fit that first so that we know the wavelength calibration
        eval = FindOptimumShiftAndSqueezeFromFraunhoferReference(adjustedFitWindow, *darkSettings, g_userSettings.sky, *scan);
        if (nullptr == eval)
        {
            ShowMessage(m_lastErrorMessage);
            return 0;
        }

        if (m_lastErrorMessage.size() > 1)
        {
            ShowMessage(m_lastErrorMessage);
        }
    }
    else if (fitWindow.findOptimalShift)
    {
        //	Find the optimal shift & squeeze from the spectrum with the highest column
        CFitWindow window2 = adjustedFitWindow;
        for (int k = 0; k < window2.nRef; ++k) {
            window2.ref[k].m_shiftOption = SHIFT_FIX;
            window2.ref[k].m_squeezeOption = SHIFT_FIX;
            window2.ref[k].m_shiftValue = 0.0;
            window2.ref[k].m_squeezeValue = 1.0;
        }
        eval = new CEvaluationBase(window2);

        // evaluate the scan one time
        if (-1 == EvaluateOpenedScan(scan, eval, darkSettings))
        {
            delete eval;
            return 0;
        }

        if (m_indexOfMostAbsorbingSpectrum < 0)
        {
            ShowMessage("Could not determine optimal shift & squeeze. No good spectra in scan. " + scan->GetFileName());
            return 0;
        }

        // Make sure that this spectrum was ok and that the column-value is high enough
        int specieNum = 0; // TODO: Is this the correct specie to check for?
        double columnError = m_result->GetColumnError(m_indexOfMostAbsorbingSpectrum, specieNum); // <-- the column error that corresponds to the highest column-value
        double highestColumn = m_result->GetColumn(m_indexOfMostAbsorbingSpectrum, specieNum);
        if (highestColumn < 2 * columnError)
        {
            ShowMessage("Could not determine optimal shift & squeeze. Maximum column is too low.");
            return 0;
        }

        CEvaluationBase* newEval = FindOptimumShiftAndSqueeze(adjustedFitWindow, m_indexOfMostAbsorbingSpectrum, *scan);
        if(nullptr == newEval)
        {
            return 0;
        }

        delete eval;
        eval = newEval;
    }
    else
    {
        //  3) do none of the above
        eval = new CEvaluationBase(adjustedFitWindow);
    }

    // Make the real evaluation of the scan
    int nSpectra = EvaluateOpenedScan(scan, eval, darkSettings);

    // Clean up
    delete eval;

    if (nSpectra == -1)
    {
        return 0;
    }

    // return the number of spectra evaluated
    return m_result->GetEvaluatedNum();
}

long CScanEvaluation::EvaluateOpenedScan(FileHandler::CScanFileHandler *scan, CEvaluationBase *eval, const Configuration::CDarkSettings *darkSettings) {
    novac::CString message;	// used for ShowMessage messages
    int	curSpectrumIndex = 0;		// keeping track of the index of the current spectrum into the .pak-file
    double highestColumnInScan = 0.0;	// the highest column-value in the evaluation

    CSpectrum dark, current;

    // ----------- Get the sky spectrum --------------
    // Get the sky and dark spectra and divide them by the number of 
    //     co-added spectra in it
    CSpectrum sky;
    if (!GetSky(*scan, g_userSettings.sky, sky)) {
        return -1;
    }
    CSpectrum skySpecBeforeDarkCorrection = sky;

    if (g_userSettings.sky.skyOption != Configuration::SKY_OPTION::USER_SUPPLIED)
    {
        // Get the dark-spectrum and remove it from the sky
        if (!GetDark(scan, sky, dark, darkSettings))
        {
            return -1;
        }
        sky.Sub(dark);
    }

    if (sky.NumSpectra() > 0 && !m_averagedSpectra) {
        sky.Div(sky.NumSpectra());
        skySpecBeforeDarkCorrection.Div(skySpecBeforeDarkCorrection.NumSpectra());
    }

    // tell the evaluator which sky-spectrum to use
    eval->SetSkySpectrum(sky);

    // Adjust the fit-low and fit-high parameters according to the spectra
    m_fitLow -= sky.m_info.m_startChannel;
    m_fitHigh -= sky.m_info.m_startChannel;

    curSpectrumIndex = -1; // we're at spectrum number 0 in the .pak-file
    m_indexOfMostAbsorbingSpectrum = -1;	// as far as we know, there's no absorption in any spectrum...

    // the data structure to keep track of the evaluation results
    if (m_result != NULL)
        delete m_result;
    m_result = new CScanResult();
    m_result->SetSkySpecInfo(skySpecBeforeDarkCorrection.m_info);
    m_result->SetDarkSpecInfo(dark.m_info);

    // Make sure that we'll start with the first spectrum in the scan
    scan->ResetCounter();

    // Evaluate all the spectra in the scan.
    while (1) {
        // remember which spectrum we're at
        int	spectrumIndex = current.ScanIndex();

        // a. Read the next spectrum from the file
        int ret = scan->GetNextSpectrum(current);

        if (ret == 0) {
            // if something went wrong when reading the spectrum
            if (scan->m_lastError == SpectrumIO::CSpectrumIO::ERROR_SPECTRUM_NOT_FOUND || scan->m_lastError == SpectrumIO::CSpectrumIO::ERROR_EOF) {
                // at the end of the file, quit the 'while' loop
                break;
            }
            else {
                novac::CString errMsg;
                errMsg.Format("Faulty spectrum found in %s", scan->GetFileName().c_str());
                switch (scan->m_lastError) {
                case SpectrumIO::CSpectrumIO::ERROR_CHECKSUM_MISMATCH:
                    errMsg.AppendFormat(", Checksum mismatch. Spectrum ignored"); break;
                case SpectrumIO::CSpectrumIO::ERROR_DECOMPRESS:
                    errMsg.AppendFormat(", Decompression error. Spectrum ignored"); break;
                default:
                    ShowMessage(", Unknown error. Spectrum ignored");
                }
                ShowMessage(errMsg);
                // remember that this spectrum is corrupted
                m_result->MarkAsCorrupted(spectrumIndex);
                continue;
            }
        }

        ++curSpectrumIndex;	// we'have just read the next spectrum in the .pak-file

        // If the read spectrum is the sky or the dark spectrum, 
        //	then don't evaluate it...
        if (current.ScanIndex() == sky.ScanIndex() || current.ScanIndex() == dark.ScanIndex()) {
            continue;
        }

        // If the spectrum is read out in an interlaced way then interpolate it back to it's original state
        if (current.m_info.m_interlaceStep > 1)
            current.InterpolateSpectrum();

        // b. Get the dark spectrum for this measured spectrum
        if (!GetDark(scan, current, dark, darkSettings))
        {
            delete scan;
            delete eval;
            return 0;
        }

        // b. Calculate the intensities, before we divide by the number of spectra
        //		and before we subtract the dark
        current.m_info.m_peakIntensity = (float)current.MaxValue(0, current.m_length - 2);
        current.m_info.m_fitIntensity = (float)current.MaxValue(m_fitLow, m_fitHigh);

        // c. Divide the measured spectrum with the number of co-added spectra
        //     The sky and dark spectra should already be divided before this loop.
        if (current.NumSpectra() > 0 && !m_averagedSpectra)
            current.Div(current.NumSpectra());

        // d. Get the dark spectrum
        if (dark.NumSpectra() > 0 && !m_averagedSpectra)
            dark.Div(dark.NumSpectra());

        // e. Check if this spectrum is worth evaluating
        if (Ignore(current, dark, m_fitLow, m_fitHigh)) {
            message.Format("  - Ignoring spectrum %d in scan %s.", current.ScanIndex(), scan->GetFileName().c_str());
            ShowMessage(message);
            continue;
        }

        // f. The spectrum is ok, remove the dark.
        current.Sub(dark);

        // e. Evaluate the spectrum
        if (eval->Evaluate(current)) {
            message.Format("Failed to evaluate spectrum %d out of %d in scan %s from spectrometer %s.",
                current.ScanIndex(), current.SpectraPerScan(), scan->GetFileName().c_str(), current.m_info.m_device.c_str());
            ShowMessage(message);
        }

        // e. Save the evaluation result
        m_result->AppendResult(eval->GetEvaluationResult(), current.m_info);

        // f. Check if this was an ok data point (CScanResult)
        m_result->CheckGoodnessOfFit(current.m_info);

        // g. If it is ok, then check if the value is higher than any of the previous ones
        if (m_result->IsOk(m_result->GetEvaluatedNum() - 1) && fabs(m_result->GetColumn(m_result->GetEvaluatedNum() - 1, 0)) > highestColumnInScan) {
            highestColumnInScan = fabs(m_result->GetColumn(m_result->GetEvaluatedNum() - 1, 0));
            m_indexOfMostAbsorbingSpectrum = curSpectrumIndex;
        }
    } // end while(1)

    return m_result->GetEvaluatedNum();
}

bool CScanEvaluation::GetDark(FileHandler::CScanFileHandler *scan, const CSpectrum &spec, CSpectrum &dark, const Configuration::CDarkSettings *darkSettings)
{
    m_lastErrorMessage = "";
    const bool successs = ScanEvaluationBase::GetDark(*scan, spec, dark, darkSettings);

    if (m_lastErrorMessage.size() > 0)
    {
        novac::CString message;
        message.Format("%s", m_lastErrorMessage.c_str());
        ShowMessage(message);
    }

    return successs;
}

bool CScanEvaluation::GetSky(FileHandler::CScanFileHandler& scan, const Configuration::CSkySettings& settings, CSpectrum &sky)
{
    m_lastErrorMessage = "";
    const bool successs = ScanEvaluationBase::GetSky(scan, settings, sky);

    if (m_lastErrorMessage.size() > 0)
    {
        novac::CString message;
        message.Format("%s", m_lastErrorMessage.c_str());
        ShowMessage(message);
    }

    return successs;
}

/** Returns true if the spectrum should be ignored */
bool CScanEvaluation::Ignore(const CSpectrum &spec, const CSpectrum &dark, int fitLow, int fitHigh) {

    // check if the intensity is below the given limit
    const double maxIntensity = spec.MaxValue(fitLow, fitHigh) - dark.MinValue(fitLow, fitHigh);

    const double dynamicRange = CSpectrometerDatabase::GetInstance().GetModel(spec.m_info.m_specModelName).maximumIntensity;

    if (maxIntensity < (dynamicRange * g_userSettings.m_minimumSaturationInFitRegion))
    {
        return true;
    }

    return false;
}


CEvaluationBase* CScanEvaluation::FindOptimumShiftAndSqueeze(const CFitWindow &fitWindow, int indexOfMostAbsorbingSpectrum, FileHandler::CScanFileHandler& scan)
{
    CSpectrum spec, sky, dark;

    // Tell the user
    novac::CString message;
    message.Format("ReEvaluating spectrum number %d to determine optimum shift and squeeze", indexOfMostAbsorbingSpectrum);
    ShowMessage(message);

    // Evaluate this spectrum again with free (and linked) shift
    CFitWindow fitWindow2 = fitWindow;
    fitWindow2.ref[0].m_shiftOption = SHIFT_FREE;
    fitWindow2.ref[0].m_squeezeOption = SHIFT_FIX;
    fitWindow2.ref[0].m_squeezeValue = 1.0;
    for (int k = 1; k < fitWindow2.nRef; ++k)
    {
        if (novac::Equals(fitWindow2.ref[k].m_specieName, "FraunhoferRef"))
        {
            continue;
        }

        fitWindow2.ref[k].m_shiftOption = SHIFT_LINK;
        fitWindow2.ref[k].m_squeezeOption = SHIFT_LINK;
        fitWindow2.ref[k].m_shiftValue = 0.0;
        fitWindow2.ref[k].m_squeezeValue = 0.0;
    }

    // Get the sky-spectrum
    if (!GetSky(scan, g_userSettings.sky, sky))
    {
        return nullptr;
    }
    if (sky.NumSpectra() > 0 && !m_averagedSpectra)
    {
        sky.Div(sky.NumSpectra());
    }

    // Get the dark-spectrum
    if (!GetDark(&scan, sky, dark))
    {
        return nullptr;
    }
    if (dark.NumSpectra() > 0 && !m_averagedSpectra)
    {
        dark.Div(dark.NumSpectra());
    }

    // Subtract the dark...
    sky.Sub(dark);

    // create the new evaluator
    CEvaluationBase* intermediateEvaluator = new CEvaluationBase(fitWindow2);
    intermediateEvaluator->SetSkySpectrum(sky);

    // Get the measured spectrum
    scan.GetSpectrum(spec, 2 + indexOfMostAbsorbingSpectrum); // The two comes from the sky and the dark spectra in the beginning
    if (spec.m_info.m_interlaceStep > 1)
    {
        spec.InterpolateSpectrum();
    }
    if (spec.NumSpectra() > 0 && !m_averagedSpectra)
    {
        spec.Div(spec.NumSpectra());
    }

    // Get the dark-spectrum and remove it
    GetDark(&scan, spec, dark);
    spec.Sub(dark);

    // Evaluate
    intermediateEvaluator->Evaluate(spec, 5000);

    // 4. See what the optimum value for the shift turned out to be.
    CEvaluationResult newResult = intermediateEvaluator->GetEvaluationResult();
    double optimumShift = newResult.m_referenceResult[0].m_shift;
    double optimumSqueeze = newResult.m_referenceResult[0].m_squeeze;

    // 5. Set the shift for all references to this value
    for (int k = 0; k < fitWindow2.nRef; ++k)
    {
        if (novac::Equals(fitWindow2.ref[k].m_specieName, "FraunhoferRef"))
        {
            continue;
        }

        fitWindow2.ref[k].m_shiftOption = SHIFT_FIX;
        fitWindow2.ref[k].m_squeezeOption = SHIFT_FIX;
        fitWindow2.ref[k].m_shiftValue = optimumShift;
        fitWindow2.ref[k].m_squeezeValue = optimumSqueeze;
    }
    delete intermediateEvaluator;

    CEvaluationBase* newEvaluator = new CEvaluationBase(fitWindow2);
    newEvaluator->SetSkySpectrum(sky);

    // 6. We're done!
    message.Format("Optimum shift set to : %.2lf. Optimum squeeze set to: %.2lf ", optimumShift, optimumSqueeze);
    ShowMessage(message);

    return newEvaluator;
}
