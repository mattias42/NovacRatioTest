#include "ScanResult.h"
#include <PPPLib/VolcanoInfo.h>

// #include "../Geometry/GeometryCalculator.h"
#include <SpectralEvaluation/Flux/PlumeInScanProperty.h>

#include <algorithm>

using namespace Evaluation;

extern novac::CVolcanoInfo g_volcanoes; // <-- A list of all known volcanoes

CScanResult::CScanResult()
{
    m_specNum = 0;
    //  m_flux.Clear();
    m_geomError = 30.0; // best-case guess, 30%
    m_spectroscopyError = 15.0; // best-case guess, 15%
    m_scatteringError = 30.0; // best-case guess, 30%
                              //  m_measurementMode = MODE_UNKNOWN;
    m_instrumentType = INSTR_GOTHENBURG;
}

CScanResult::CScanResult(const CScanResult& s2) :
    // m_flux(s2.m_flux),
    m_plumeProperties(s2.m_plumeProperties),
    m_geomError(s2.m_geomError),
    m_scatteringError(s2.m_scatteringError),
    m_spectroscopyError(s2.m_spectroscopyError),
    m_specNum(s2.m_specNum),
    // m_measurementMode(s2.m_measurementMode),
    m_instrumentType(s2.m_instrumentType)
{
    this->m_spec = s2.m_spec;
    this->m_specInfo = s2.m_specInfo;
    this->m_corruptedSpectra = s2.m_corruptedSpectra;

    this->m_plumeProperties = s2.m_plumeProperties;

    this->m_skySpecInfo = s2.m_skySpecInfo;
    this->m_darkSpecInfo = s2.m_darkSpecInfo;
}

CScanResult &CScanResult::operator=(const CScanResult &s2)
{
    // The calculated flux and offset
    // this->m_flux = s2.m_flux;

    this->m_plumeProperties = s2.m_plumeProperties;

    // The errors
    m_geomError = s2.m_geomError;
    m_scatteringError = s2.m_scatteringError;
    m_spectroscopyError = s2.m_spectroscopyError;

    this->m_plumeProperties = s2.m_plumeProperties;

    this->m_spec = s2.m_spec;
    this->m_specInfo = s2.m_specInfo;
    this->m_corruptedSpectra = s2.m_corruptedSpectra;
    this->m_specNum = s2.m_specNum;

    this->m_skySpecInfo = s2.m_skySpecInfo;
    this->m_darkSpecInfo = s2.m_darkSpecInfo;

    // this->m_measurementMode = s2.m_measurementMode;
    this->m_instrumentType = s2.m_instrumentType;

    return *this;
}


void CScanResult::InitializeArrays(long specNum)
{
    if (specNum < 0 || specNum > 1024) {
        return;
    }

    m_spec.reserve(specNum);
    m_specInfo.reserve(specNum);
}

/** Appends the result to the list of calculated results */
int CScanResult::AppendResult(const CEvaluationResult &evalRes, const CSpectrumInfo &specInfo) {

    // Append the evaluationresult to the end of the 'm_spec'-vector
    m_spec.push_back(CEvaluationResult(evalRes));

    // Append the spectral information to the end of the 'm_specInfo'-vector
    m_specInfo.push_back(CSpectrumInfo(specInfo));

    // Increase the numbers of spectra in this result-set.
    ++m_specNum;
    return 0;
}

const CEvaluationResult *CScanResult::GetResult(unsigned int specIndex) const {
    if (specIndex >= m_specNum)
        return NULL; // not a valid index

    return &m_spec.at(specIndex);
}

void CScanResult::MarkAsCorrupted(unsigned int specIndex) {
    m_corruptedSpectra.push_back(specIndex);
}

int CScanResult::GetCorruptedNum() const {
    return (int)m_corruptedSpectra.size();
}

int CScanResult::RemoveResult(unsigned int specIndex)
{
    if (specIndex >= m_specNum)
        return 1; // not a valid index

    // Remove the desired value
    auto it = m_specInfo.begin() + specIndex;
    m_specInfo.erase(it);
    // m_specInfo.RemoveAt(specIndex, 1);

    // Decrease the number of values in the list
    m_specNum -= 1;

    return 0;
}

