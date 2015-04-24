//=============================================================================================================
/**
* @file     filterdata.h
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Florian Schlembach <florian.schlembach@tu-ilmenau.de>;
*           Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>;
*           Jens Haueisen <jens.haueisen@tu-ilmenau.de>
* @version  1.0
* @date     February, 2014
*
* @section  LICENSE
*
* Copyright (C) 2014, Lorenz Esch, Florian Schlembach, Christoph Dinh and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    The FilterData class represents a derived class from an arbitrary MNEOperator class object.
*           Thus, it is a filter object that generates the FIR filter coefficients using Park's McClellan's
*           filter design algorithm [1] and offers a overlap-add method [2] for frequency filtering of an input
*           sequence. In this regard, the filter coefficients of a certain filter order are zero-padded to fill
*           a length of an multiple integer of a power of 2 in order to efficiently compute a FFT. The length of
*           the FFT is given by the next power of 2 of the length of the input sequence. In order to avoid
*           circular-convolution, the input sequence is given by the FFT-length-NumFilterTaps.
*
*           e.g. FFT length=4096, NumFilterTaps=80 -> input sequence 4096-80=4016
*
*
*           [1] http://en.wikipedia.org/wiki/Parks%E2%80%93McClellan_filter_design_algorithm
*           [2] http://en.wikipedia.org/wiki/Overlap_add
*/

#ifndef FILTERDATA_H
#define FILTERDATA_H


//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "../utils_global.h"
#include "parksmcclellan.h"
#include "loadfilter.h"
#include "cosinefilter.h"
#include "../mnemath.h"
#include "iostream"


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QFile>
#include <QTextStream>
#include <QDebug>


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================

#include <Eigen/Core>
#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <unsupported/Eigen/FFT>

#ifndef EIGEN_FFTW_DEFAULT
#define EIGEN_FFTW_DEFAULT
#endif


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE UTILSLIB
//=============================================================================================================

namespace UTILSLIB
{


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace Eigen;


//*************************************************************************************************************

class UTILSSHARED_EXPORT FilterData
{

public:
    enum DesignMethod {
        Tschebyscheff,
        Cosine,
        External
    } m_designMethod;

    enum FilterType {
       LPF,
       HPF,
       BPF,
       NOTCH
    } m_Type;

    enum CompensateEdgeEffects {
       ZeroPad,
       MirrorData,
       NoEdgeEffectCompensation
    };

    FilterData();

    /**
    * Constructs a FilterData object
    * @param [in] unique_name defines the name of the generated filter
    * @param [in] type of the filter: LPF, HPF, BPF, NOTCH (from enum FilterType)
    * @param [in] order represents the order of the filter, the higher the higher is the stopband attenuation
    * @param [in] centerfreq determines the center of the frequency
    * @param [in] bandwidth ignored if FilterType is set to LPF,HPF. if NOTCH/BPF: bandwidth of stop-/passband
    * @param [in] parkswidth determines the width of the filter slopes (steepness)
    * @param [in] sFreq sampling frequency
    * @param [in] fftlength length of the fft (multiple integer of 2^x)
    * @param [in] designMethod specifies the design method to use. Choose between Cosind and Tschebyscheff
    */
    FilterData(QString unique_name, FilterType type, int order, double centerfreq, double bandwidth, double parkswidth, double sFreq, qint32 fftlength=4096, DesignMethod designMethod = Cosine);

    /**
    * Constructs a FilterData object
    *
    * @param [in] path path to txt file which contains filter coeffiecients
    * @param [in] fftlength length of the fft (multiple integer of 2^x)
    */
//    FilterData(QString &path, qint32 fftlength);

    /**
     * @brief fftTransformCoeffs transforms the calculated filter coefficients to frequency-domain
     */
    void fftTransformCoeffs();

    /**
     * @brief designFilter designs the actual filter with the given parameters
     */
    void designFilter();

    /**
    * Applies the current filter to the input data using convolution in time domain. Pro: Uses only past samples (real-time capable) Con: Might not be as ideal as acausal version (steepness etc.)
    *
    * @param [in] data holds the data to be filtered
    * @param [in] keepOverhead whether the result should still include the overhead information in front and back of the data
    *
    * @return the filtered data in form of a RoVecotrXd
    */
    RowVectorXd applyConvFilter(const RowVectorXd& data, bool keepOverhead = false, CompensateEdgeEffects compensateEdgeEffects = MirrorData) const;

