#include "ui_mainwindow.h"
#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include "qt/CustomSlider.h"

MainWindow::MainWindow( QWidget* parent_,
                        bool updateOnIdle )
  : QMainWindow( parent_ )
  , _lastOpenedFileName( "" )
  , _ui( new Ui::MainWindow )
  , _openGLWidget( nullptr )
{
  _ui->setupUi( this );

  _ui->actionUpdateOnIdle->setChecked( updateOnIdle );
  _ui->actionPaintNeurons->setChecked( true );
  _ui->actionShowFPSOnIdleUpdate->setChecked( false );

#ifdef NSOL_USE_BBPSDK
  _ui->actionOpenBlueConfig->setEnabled( true );
#else
  _ui->actionOpenBlueConfig->setEnabled( false );
#endif

#ifdef NSOL_USE_QT5CORE
  _ui->actionOpenXMLScene->setEnabled( true );
#else
  _ui->actionOpenXMLScene->setEnabled( false );
#endif


  connect( _ui->actionQuit, SIGNAL( triggered( )),
           QApplication::instance(), SLOT( quit( )));

}

void MainWindow::init( const std::string& zeqUri )
{

  _openGLWidget = new OpenGLWidget( 0, 0,
                                    _ui->actionPaintNeurons->isChecked( ),
                                    zeqUri );
  this->setCentralWidget( _openGLWidget );
  qDebug( ) << _openGLWidget->format( );

  _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ));

  connect( _ui->actionUpdateOnIdle, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleUpdateOnIdle( )));

  connect( _ui->actionPaintNeurons, SIGNAL( triggered( )),
           _openGLWidget, SLOT( togglePaintNeurons( )));

  connect( _ui->actionBackgroundColor, SIGNAL( triggered( )),
           _openGLWidget, SLOT( changeClearColor( )));

  connect( _ui->actionShowFPSOnIdleUpdate, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleShowFPS( )));

  connect( _ui->actionWireframe, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleWireframe( )));

  connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( )),
           this, SLOT( openBlueConfigThroughDialog( )));

  connect( _ui->actionOpenXMLScene, SIGNAL( triggered( )),
           this, SLOT( openXMLSceneThroughDialog( )));

  connect( _ui->actionOpenSWCFile, SIGNAL( triggered( )),
           this, SLOT( openSWCFileThroughDialog( )));


  initSimulationDock( );
  initSimColorDock( );
}

MainWindow::~MainWindow( void )
{
    delete _ui;
}


void MainWindow::showStatusBarMessage ( const QString& message )
{
  _ui->statusbar->showMessage( message );
}

void MainWindow::openBlueConfig( const std::string& fileName,
                                 OpenGLWidget::TSimulationType simulationType,
                                 const std::string& reportLabel)
{
  _openGLWidget->loadData( fileName,
                           OpenGLWidget::TDataFileType::tBlueConfig,
                           simulationType, reportLabel );


  connect( _openGLWidget, SIGNAL( updateSlider( float )),
           this, SLOT( UpdateSimulationSlider( float )));


  _startTimeLabel->setText(
      QString::number( (double)_openGLWidget->player( )->startTime( )));

  _endTimeLabel->setText(
        QString::number( (double)_openGLWidget->player( )->endTime( )));


  changeEditorColorMapping( );
}

void MainWindow::openBlueConfigThroughDialog( void )
{
#ifdef NSOL_USE_BBPSDK

  QString path = QFileDialog::getOpenFileName(
    this, tr( "Open BlueConfig" ), _lastOpenedFileName,
    tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ),
    nullptr, QFileDialog::DontUseNativeDialog );

  if (path != QString( "" ))
  {
    bool ok1, ok2;
    QInputDialog simTypeDialog;
    OpenGLWidget::TSimulationType simType;
    QStringList items = {"Spikes", "Voltages"};

    QString text = QInputDialog::getItem(
      this, tr( "Please select simulation type" ),
      tr( "Type:" ), items, 0, false, &ok1 );

    if( !ok1 )
      return;

    if( text == items[0] )
    {
      simType = OpenGLWidget::TSpikes;
      ok2 = true;
    }
    else
    {
      simType = OpenGLWidget::TVoltages;

      text = QInputDialog::getText(
          this, tr( "Please select report" ),
          tr( "Report:" ), QLineEdit::Normal,
          "soma", &ok2 );
    }

    if ( ok1 && ok2 && !text.isEmpty( ))
    {
//      std::string targetLabel = text.toStdString( );
      std::string reportLabel = text.toStdString( );
      _lastOpenedFileName = QFileInfo(path).path( );
      std::string fileName = path.toStdString( );
      openBlueConfig( fileName, simType, reportLabel );
    }


  }
#endif

}


void MainWindow::openXMLScene( const std::string& fileName )
{
  _openGLWidget->loadData( fileName,
    OpenGLWidget::TDataFileType::NsolScene );
}


