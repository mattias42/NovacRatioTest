#pragma once

#include "LocationConfiguration.h"
#include "EvaluationConfiguration.h"
#include "DarkCorrectionConfiguration.h"
/**
    The class <b>CInstrumentConfiguration</b> containst the configuration
        of one single instrument for the NovacPPP.
        Each instance of CInstrumentConfiguration holds the serial-number
        of the instrument (which is used to identify each instrument),
        a set of Fit-windows telling us how to evaluate data from the instrument
        and a set of locations telling us where the instrument has been
        located at different times.
*/

namespace Configuration {
    class CInstrumentConfiguration
    {
    public:
        CInstrumentConfiguration(void);
        ~CInstrumentConfiguration(void);

        /** The serial of the spectrometer. This defines the instrument
            and is used to identify instruments. */
        novac::CString m_serial;

        /** The evaluation configuration of this instrument.
            This holds a set of fit-windows telling us how to evaluate the
            data from this instrument. */
        CEvaluationConfiguration m_eval;

        /** The location configuration of this instrument.
            This holds a set of locations, telling us where the
            instrument has been located at different times and when
            it has been moved
        */
        CLocationConfiguration m_location;

        /** The settings for how to correct for the dark current. */
        CDarkCorrectionConfiguration m_darkCurrentCorrection;
    };
}