/** Stores the information about the sky-spectrum used */
void CScanResult::SetSkySpecInfo(const CSpectrumInfo &skySpecInfo) {
    this->m_skySpecInfo = skySpecInfo;
}

/** Stores the information about the dark-spectrum used */
void CScanResult::SetDarkSpecInfo(const CSpectrumInfo &darkSpecInfo) {
    this->m_darkSpecInfo = darkSpecInfo;
}

/** Stores the information about the offset-spectrum used */
void CScanResult::SetOffsetSpecInfo(const CSpectrumInfo &offsetSpecInfo) {
    this->m_offsetSpecInfo = offsetSpecInfo;
}

/** Stores the information about the dark-current-spectrum used */
void CScanResult::SetDarkCurrentSpecInfo(const CSpectrumInfo &darkCurSpecInfo) {
    this->m_darkCurSpecInfo = darkCurSpecInfo;
}

/** Check the last spectrum point for goodness of fit */
bool CScanResult::CheckGoodnessOfFit(const CSpectrumInfo& info, float chi2Limit, float upperLimit, float lowerLimit) {
    return CheckGoodnessOfFit(info, m_specNum - 1, chi2Limit, upperLimit, lowerLimit);
}

/** Check spectrum number 'index' for goodness of fit */
bool CScanResult::CheckGoodnessOfFit(const CSpectrumInfo& info, int index, float chi2Limit, float upperLimit, float lowerLimit) {
    if (index < 0 || (unsigned int)index >= m_specNum)
        return false;

    // remember the electronic offset (NB. this is not same as the scan-offset)
    //  m_specInfo[index].m_offset    = (float)offsetLevel;

    return m_spec[index].CheckGoodnessOfFit(info, chi2Limit, upperLimit, lowerLimit);
}

int CScanResult::CalculateOffset(const CMolecule &specie) {
    if (m_specNum < 0) {
        return 1;
    }

    // Get the index for the specie for which we want to calculate the offset
    long specieIndex = GetSpecieIndex(specie.m_name);
    if (specieIndex == -1) { // if the specified specie does not exist
        return 1;
    }

    std::vector<double> columns(m_specNum);
    std::vector<bool> badEval(m_specNum);

    // We then need to rearrange the column data a little bit. 
    for (unsigned int i = 0; i < m_specNum; ++i) {
        columns[i] = m_spec[i].m_referenceResult[specieIndex].m_column;

        // The spectrum is considered as bad if the goodness-of-fit checking
        // has marked it as bad or the user has marked it as deleted
        if (m_spec[i].IsBad() || m_spec[i].IsDeleted()) {
            badEval[i] = true;
        }
        else {
            badEval[i] = false;
        }
    }

    // Calculate the offset
    this->m_plumeProperties.offset = CalculatePlumeOffset(columns, badEval, m_specNum);

    return 0;
}

