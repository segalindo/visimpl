/*
 * Histogram.h
 *
 *  Created on: 11 de mar. de 2016
 *      Author: sgalindo
 */

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <brion/brion.h>
#include <prefr/prefr.h>

#include <unordered_set>

#include <QFrame>

#include "SimulationData.h"

typedef std::unordered_set< uint32_t > GIDUSet;
typedef utils::InterpolationSet< glm::vec4 > TColorMapper;

namespace visimpl
{
  typedef enum
  {
    T_COLOR_LINEAR = 0,
//    T_COLOR_EXPONENTIAL,
    T_COLOR_LOGARITHMIC,
    T_COLOR_UNDEFINED
  } TColorScale;

  typedef enum
  {
    T_NORM_GLOBAL = 0,
    T_NORM_MAX
  } TNormalize_Rule;

  typedef enum
  {
    T_REP_DENSE = 0,
    T_REP_CURVE

  } TRepresentation_Mode;

  class MultiLevelHistogram : public QFrame
  {

    Q_OBJECT;

  protected:

    class Histogram : public std::vector< unsigned int >
    {
    public:

      unsigned int _maxValueHistogramLocal;
      unsigned int _maxValueHistogramGlobal;
      QGradientStops _gradientStops;
      QPolygonF _curveStopsLocal;
      QPolygonF _curveStopsGlobal;
    };

  public:

    typedef enum
    {
      T_HIST_MAIN = 0,
      T_HIST_FOCUS
    } THistogram;

    MultiLevelHistogram( void );
    MultiLevelHistogram( const brion::Spikes& spikes, float startTime, float endTime );
//    MultiLevelHistogram( const brion::SpikeReport& spikeReport );
    MultiLevelHistogram( const visimpl::SpikeData& spikeReport );

    virtual void init( unsigned int binsNumber = 250, float zoomFactor = 1.5f );

    void Spikes( const brion::Spikes& spikes, float startTime, float endTime );
//    void Spikes( const brion::SpikeReport& spikeReport );
    void Spikes( const visimpl::SpikeData& spikeReport  );

    void BuildHistogram( THistogram histogram = T_HIST_MAIN );

    void CalculateColors( THistogram histogramNumber = T_HIST_MAIN );

    void bins( unsigned int binsNumber );
    unsigned int bins( void );

    void zoomFactor( float factor );
    float zoomFactor( void );

    void filteredGIDs( const GIDUSet& gids );
    const GIDUSet& filteredGIDs( void );

    void colorScaleLocal( TColorScale scale );
    TColorScale colorScaleLocal( void );

    void colorScaleGlobal( TColorScale scale );
    TColorScale colorScaleGlobal( void );

    QColor colorLocal( void );
    void colorLocal( const QColor& );

    QColor colorGlobal( void );
    void colorGlobal( const QColor& );

    void normalizeRule( TNormalize_Rule normRule );
    TNormalize_Rule normalizeRule( void );

    void representationMode( TRepresentation_Mode repType );
    TRepresentation_Mode representationMode( void );

    unsigned int histogramSize( void ) const;
    unsigned int maxLocal( void );
    unsigned int maxGlobal( void );

    unsigned int focusHistogramSize( void ) const;
    unsigned int focusMaxLocal( void );
    unsigned int focusMaxGlobal( void );

    const utils::InterpolationSet< glm::vec4 >& colorMapper( void );
    void colorMapper(const utils::InterpolationSet< glm::vec4 >& colors );

    const QGradientStops& gradientStops( void );

    virtual void mousePressEvent( QMouseEvent* event_ );
    virtual void mouseMoveEvent( QMouseEvent* event_ );
    void mousePosition( QPoint* mousePosition_ );

    void regionWidth( float region_ );
    float regionWidth( void );
    void paintRegion( bool region = false );

    unsigned int valueAt( float percentage );
    unsigned int focusValueAt( float percentage );

    bool isInitialized( void );

    QPolygonF localFunction( void ) const;
    QPolygonF globalFunction( void ) const;

    QPolygonF focusLocalFunction( void ) const;
    QPolygonF focusGlobalFunction( void ) const;

signals:

    void mousePositionChanged( QPoint point );
    void mouseClicked( float position );
    void modifierClicked( void );

  protected:

    virtual void paintEvent(QPaintEvent* event);

    Histogram _mainHistogram;
    Histogram _focusHistogram;

    unsigned int _bins;
    float _zoomFactor;

    const brion::Spikes* _spikes;
    float _startTime;
    float _endTime;

    float (*_scaleFuncLocal)( float value, float maxValue);
    float (*_scaleFuncGlobal)( float value, float maxValue);

    TColorScale _colorScaleLocal;
    TColorScale _colorScaleGlobal;

    QColor _colorLocal;
    QColor _colorGlobal;

    TColorScale _prevColorScaleLocal;
    TColorScale _prevColorScaleGlobal;

    TNormalize_Rule _normRule;
    TRepresentation_Mode _repMode;

    utils::InterpolationSet< glm::vec4 > _colorMapper;

    GIDUSet _filteredGIDs;

    QPoint* _lastMousePosition;

    bool _paintRegion;
    float _regionWidth;
  };
}



#endif /* __HISTOGRAM_H__ */