    /**
    * Applies the current filter to the input data using multiplication in frequency domain. Pro: Fast, good filter parameters Con: Smears in error from future samples. Uses future samples (nor real time capable)
    *
    * @param [in] data holds the data to be filtered
    * @param [in] keepOverhead whether the result should still include the overhead information in front and back of the data
    * @param [in] compensateEdgeEffects defines how the edge effects should be handlted. Choose between ZeroPad and Mirroring
    *
    * @return the filtered data in form of a RoVecotrXd
    */
    RowVectorXd applyFFTFilter(const RowVectorXd& data, bool keepOverhead = false, CompensateEdgeEffects compensateEdgeEffects = MirrorData) const;

    /**
     * @brief getStringForDesignMethod returns the current design method as a string
     */
    inline QString getStringForDesignMethod(const FilterData::DesignMethod &designMethod);

    /**
     * @brief getStringFilterType returns the current filter type as a string
     */
    inline QString getStringForFilterType(const FilterData::FilterType &filterType);

    /**
     * @brief getStringForDesignMethod returns the current design dependent on an input string
     */
    inline FilterData::DesignMethod getDesignMethodForString(const QString &designMethodString);

    /**
     * @brief getFilterTypeForString returns the current filter type dependent on an input string
     */
    inline FilterData::FilterType getFilterTypeForString(const QString &filerTypeString);

    double          m_sFreq;            /**< the sampling frequency. */
    int             m_iFilterOrder;     /**< represents the order of the filter instance. */
    int             m_iFFTlength;       /**< represents the filter length. */
    double          m_dCenterFreq;      /**< contains center freq of the filter. */
    double          m_dBandwidth;       /**< contains bandwidth of the filter. */
    double          m_dParksWidth;      /**< contains the parksmcallen width. */

    double          m_dLowpassFreq;     /**< lowpass freq (higher cut off) of the filter. */
    double          m_dHighFreq;        /**< lowpass freq (lower cut off) of the filter. */

    QString         m_sName;            /**< contains name of the filter. */

    RowVectorXd     m_dCoeffA;          /**< contains the forward filter coefficient set. */
    RowVectorXd     m_dCoeffB;          /**< contains the backward filter coefficient set (empty if FIR filter). */

    RowVectorXcd    m_dFFTCoeffA;       /**< the FFT-transformed forward filter coefficient set, required for frequency-domain filtering, zero-padded to m_iFFTlength. */
    RowVectorXcd    m_dFFTCoeffB;       /**< the FFT-transformed backward filter coefficient set, required for frequency-domain filtering, zero-padded to m_iFFTlength. */
};


//*************************************************************************************************************
//=============================================================================================================
// INLINE DEFINITIONS
//=============================================================================================================

inline QString FilterData::getStringForDesignMethod(const FilterData::DesignMethod &designMethod)
{
    QString designMethodString = "External";

    if(designMethod == FilterData::External)
        designMethodString = "External";

    if(designMethod == FilterData::Cosine)
        designMethodString = "Cosine";

    if(designMethod == FilterData::Tschebyscheff)
        designMethodString = "Tschebyscheff";

    return designMethodString;
}


//*************************************************************************************************************

inline QString FilterData::getStringForFilterType(const FilterData::FilterType &filterType)
{
    QString filterTypeString = "LPF";

    if(filterType == FilterData::LPF)
        filterTypeString = "LPF";

    if(filterType == FilterData::HPF)
        filterTypeString = "HPF";

    if(filterType == FilterData::BPF)
        filterTypeString = "BPF";

    if(filterType == FilterData::NOTCH)
        filterTypeString = "NOTCH";

    return filterTypeString;
}


//*************************************************************************************************************

inline FilterData::DesignMethod FilterData::getDesignMethodForString(const QString &designMethodString)
{
    FilterData::DesignMethod designMethod = FilterData::External;

    if(designMethodString == "External")
        designMethod = FilterData::External;

    if(designMethodString == "Tschebyscheff")
        designMethod = FilterData::Tschebyscheff;

    if(designMethodString == "Cosine")
        designMethod = FilterData::Cosine;

    return designMethod;
}


//*************************************************************************************************************

inline FilterData::FilterType FilterData::getFilterTypeForString(const QString &filterTypeString)
{
    FilterData::FilterType filterType;

    if(filterTypeString == "LPF")
        filterType = FilterData::LPF;

    if(filterTypeString == "HPF")
        filterType = FilterData::HPF;

    if(filterTypeString == "BPF")
        filterType = FilterData::BPF;

    if(filterTypeString == "NOTCH")
        filterType = FilterData::NOTCH;

    return filterType;
}

} // NAMESPACE UTILSLIB

#endif // FILTERDATA_H