// int CScanResult::CalculateFlux(const CMolecule &specie, const Meteorology::CWindField &wind, const Geometry::CPlumeHeight &relativePlumeHeight, double compass, double coneAngle, double tilt) {
//     unsigned long i; // iterator
//     CDateTime startTime1, startTime2;
//     Meteorology::CWindField modifiedWind;
// 
//     // If this is a not a flux measurement, then don't calculate any flux
//     if (!IsFluxMeasurement())
//         return 1;
// 
//     // get the specie index
//     int specieIndex = GetSpecieIndex(specie.m_name);
//     if (specieIndex == -1) {
//         return 1;
//     }
// 
//     // pull out the good data points out of the measurement and ignore the bad points
//     // at the same time convert to mg/m2
//     std::vector<double> scanAngle(m_specNum);
//     std::vector<double> scanAngle2(m_specNum);
//     std::vector<double> column(m_specNum);
//     unsigned int nDataPoints = 0;
//     for (i = 0; i < m_specNum; ++i) {
//         if (m_spec[i].IsBad() || m_spec[i].IsDeleted())
//             continue; // this is a bad measurement
//         if (m_specInfo[i].m_flag >= 64)
//             continue; // this is a direct-sun measurement, don't use it to calculate the flux...
// 
//         scanAngle[nDataPoints] = m_specInfo[i].m_scanAngle;
//         scanAngle2[nDataPoints] = m_specInfo[i].m_scanAngle2;
//         column[nDataPoints] = specie.Convert_MolecCm2_to_kgM2(m_spec[i].m_referenceResult[specieIndex].m_column);
//         ++nDataPoints;
//     }
// 
//     // if there are no good datapoints in the measurement, the flux is assumed to be zero
//     if (nDataPoints < 10) {
//         m_flux.Clear();
//         if (nDataPoints == 0)
//             ShowMessage("Could not calculate flux, no good datapoints in measurement");
//         else
//             ShowMessage("Could not calculate flux, too few good datapoints in measurement");
//         return 1;
//     }
// 
//     // Get the times of the scan
//     this->GetSkyStartTime(startTime1);
//     this->GetStartTime(0, startTime2);
//     if (startTime1 < startTime2) {
//         m_flux.m_startTime = startTime1;
//     }
//     else {
//         m_flux.m_startTime = startTime2;
//     }
//     this->GetStopTime(m_specNum - 1, m_flux.m_stopTime);
// 
//     // and the serial number of the instrument
//     m_flux.m_instrument.Format(GetSerial());
// 
//     // Calculate the flux
//     m_flux.m_flux = Common::CalculateFlux(scanAngle.data(), scanAngle2.data(), column.data(), specie.Convert_MolecCm2_to_kgM2(m_plumeProperties.offset), nDataPoints, wind, relativePlumeHeight, compass, m_instrumentType, coneAngle, tilt);
//     m_flux.m_windField = wind;
//     m_flux.m_plumeHeight = relativePlumeHeight;
//     m_flux.m_compass = compass;
//     m_flux.m_coneAngle = coneAngle;
//     m_flux.m_tilt = tilt;
//     m_flux.m_numGoodSpectra = nDataPoints;
// 
//     // calculate the completeness and centre of the plume
//     CalculatePlumeCentre(specie);
// 
//     m_flux.m_scanOffset = m_plumeProperties.offset;
//     m_flux.m_completeness = m_plumeProperties.completeness;
//     m_flux.m_plumeCentre[0] = m_plumeProperties.plumeCenter;
//     m_flux.m_plumeCentre[1] = m_plumeProperties.plumeCenter2;
//     m_flux.m_instrumentType = m_instrumentType;
// 
//     // Try to make an estimation of the error in flux from the
//     //  wind field used and from the plume height used
// 
//     // 1. the wind field
//     modifiedWind = wind;
//     modifiedWind.SetWindDirection(wind.GetWindDirection() - wind.GetWindDirectionError(), wind.GetWindDirectionSource());
//     double flux1 = Common::CalculateFlux(scanAngle.data(), scanAngle2.data(), column.data(), specie.Convert_MolecCm2_to_kgM2(m_plumeProperties.offset), nDataPoints, modifiedWind, relativePlumeHeight, compass, m_instrumentType, coneAngle, tilt);
// 
//     modifiedWind.SetWindDirection(wind.GetWindDirection() + wind.GetWindDirectionError(), wind.GetWindDirectionSource());
//     double flux2 = Common::CalculateFlux(scanAngle.data(), scanAngle2.data(), column.data(), specie.Convert_MolecCm2_to_kgM2(m_plumeProperties.offset), nDataPoints, modifiedWind, relativePlumeHeight, compass, m_instrumentType, coneAngle, tilt);
// 
//     double fluxErrorDueToWindDirection = std::max(fabs(flux2 - m_flux.m_flux), fabs(flux1 - m_flux.m_flux));
// 
//     double fluxErrorDueToWindSpeed = m_flux.m_flux * wind.GetWindSpeedError() / wind.GetWindSpeed();
// 
//     m_flux.m_fluxError_Wind = sqrt(fluxErrorDueToWindDirection * fluxErrorDueToWindDirection + fluxErrorDueToWindSpeed * fluxErrorDueToWindSpeed);
// 
//     // 2. the plume height
//     m_flux.m_fluxError_PlumeHeight = m_flux.m_flux * relativePlumeHeight.m_plumeAltitudeError / relativePlumeHeight.m_plumeAltitude;
// 
//     return 0;
// }

/** Tries to find a plume in the last scan result. If the plume is found
        this function returns true. The result of the calculations is stored in
        the member-variables 'm_plumeCentre', 'm_plumeCompleteness' and m_plumeEdge[0] and m_plumeEdge[1] */
