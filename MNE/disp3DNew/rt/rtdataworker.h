//=============================================================================================================
/**
* @file     rtdataworker.h
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     December, 2015
*
* @section  LICENSE
*
* Copyright (C) 2015, Lorenz Esch and Matti Hamalainen. All rights reserved.
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
* @brief    RtDataWorker class declaration
*
*/

#ifndef RTDATAWORKER_H
#define RTDATAWORKER_H

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include <iostream>

#include "../disp3dnew_global.h"
#include "../helpers/types.h"

#include <disp/helpers/colormap.h>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QObject>
#include <QList>
#include <QThread>
#include <QSharedPointer>
#include <QMutex>
#include <QTime>


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================

#include <Eigen/Core>


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE DISP3DLIB
//=============================================================================================================

namespace DISP3DNEWLIB
{

//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace Eigen;
using namespace DISPLIB;


//=============================================================================================================
/**
* Worker which schedules data with the right timing
*
* @brief Data scheduler
*/
class DISP3DNEWSHARED_EXPORT RtDataWorker : public QThread
{
    Q_OBJECT
public:
    typedef QSharedPointer<RtDataWorker> SPtr;            /**< Shared pointer type for RtDataWorker class. */
    typedef QSharedPointer<const RtDataWorker> ConstSPtr; /**< Const shared pointer type for RtDataWorker class. */

    //=========================================================================================================
    /**
    * Default constructor.
    *
    * @param[in] parent      The parent of the QObject.
    */
    RtDataWorker(QObject* parent = 0);

    //=========================================================================================================
    /**
    * Default destructor.
    */
    ~RtDataWorker();

    //=========================================================================================================
    /**
    * Add surface data which the streamed data is plotted on.
    *
    * @param[in] arraySurfaceVertColor  The vertex colors for the surface where the data is to be plotted on.
    * @param[in] vecVertNo              The vertex indexes.
    */
    void setSurfaceData(const QByteArray& arraySurfaceVertColor, const VectorXi& vecVertNo);

    //=========================================================================================================
    /**
    * Add data which is to be streamed.
    *
    * @param[in] data         The new data.
    */
    void addData(const MatrixXd& data);

    //=========================================================================================================
    /**
    * Clear this worker.
    */
    void clear();

    //=========================================================================================================
    /**
    * Set the number of average to take after emitting the data to the listening threads.
    *
    * @param[in] samples            The new number of averages.
    */
    void setAverage(qint32 samples);

    //=========================================================================================================
    /**
    * Set the length in milli Seconds to wait inbetween data samples.
    *
    * @param[in] iMSec              The new length in milli Seconds to wait inbetween data samples.
    */
    void setInterval(const int& iMSec);

    //=========================================================================================================
    /**
    * Set the visualization type.
    *
    * @param[in] iVisType           The new visualization type.
    */
    void setVisualizationType(const int& iVisType);

    //=========================================================================================================
    /**
    * Set the type of the colormap.
    *
    * @param[in] sColormapType      The new colormap type.
    */
    void setColormapType(const QString& sColormapType);

    //=========================================================================================================
    /**
    * Set the normalization value.
    *
    * @param[in] dValue             The new normalization value.
    */
    void setNormalization(const double& dValue);

    //=========================================================================================================
    /**
    * Set the loop functionality on or off.
    *
    * @param[in] looping            The new looping state.
    */
    void setLoop(bool looping);

    //=========================================================================================================
    /**
    * QThread functions
    */
    void stop();
    void start();

protected:
    //=========================================================================================================
    /**
    * QThread functions
    */
    virtual void run();

private:
    QByteArray performVisualizationTypeCalculation(const QByteArray& sourceColorSamples);

    //=========================================================================================================
    /**
    * Transform the data sample values to color values.
    *
    * @param[in] data               The data which is to be transformed.
    *
    * @return                       Returns the colors in form of a QByteArray.
    */
    QByteArray transformDataToColor(const VectorXd& data);

    QMutex      m_qMutex;               /**< The thread's mutex. */

    QByteArray  m_arraySurfaceVertColor;/**< The vertex colors for the surface where the data is to be plotted on. */
    MatrixXd    m_matData;              /**< List that holds the fiff matrix data <n_channels x n_samples>. */
    VectorXi    m_vecVertNo;            /**< Vector with the source vertx indexes. */

    bool        m_bIsRunning;           /**< Flag if this thread is running. */
    bool        m_bIsLooping;           /**< Flag if this thread should repeat sending the same data over and over again. */

    qint32      m_iAverageSamples;      /**< Number of average to compute. */
    qint32      m_iCurrentSample;       /**< Number of the current sample which is/was streamed. */
    qint32      m_iMSecIntervall;       /**< Length in milli Seconds to wait inbetween data samples. */
    int         m_iVisualizationType;   /**< The visualization type (single vertex, smoothing, annotation based). */

    double      m_dNormalization;       /**< Normalization value. */
    double      m_dNormalizationMax;    /**< Value to normalize to. */

    QString     m_sColormap;            /**< The type of colormap ("Hot", "Hot Negative 1", etc.). */

signals:
    //=========================================================================================================
    /**
    * Emit this signal whenever this item should send a new sample to its listening threads.
    *
    * @param[in] colorSample     The samples data in form of rgb colors as QByteArray.
    */
    void newRtData(QByteArray colorSample);
};

} // NAMESPACE

#endif // RTDATAWORKER_H
