/*
 * @file  TransferFunctionWidget.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef __TRANSFERFUNCTIONWIDGET_H__
#define __TRANSFERFUNCTIONWIDGET_H__

#include <prefr/prefr.h>

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QComboBox>

#include "EditorTF/ColorPoints.h"
#include "EditorTF/Gradient.h"

class TransferFunctionWidget : public QWidget
{
  Q_OBJECT;

public:

  class Preset
  {
  public:
    Preset( const QString& name_, const QGradientStops& stops_ )
      : _name( name_ )
      , _stops( stops_ )
    {
    }
    const QString& name( ) const { return _name; }
    const QGradientStops& stops( ) const { return _stops; }
  protected:
    QString _name;
    QGradientStops _stops;
  };

  TransferFunctionWidget( QWidget* parent = 0 );
  virtual ~TransferFunctionWidget( );

  visimpl::TTransferFunction getColors( bool includeAlpha = true );
  void setColorPoints( const visimpl::TTransferFunction& colors,
                       bool updateResult = true );

  visimpl::TSizeFunction getSizeFunction( void );
  void setSizeFunction( const visimpl::TSizeFunction& sizeFunc );


  visimpl::TTransferFunction getPreviewColors( void );
  visimpl::TSizeFunction getSizePreview( void );




protected slots:

  void gradientClicked( void );

//  void buttonClicked( QAbstractButton* button );
  void acceptClicked( void );
  void cancelClicked( void );
  void previewClicked( void );

  void colorPointsChanged( const QPolygonF &points );

  void presetSelected( int presetIdx );

signals:

  void colorChanged( void );
  void previewColor( void );

  void sizeChanged( void );
  void sizePreview( void );

protected:

  visimpl::TSizeFunction pointsToSizeFunc( const QPolygonF& points );
  visimpl::TSizeFunction pointsToSizeFunc( const QPolygonF& points,
                                  float minValue,
                                  float maxValue );
  QPolygonF sizeFuncToPoints( const visimpl::TSizeFunction& sizeFunc );

  virtual void mousePressEvent( QMouseEvent * event );

  void InitDialog( void );

  // Color transfer function points
  ColorPoints *_redPoints;
  ColorPoints *_bluePoints;
  ColorPoints *_greenPoints;
  ColorPoints *_alphaPoints;

  // Size transfer function points
  ColorPoints* _sizePoints;
  Gradient* _sizeFrame;

  QDoubleSpinBox* _minSizeBox;
  QDoubleSpinBox* _maxSizeBox;

  visimpl::TSizeFunction _sizeFunction;
  float _minSize;
  float _maxSize;
  QLabel* _minValueLabel;
  QLabel* _maxValueLabel;

//    osg::ref_ptr<osg::Texture1D> _texture;

  Gradient* _gradientFrame;
  Gradient* _nTGradientFrame;
  Gradient* redGradientFrame;
  Gradient* greenGradientFrame;
  Gradient* blueGradientFrame;
  Gradient* alphaGradientFrame;

  QWidget* _dialog;
  QComboBox* _presetsComboBox;
  QPushButton* _saveButton;
  QPushButton* _discardButton;
  QPushButton* _previewButton;
  bool previewed;

  Gradient* _result;
  QGradientStops _tResult;

  std::vector< Preset > _presets;
};



#endif /* __TRANSFERFUNCTIONWIDGET_H__ */