bool CScanResult::CalculatePlumeCentre(const CMolecule &specie) {
    return CalculatePlumeCentre(specie, this->m_plumeProperties);
}

/** Tries to find a plume in the last scan result. If the plume is found
        this function returns true, and the centre of the plume (in scanAngles)
        is given in 'plumeCentre' and the widht of the plume (in scanAngles)
        is given in 'plumeWidth' */

bool CScanResult::CalculatePlumeCentre(const CMolecule &specie, CPlumeInScanProperty &plumeProperties) {
    unsigned long i; // iterator
    double offset = m_plumeProperties.offset;
    m_plumeProperties = CPlumeInScanProperty(); // notify that the plume-centre position is unknown
    m_plumeProperties.offset = offset; // we didn't mean to mess with the offset...

    // if this is a wind-speed measurement, then there's no use to try to 
    //  calculate the plume-centre
    // if (this->IsWindMeasurement())
    //     return false;

    // get the specie index
    int specieIndex = GetSpecieIndex(specie.m_name);
    if (specieIndex == -1) {
        return false;
    }

    // pull out the good data points out of the measurement and ignore the bad points

    std::vector<double> scanAngle(m_specNum);
    std::vector<double> phi(m_specNum);
    std::vector<double> column(m_specNum);
    std::vector<double> columnError(m_specNum);
    std::vector<bool> badEval(m_specNum);
    for (i = 0; i < m_specNum; ++i) {
        if (m_spec[i].IsBad() || m_spec[i].IsDeleted()) {
            badEval[i] = true;
        }
        else {
            badEval[i] = false;
            scanAngle[i] = m_specInfo[i].m_scanAngle;
            phi[i] = m_specInfo[i].m_scanAngle2;
            column[i] = m_spec[i].m_referenceResult[specieIndex].m_column;
            columnError[i] = m_spec[i].m_referenceResult[specieIndex].m_columnError;
        }
    }

    // Calculate the offset of the scan
    plumeProperties.offset = CalculatePlumeOffset(column, badEval, m_specNum);

    // Estimate the completeness of the plume (this will call on FindPlume we don't need to do that here...)
    std::string message;
    bool ret = CalculatePlumeCompleteness(scanAngle, phi, column, columnError, badEval, offset, m_specNum, m_plumeProperties, &message);

    // Calculate the centre of the plume
    // bool ret = FindPlume(scanAngle, phi, column, columnError, badEval, m_specNum, m_plumeProperties);

    if (ret) {
        // Remember the calculated value of the plume centre
        plumeProperties = m_plumeProperties;
    }
    else {
        // If there's no plume, then the flux is probably not very good
        ShowMessage(message);
    }

    return ret;
}

/** Calculates the maximum good column value in the scan, corrected for the offset */
double CScanResult::GetMaxColumn(const novac::CString &specie) const {
    unsigned long i; // iterator
    double maxColumn = 0.0;

    // get the specie index
    int specieIndex = GetSpecieIndex(specie);
    if (specieIndex == -1) {
        return 0.0;
    }

    // Go through the column values and pick out the highest
    for (i = 0; i < m_specNum; ++i) {
        if (m_spec[i].IsBad() || m_spec[i].IsDeleted()) {
            continue;
        }
        maxColumn = std::max(maxColumn, m_spec[i].m_referenceResult[specieIndex].m_column - m_plumeProperties.offset);
    }

    return maxColumn;
}

double CScanResult::GetCalculatedPlumeCentre(int motor) const
{
    if (motor == 1)
        return m_plumeProperties.plumeCenter2;
    else
        return m_plumeProperties.plumeCenter;
}

/** Returns the calculated plume edges */
void CScanResult::GetCalculatedPlumeEdges(double &lowEdge, double &highEdge) const {
    lowEdge = m_plumeProperties.plumeEdgeLow;
    highEdge = m_plumeProperties.plumeEdgeHigh;
}

double CScanResult::GetColumn(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, COLUMN);
}

double CScanResult::GetColumn(unsigned long spectrumNum, CMolecule &molec) const {
    int index = this->GetSpecieIndex(molec.m_name);
    if (index == -1)
        return NOT_A_NUMBER;
    else
        return GetFitParameter(spectrumNum, index, COLUMN);
}