void MainWindow::openXMLSceneThroughDialog( void )
{
#ifdef NSOL_USE_QT5CORE
  QString path = QFileDialog::getOpenFileName(
    this, tr( "Open XML Scene" ), _lastOpenedFileName,
    tr( "XML ( *.xml);; All files (*)" ), nullptr,
    QFileDialog::DontUseNativeDialog );

  if ( path != QString( "" ))
  {
    std::string fileName = path.toStdString( );
    openXMLScene( fileName );
  }
#endif

}


void MainWindow::openSWCFile( const std::string& fileName )
{
  _openGLWidget->loadData( fileName,
    OpenGLWidget::TDataFileType::SWC );
}


void MainWindow::openSWCFileThroughDialog( void )
{
  QString path = QFileDialog::getOpenFileName(
    this, tr( "Open Swc File" ), _lastOpenedFileName,
    tr( "swc ( *.swc);; All files (*)" ), nullptr,
    QFileDialog::DontUseNativeDialog );

  if ( path != QString( "" ))
  {
    std::string fileName = path.toStdString( );
    openSWCFile( fileName );
  }

}

void MainWindow::initSimulationDock( void )
{
  _simulationDock = new QDockWidget( );
  _simulationDock->setMinimumHeight( 100 );
  _simulationDock->setSizePolicy( QSizePolicy::MinimumExpanding,
                                  QSizePolicy::MinimumExpanding );

  unsigned int totalHSpan = 20;

  QWidget* content = new QWidget( );
  QGridLayout* dockLayout = new QGridLayout( );
  content->setLayout( dockLayout );

  _simSlider = new CustomSlider( Qt::Horizontal );
  _simSlider->setMinimum( 0 );
  _simSlider->setMaximum( 1000 );
  _simSlider->setSizePolicy( QSizePolicy::Preferred,
                             QSizePolicy::Preferred );

//  QPushButton* playButton = new QPushButton( );
  _playButton = new QPushButton( );
  _playButton->setSizePolicy( QSizePolicy::MinimumExpanding,
                             QSizePolicy::MinimumExpanding );
  QPushButton* stopButton = new QPushButton( );
  QPushButton* nextButton = new QPushButton( );
  QPushButton* prevButton = new QPushButton( );
  QPushButton* repeatButton = new QPushButton( );
  repeatButton->setCheckable( true );
  repeatButton->setChecked( false );

//  QIcon playIcon;
//  QIcon pauseIcon;
  QIcon stopIcon;
  QIcon nextIcon;
  QIcon prevIcon;
  QIcon repeatIcon;

  playIcon.addFile( QStringLiteral( ":/icons/play.png" ), QSize( ),
                    QIcon::Normal, QIcon::Off );
  pauseIcon.addFile( QStringLiteral( ":/icons/pause.png" ), QSize( ),
                     QIcon::Normal, QIcon::Off) ;
  stopIcon.addFile( QStringLiteral( ":/icons/stop.png" ), QSize( ),
                    QIcon::Normal, QIcon::Off );
  nextIcon.addFile( QStringLiteral( ":/icons/next.png" ), QSize( ),
                    QIcon::Normal, QIcon::Off );
  prevIcon.addFile( QStringLiteral( ":/icons/previous.png" ), QSize( ),
                    QIcon::Normal, QIcon::Off );
  repeatIcon.addFile( QStringLiteral( ":/icons/repeat.png" ), QSize( ),
                      QIcon::Normal, QIcon::Off );

  _playButton->setIcon( playIcon );
  stopButton->setIcon( stopIcon );
  nextButton->setIcon( nextIcon );
  prevButton->setIcon( prevIcon );
  repeatButton->setIcon( repeatIcon );

  _startTimeLabel = new QLabel( "" );
  _startTimeLabel->setSizePolicy( QSizePolicy::MinimumExpanding,
                                  QSizePolicy::Preferred );
  _endTimeLabel = new QLabel( "" );
  _endTimeLabel->setSizePolicy( QSizePolicy::Preferred,
                                  QSizePolicy::Preferred );

  dockLayout->addWidget( _startTimeLabel, 0, 0, 1, 2 );
  dockLayout->addWidget( _simSlider, 0, 1, 1, totalHSpan - 3 );
  dockLayout->addWidget( _endTimeLabel, 0, totalHSpan - 2, 1, 1, Qt::AlignRight );
  dockLayout->addWidget( repeatButton, 1, 7, 1, 1 );
  dockLayout->addWidget( prevButton, 1, 8, 1, 1 );
  dockLayout->addWidget( _playButton, 1, 9, 2, 2 );
  dockLayout->addWidget( stopButton, 1, 11, 1, 1 );
  dockLayout->addWidget( nextButton, 1, 12, 1, 1 );

  connect( _playButton, SIGNAL( clicked( )),
           this, SLOT( Play( )));

  connect( stopButton, SIGNAL( clicked( )),
             this, SLOT( Stop( )));

  connect( nextButton, SIGNAL( clicked( )),
             this, SLOT( GoToEnd( )));

  connect( prevButton, SIGNAL( clicked( )),
             this, SLOT( Restart( )));

  connect( repeatButton, SIGNAL( toggled( bool )),
             this, SLOT( Repeat( bool )));

  connect( _simSlider, SIGNAL( sliderPressed( )),
           this, SLOT( PlayAt( )));

//  connect( _simSlider, SIGNAL( sliderMoved( )),
//             this, SLOT( PlayAt( )));

  _simulationDock->setWidget( content );
  this->addDockWidget( Qt::/*DockWidgetAreas::enum_type::*/BottomDockWidgetArea,
                       _simulationDock );
}

