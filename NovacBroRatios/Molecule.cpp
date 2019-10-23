#include "Molecule.h"

CMolecule::CMolecule(void)
{
    m_name = "SO2";
    m_molecularWeight = 64.0638; // the molecular weight for SO2
}

/** Constructor for a standard gas
    Creates an object which has the properties
    of the standard gas supplied */
CMolecule::CMolecule(STANDARD_MOLECULE molec) {
    switch (molec) {
    case MOLEC_SO2: m_name = "SO2";	m_molecularWeight = 64.0638; break;
    case MOLEC_O3:	m_name = "O3";	m_molecularWeight = 47.9982; break;
    case MOLEC_BRO: m_name = "BrO";	m_molecularWeight = 95.8980; break;
    case MOLEC_NO2: m_name = "NO2";	m_molecularWeight = 46.0055; break;
    case MOLEC_HCHO: m_name = "HCHO"; m_molecularWeight = 30.0206; break;
    }
}

CMolecule::~CMolecule(void)
{
}

/** Takes a column in molec/cm2 and converts it to mg/m2 */
double CMolecule::Convert_MolecCm2_to_kgM2(double molec_cm2) const {
    return molec_cm2 * (10.0) * this->m_molecularWeight / AVOGADROS_NUMBER;
}