double CScanResult::GetColumnError(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, COLUMN_ERROR);
}

double CScanResult::GetShift(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, SHIFT);
}

double CScanResult::GetShiftError(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, SHIFT_ERROR);
}

double CScanResult::GetSqueeze(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, SQUEEZE);
}

double CScanResult::GetSqueezeError(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, SQUEEZE_ERROR);
}

/** @return the delta of the fit for spectrum number 'spectrumNum'
      @param spectrumNum - the spectrum number (zero-based) for which the delta value is desired         */
double CScanResult::GetDelta(unsigned long spectrumNum) const {
    return this->m_spec[spectrumNum].m_delta;
}

/** @return the chi-square of the fit for spectrum number 'spectrumNum'
      @param spectrumNum - the spectrum number (zero-based) for which the delta value is desired         */
double CScanResult::GetChiSquare(unsigned long spectrumNum) const {
    return this->m_spec[spectrumNum].m_chiSquare;
}

/** Returns the desired fit parameter */
double CScanResult::GetFitParameter(unsigned long specIndex, unsigned long specieIndex, FIT_PARAMETER parameter) const {
    if (specIndex < 0 || specIndex > m_specNum)
        return 0.0f;

    if (specieIndex < 0 || specieIndex > this->m_spec[specIndex].m_referenceResult.size())
        return 0.0f;

    switch (parameter) {
    case COLUMN:        return this->m_spec[specIndex].m_referenceResult[specieIndex].m_column;
    case COLUMN_ERROR:  return this->m_spec[specIndex].m_referenceResult[specieIndex].m_columnError;
    case SHIFT:         return this->m_spec[specIndex].m_referenceResult[specieIndex].m_shift;
    case SHIFT_ERROR:   return this->m_spec[specIndex].m_referenceResult[specieIndex].m_shiftError;
    case SQUEEZE:       return this->m_spec[specIndex].m_referenceResult[specieIndex].m_squeeze;
    case SQUEEZE_ERROR: return this->m_spec[specIndex].m_referenceResult[specieIndex].m_squeezeError;
    case DELTA:         return this->m_spec[specIndex].m_delta;
    default:            return 0.0f;
    }
}

const CSpectrumInfo& CScanResult::GetSpectrumInfo(unsigned long index) const {
    return m_specInfo[index];
}

/** Returns a reference to the spectrum info-structure of the sky-spectrum used */
const CSpectrumInfo &CScanResult::GetSkySpectrumInfo() const {
    return m_skySpecInfo;
}

/** Returns a reference to the spectrum info-structure of the dark-spectrum used */
const CSpectrumInfo &CScanResult::GetDarkSpectrumInfo() const {
    return m_darkSpecInfo;
}

/** Marks the desired spectrum with the supplied mark_flag.
    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
    @return SUCCESS on success. */
bool CScanResult::MarkAs(unsigned long index, int MARK_FLAG) {
    if (!IsValidSpectrumIndex(index))
        return FAIL;

    return m_spec[index].MarkAs(MARK_FLAG);
}

/** Removes the desired mark from the desired spectrum
    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
    @return SUCCESS on success. */
bool CScanResult::RemoveMark(unsigned long index, int MARK_FLAG) {
    if (!IsValidSpectrumIndex(index))
        return FAIL;

    return m_spec[index].RemoveMark(MARK_FLAG);
}

/** Returns the latitude of the system */
double CScanResult::GetLatitude() const {
    for (unsigned int k = 0; k < m_specNum; ++k) {
        const CSpectrumInfo &info = m_specInfo[k];
        if (fabs(info.m_gps.m_latitude) > 1e-2)
            return info.m_gps.m_latitude;
    }
    return 0.0;
}

/** Returns the longitude of the system */
double CScanResult::GetLongitude() const {
    for (unsigned int k = 0; k < m_specNum; ++k) {
        const CSpectrumInfo &info = m_specInfo[k];
        if (fabs(info.m_gps.m_longitude) > 1e-2)
            return info.m_gps.m_longitude;
    }
    return 0.0;
}
/** Returns the altitude of the system */
double CScanResult::GetAltitude() const {
    for (unsigned int k = 0; k < m_specNum; ++k) {
        const CSpectrumInfo &info = m_specInfo[k];
        if (fabs(info.m_gps.m_altitude) > 1e-2)
            return info.m_gps.m_altitude;
    }
    return 0.0;
}