void MainWindow::initSimColorDock( void )
{
  _simConfigurationDock = new QDockWidget( );
  _simConfigurationDock->setMinimumHeight( 100 );
  _simConfigurationDock->setMinimumWidth( 600 );
  _simConfigurationDock->setMaximumHeight( 400 );
  _simConfigurationDock->setSizePolicy( QSizePolicy::MinimumExpanding,
                                  QSizePolicy::MinimumExpanding );

  _tfEditor = new TransferFunctionEditor( );

  QWidget* container = new QWidget( );
  QVBoxLayout* verticalLayout = new QVBoxLayout( );
  QPushButton* applyColorButton = new QPushButton( QString( "Apply" ));

  verticalLayout->addWidget( _tfEditor );
  verticalLayout->addWidget( applyColorButton );

  container->setLayout( verticalLayout );
  _simConfigurationDock->setWidget( container );

  this->addDockWidget( Qt::/*DockWidgetAreas::enum_type::*/RightDockWidgetArea,
                       _simConfigurationDock );

  connect( applyColorButton, SIGNAL( clicked( void )),
             this, SLOT( UpdateSimulationColorMapping( void )));

}

void MainWindow::Play( void )
{
//  playIcon.swap( pauseIcon );

  if( _openGLWidget )
  {
    _openGLWidget->PlayPause( );

    if( _openGLWidget->player( )->isPlaying( ))
    {
      _playButton->setIcon( pauseIcon );
    }
    else
    {
      _playButton->setIcon( playIcon );
    }
  }
}

void MainWindow::Stop( void )
{
  if( _openGLWidget )
  {
    _openGLWidget->Stop( );
    _playButton->setIcon( playIcon );
    _startTimeLabel->setText(
          QString::number( (double)_openGLWidget->player( )->startTime( )));
  }
}

void MainWindow::Repeat( bool repeat )
{
  if( _openGLWidget )
  {
    _openGLWidget->Repeat( repeat );
  }
}

void MainWindow::PlayAt( void )
{
  if( _openGLWidget )
  {
    PlayAt( _simSlider->sliderPosition( ));
  }
}

void MainWindow::PlayAt( int sliderPosition )
{
  if( _openGLWidget )
  {

    _openGLWidget->Pause( );
    _openGLWidget->resetParticles( );

    int value = _simSlider->value( );
    float percentage = float( value - _simSlider->minimum( )) /
                       float( _simSlider->maximum( ) - _simSlider->minimum( ));
    _simSlider->setSliderPosition( sliderPosition );

    _openGLWidget->PlayAt( percentage );

  }
}

void MainWindow::Restart( void )
{
  if( _openGLWidget )
  {
    _openGLWidget->Restart( );

    if( _openGLWidget->player( )->isPlaying( ))
      _playButton->setIcon( pauseIcon );
    else
      _playButton->setIcon( playIcon );
  }
}

void MainWindow::GoToEnd( void )
{
  if( _openGLWidget )
  {
    _openGLWidget->GoToEnd( );
  }
}

void MainWindow::UpdateSimulationSlider( float percentage )
{
//  if( _openGLWidget->player( )->isPlaying( ))
  {

    _startTimeLabel->setText(
          QString::number( (double)_openGLWidget->player( )->currentTime( )));

//    float percentage = _openGLWidget->player( )->GetRelativeTime( );

    int total = _simSlider->maximum( ) - _simSlider->minimum( );

    int position = percentage * total;
//    std::cout << "Timer: " << percentage << " * "
//              << total << " = " << position << std::endl;

    _simSlider->setSliderPosition( position );
  }
}

void MainWindow::UpdateSimulationColorMapping( void )
{
//  TTransferFunction& colors = _tfEditor->getColorPoints( );

  _openGLWidget->changeSimulationColorMapping( _tfEditor->getColorPoints( ));
}

void MainWindow::changeEditorColorMapping( void )
{
  _tfEditor->setColorPoints( _openGLWidget->getSimulationColorMapping( ));
}