/** Returns the compass-direction of the system */
double CScanResult::GetCompass() const {
    if (m_specNum == 0)
        return 0.0;

    const CSpectrumInfo &info = m_specInfo.front();
    return info.m_compass;
}

/** Returns the battery-voltage of the sky spectrum */
float CScanResult::GetBatteryVoltage() const {
    if (m_specNum == 0)
        return 0.0;

    const CSpectrumInfo &info = m_specInfo.front();
    return info.m_batteryVoltage;
}

/** Returns the cone angle of the scanning instrument */
double CScanResult::GetConeAngle() const {
    if (m_specNum == 0)
        return 90.0;

    const CSpectrumInfo &info = m_specInfo.front();
    return info.m_coneAngle;
}

/** Returns the pitch of the scanning instrument */
double CScanResult::GetPitch() const {
    if (m_specNum == 0)
        return 90.0;

    const CSpectrumInfo &info = m_specInfo.front();
    return info.m_pitch;
}

/** Returns the roll of the scanning instrument */
double CScanResult::GetRoll() const {
    if (m_specNum == 0)
        return 90.0;

    const CSpectrumInfo &info = m_specInfo.front();
    return info.m_roll;
}

/** Returns the name of the requested spectrum */
novac::CString CScanResult::GetName(int index) const {
    if (!IsValidSpectrumIndex(index))
        return novac::CString("");

    const CSpectrumInfo &info = m_specInfo[index];
    return info.m_name;
}

/** Returns the serial-number of the spectrometer that collected this scan */
novac::CString CScanResult::GetSerial() const {
    for (unsigned int k = 0; k < m_specNum; ++k) {
        const CSpectrumInfo &info = m_specInfo[k];
        if (info.m_device.size() > 0)
            return info.m_device;
    }
    return novac::CString("");
}

// MEASUREMENT_MODE CScanResult::CheckMeasurementMode() {
//     if (IsStratosphereMeasurement()) {
//         m_measurementMode = MODE_STRATOSPHERE;
//     }
//     else if (IsWindMeasurement()) {
//         m_measurementMode = MODE_WINDSPEED;
//     }
//     else if (this->IsDirectSunMeasurement()) {
//         m_measurementMode = MODE_DIRECT_SUN;
//     }
//     else if (this->IsCompositionMeasurement()) {
//         m_measurementMode = MODE_COMPOSITION;
//     }
//     else {
//         m_measurementMode = MODE_FLUX;
//     }
// 
//     return m_measurementMode;
// }

// MEASUREMENT_MODE CScanResult::GetMeasurementMode() const {
//     if (IsStratosphereMeasurement()) {
//         return MODE_STRATOSPHERE;
//     }
//     else if (IsWindMeasurement()) {
//         return MODE_WINDSPEED;
//     }
//     else if (this->IsDirectSunMeasurement()) {
//         return MODE_DIRECT_SUN;
//     }
//     else if (this->IsCompositionMeasurement()) {
//         return MODE_COMPOSITION;
//     }
//     else {
//         return MODE_FLUX;
//     }
// }

// bool CScanResult::IsStratosphereMeasurement() const {
//     double SZA, SAZ;
//     CDateTime startTime;
// 
//     // Check so that the measurement is long enough, but not too long
//     if (m_specNum < 3 || m_specNum > 50)
//         return false;
// 
//     // Check if we've already checked the mode
//     if (m_measurementMode == MODE_STRATOSPHERE)
//         return true;
// 
//     // If the measurement started at a time when the Solar Zenith Angle 
//     // was larger than 75 degrees then it is not a wind-speed measurement
//     this->GetStartTime(0, startTime);
//     if (SUCCESS != Common::GetSunPosition(startTime, GetLatitude(), GetLongitude(), SZA, SAZ))
//         return false; // error
// 
//     // It is here assumed that the measurement is a stratospheric measurment
//     // if there are more than 3 repetitions in the zenith positon
//     int nRepetitions = 0; // <-- the number of repetitions in one position
//     for (unsigned int k = 0; k < m_specNum; ++k) {
//         float pos = GetScanAngle(k);
//         if (fabs(pos) < 1e-2)
//             ++nRepetitions;
//         else {
//             nRepetitions = 0;
//         }
// 
//         if (nRepetitions > 50) {
//             if (fabs(SZA) > 75.0)
//                 return true;
//             else
//                 return false;
//         }
//     }
// 
//     if (nRepetitions > 2)
//         return true;
// 
//     return false;
// }
// 
// bool CScanResult::IsFluxMeasurement() {
// 
//     // Check if we've already checked the mode
//     if (m_measurementMode == MODE_FLUX)
//         return true;
// 
//     // Then check the measurement mode
//     this->CheckMeasurementMode();
// 
//     if (m_measurementMode == MODE_FLUX) {
//         return true;
//     }
//     else {
//         return false;
//     }
// }
// 
// bool CScanResult::IsWindMeasurement() const {
//     if (this->IsWindMeasurement_Gothenburg())
//         return true;
//     if (this->IsWindMeasurement_Heidelberg())
//         return true;
// 
//     return false;
// }
// 
// bool CScanResult::IsWindMeasurement_Gothenburg() const {
//     double SZA, SAZ;
//     CDateTime startTime;
// 
//     // Check so that the measurement is long enough
//     if (m_specNum < 52)
//         return false;
// 
//     // Check if we've already checked the mode
//     if (m_measurementMode == MODE_WINDSPEED)
//         return true;
// 
//     // If the measurement started at a time when the Solar Zenith Angle 
//     // was larger than 85 degrees then it is not a wind-speed measurement
//     this->GetStartTime(0, startTime);
//     if (SUCCESS != Common::GetSunPosition(startTime, GetLatitude(), GetLongitude(), SZA, SAZ))
//         return false; // error
//     if (fabs(SZA) >= 85.0)
//         return false;
// 
//     // Check if this is a wind-measurement in the Gothenburg method...
//     int nRepetitions = 0; // <-- the number of repetitions in one position
//     float lastPos = GetScanAngle(3);
//     float lastPos2 = GetScanAngle2(3);
// 
//     // It is here assumed that the measurement is a wind speed measurment
//     // if there are more then 50 repetitions in one measurement positon
//     for (unsigned int k = 4; k < m_specNum; ++k) {
//         float pos = GetScanAngle(k);
//         float pos2 = GetScanAngle2(k);
//         if ((fabs(pos - lastPos) < 1e-2) && (fabs(pos2 - lastPos2) < 1e-2))
//             ++nRepetitions;
//         else {
//             nRepetitions = 0;
//             lastPos = pos;
//             lastPos2 = pos2;
//         }
// 
//         if (nRepetitions > 50) {
//             return true;
//         }
//     }
// 
//     return false;
// }
// 
// bool CScanResult::IsWindMeasurement_Heidelberg() const {
//     double SAZ, SZA;
//     CDateTime startTime;
// 
//     // Check so that the measurement is long enough
//     if (m_specNum < 52)
//         return false;
// 
//     // Check if we've already checked the mode
//     if (m_measurementMode == MODE_WINDSPEED)
//         return true;
// 
//     // Check if the channel-number is equal to 0
//     if (m_specInfo[0].m_channel > 0)
//         return false;
// 
//     // If the measurement started at a time when the Solar Zenith Angle 
//     // was larger than 75 degrees then it is not a wind-speed measurement
//     this->GetStartTime(0, startTime);
//     if (SUCCESS != Common::GetSunPosition(startTime, GetLatitude(), GetLongitude(), SZA, SAZ))
//         return false; // error
//     if (fabs(SZA) >= 75.0)
//         return false;
// 
//     // Check if this is a wind-measurement in the Heidelberg method...
//     int nRepetitions = 0; // <-- the number of repetitions in one position
//     float scanAngle[2] = { GetScanAngle(3), GetScanAngle(4) };
//     float scanAngle2[2] = { GetScanAngle2(3),GetScanAngle2(4) };
//     int  scanIndex = 0;
// 
//     // It is here assumed that the measurement is a wind speed measurement
//     // if there are more then 50 repetitions in one measurement positon
//     for (unsigned int k = 5; k < m_specNum; ++k) {
//         float pos = GetScanAngle(k);
//         float pos2 = GetScanAngle2(k);
// 
//         if ((fabs(pos - scanAngle[scanIndex]) < 1e-2) && (fabs(pos2 - scanAngle2[scanIndex]) < 1e-2)) {
//             ++nRepetitions;
//             scanIndex = (scanIndex + 1) % 2;
//         }
//         else {
//             return false;
//         }
// 
//         if (nRepetitions > 50) {
//             return true;
//         }
//     }
// 
//     return false;
// }
// 
// /** Returns true if this is a direct-sun mode measurement */
// bool CScanResult::IsDirectSunMeasurement() const {
//     // It is here assumed that the measurement is a direct-sun measurment
//     // if there is at least 1 spectrum with the name 'direct_sun'
//     for (unsigned int k = 5; k < m_specNum; ++k) {
//         novac::CString name = GetName(k);
//         if (Equals(name, "direct_sun"))
//             return true;
//     }
// 
//     return false;
// }
// 
// /** Returns true if this is a lunar mode measurement */
// bool CScanResult::IsLunarMeasurement() const {
//     int nFound = 0;
// 
//     // It is here assumed that the measurement is a lunar measurment
//     // if there is at least 1 spectrum with the name 'lunar'
//     for (unsigned int k = 5; k < m_specNum; ++k) {
//         novac::CString name = GetName(k);
//         if (Equals(name, "lunar"))
//             ++nFound;
//         if (nFound == 5)
//             return true;
//     }
// 
//     return false;
// }
// 
// /** Returns true if this is a composition mode measurement */
// bool CScanResult::IsCompositionMeasurement() const {
//     // It is here assumed that the measurement is a composition measurment
//     // if there is 
//     //  * at least 1 spectrum with the name 'comp'
//     //  * no more than 15 spectra in total
//     if (m_specNum > 15)
//         return false;
// 
//     for (unsigned int k = 0; k < m_specNum; ++k) {
//         novac::CString name = GetName(k);
//         if (Equals(name, "comp")) {
//             return true;
//         }
//     }
// 
//     return false;
// }

/** returns the time and date (UMT) when evaluated spectrum number 'index' was started.
    @param index - the zero based index into the list of evaluated spectra */
RETURN_CODE CScanResult::GetStartTime(unsigned long index, CDateTime &t) const {
    if (!IsValidSpectrumIndex(index))
        return FAIL;

    // The start-time
    t = m_specInfo[index].m_startTime;

    return SUCCESS;
}

void CScanResult::GetSkyStartTime(CDateTime &t) const
{
    t = m_skySpecInfo.m_startTime;
    return;
}
CDateTime CScanResult::GetSkyStartTime() const
{
    return m_skySpecInfo.m_startTime;
}

/** returns the time and date (UMT) when evaluated spectrum number 'index' was stopped.
    @param index - the zero based index into the list of evaluated spectra.
        @return SUCCESS if the index is valid */
RETURN_CODE CScanResult::GetStopTime(unsigned long index, CDateTime &t) const {
    if (!IsValidSpectrumIndex(index))
        return FAIL;

    t = m_specInfo[index].m_stopTime;

    return SUCCESS;
}

/** Sets the type of the instrument used */
void CScanResult::SetInstrumentType(INSTRUMENT_TYPE type) {
    this->m_instrumentType = type;
}

/** Sets the type of the instrument used */
INSTRUMENT_TYPE CScanResult::GetInstrumentType() const {
    return this->m_instrumentType;
}

/** Getting the estimated geometrical error */
double CScanResult::GetGeometricalError() const {
    return m_geomError;
}

/** Getting the scattering Error */
double CScanResult::GetScatteringError() const {
    return m_scatteringError;
}

/** Getting the spectroscopical error */
double CScanResult::GetSpectroscopicalError() const {
    return m_spectroscopyError;
}

/** Getting the estimated geometrical error */
void CScanResult::SetGeometricalError(double err) {
    m_geomError = err;
}

/** Getting the scattering Error */
void CScanResult::SetScatteringError(double err) {
    m_scatteringError = err;
}

/** Getting the spectroscopical error */
void CScanResult::SetSpectroscopicalError(double err) {
    m_spectroscopyError = err;
}
