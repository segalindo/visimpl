/*
 * @file  MainWindow.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifdef VISIMPL_USE_GMRVLEX
#include <gmrvlex/version.h>
#endif
#ifdef VISIMPL_USE_DEFLECT
#include <deflect/version.h>
#endif
#ifdef VISIMPL_USE_NSOL
#include <nsol/version.h>
#endif
#ifdef VISIMPL_USE_ZEROEQ
#include <zeroeq/version.h>
#endif
#ifdef VISIMPL_USE_RETO
#include <reto/version.h>
#endif
#ifdef VISIMPL_USE_SCOOP
#include <scoop/version.h>
#endif
#ifdef VISIMPL_USE_SIMIL
#include <simil/version.h>
#endif
#ifdef VISIMPL_USE_PREFR
#include <prefr/version.h>
#endif
#include <stackviz/version.h>

#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>

#include <boost/bind.hpp>

#include <thread>

#include <sumrice/sumrice.h>

namespace stackviz
{

  MainWindow::MainWindow( QWidget* parent_ )
  : QMainWindow( parent_ )
  , _ui( new Ui::MainWindow )
  , _lastOpenedFileName( "" )
  , _simulationType( simil::TSimNetwork )
  , _summary( nullptr )
  , _player( nullptr )
  , _data( nullptr )
  , _subsetEventManager( nullptr )
  , _autoAddAvailableSubsets( true )
  , _autoCalculateCorrelations( false )
  , _autoAddEvents( true )
  , _autoAddEventSubset( true )
  , _dockSimulation( nullptr )
  , _playButton( nullptr )
  , _simSlider( nullptr )
  , _repeatButton( nullptr )
  , _goToButton( nullptr )
  , _playing( false )
  , _startTimeLabel( nullptr )
  , _endTimeLabel( nullptr )
  , _displayManager( nullptr )
  #ifdef VISIMPL_USE_ZEROEQ
  , _zeqConnection( false )
  , _subscriber( nullptr )
  , _publisher( nullptr )
  , _thread( nullptr )
  #endif
  , _contentWidget( nullptr )
  , _stackLayout( nullptr )
  , _columnsNumber( 3 )
  , resizingEnabled( true )
  {
    _ui->setupUi( this );

  #ifdef VISIMPL_USE_SIMIL
    _ui->actionOpenBlueConfig->setEnabled( true );
  #else
    _ui->actionOpenBlueConfig->setEnabled( false );
  #endif

    connect( _ui->actionQuit, SIGNAL( triggered( void )),
             QApplication::instance(), SLOT( quit( void )));

    // Connect about dialog
    connect( _ui->actionAbout, SIGNAL( triggered( void )),
             this, SLOT( aboutDialog( void )));

    _columnsNumber = 100;
    resizingEnabled = false;
  //  QTimer::singleShot( 0, this, SLOT( loadComplete( )));
  }

  void MainWindow::init( const std::string&
  #ifdef VISIMPL_USE_ZEROEQ
                         zeqUri
  #endif
    )
  {

    connect( _ui->actionOpenBlueConfig, SIGNAL( triggered( void )),
             this, SLOT( openBlueConfigThroughDialog( void )));

    connect( _ui->actionOpenCSVFiles, SIGNAL( triggered( void )),
             this, SLOT( openCSVFilesThroughDialog( void )));

    connect( _ui->actionOpenSubsetEventsFile, SIGNAL( triggered( void )),
             this, SLOT( openSubsetEventsFileThroughDialog( void )));

    initPlaybackDock( );

    connect( _dockSimulation->toggleViewAction( ), SIGNAL( toggled( bool )),
               _ui->actionTogglePlaybackDock, SLOT( setChecked( bool )));

    connect( _ui->actionTogglePlaybackDock, SIGNAL( triggered( void )),
             this, SLOT( togglePlaybackDock( void )));

    connect( _ui->actionShowDataManager, SIGNAL( triggered( void )),
             this, SLOT( showDisplayManagerWidget( void )));

    #ifdef VISIMPL_USE_ZEROEQ

    _setZeqUri( zeqUri );

    _ui->actionTogglePlaybackDock->setChecked( true );

    #endif

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
                                   simil::TSimulationType simulationType,
                                   const std::string& target,
                                   const std::string& subsetEventFile )
  {
    ( void ) target;

    _simulationType = simulationType;

    switch( _simulationType )
   {
     case simil::TSimSpikes:
     {
        simil::SpikeData* spikeData =
            new simil::SpikeData( fileName, simil::TBlueConfig, target );

        spikeData->reduceDataToGIDS( );

        _data = spikeData;

       simil::SpikesPlayer* player = new simil::SpikesPlayer( );
       player->LoadData( _data );
       _player = player;

       _subsetEventManager = _player->data( )->subsetsEvents( );

       break;
     }
     case simil::TSimVoltages:
  #ifdef SIMIL_USE_BRION
//       _player = new simil::VoltagesPlayer( fileName, reportLabel, true);
  #else
       std::cerr << "SimIL without Brion support." << std::endl;
       exit( -1 );
  #endif
  //     _deltaTime = _player->deltaTime( );
       break;

     default:
       VISIMPL_THROW("Cannot load an undefined simulation type.");

   }

    configurePlayer( );
    initSummaryWidget( );

    openSubsetEventFile( subsetEventFile, true );

    if( _autoAddEvents )
      _summary->generateEventsRep( );

    if( _autoAddEventSubset )
      _summary->importSubsetsFromSubsetMngr( );

    if( _displayManager )
      _displayManager->refresh( );
  }

  void MainWindow::openHDF5File( const std::string& networkFile,
                                 simil::TSimulationType simulationType,
                                 const std::string& activityFile,
                                 const std::string& subsetEventFile )
  {
    _simulationType = simulationType;

    simil::SpikesPlayer* player = new simil::SpikesPlayer( );
    player->LoadData( simil::TDataType::THDF5,  networkFile, activityFile );
    _player = player;

    _subsetEventManager = _player->data( )->subsetsEvents( );

    configurePlayer( );
    initSummaryWidget( );

    openSubsetEventFile( subsetEventFile, true );

    if( _autoAddEvents )
      _summary->generateEventsRep( );

    if( _autoAddEventSubset )
      _summary->importSubsetsFromSubsetMngr( );

    if( _displayManager )
      _displayManager->refresh( );
  }

  void MainWindow::openCSVFile( const std::string& networkFile,
                                simil::TSimulationType simulationType,
                                const std::string& activityFile,
                                const std::string& subsetEventFile )
  {
    _simulationType = simulationType;

    simil::SpikesPlayer* player = new simil::SpikesPlayer( );
    _player = player;

    player->LoadData( simil::TDataType::TCSV, networkFile, activityFile );

    _subsetEventManager = _player->data( )->subsetsEvents( );

    configurePlayer( );
    initSummaryWidget( );

    openSubsetEventFile( subsetEventFile, true );

    if( _autoAddEvents )
      _summary->generateEventsRep( );

    if( _autoAddEventSubset )
      _summary->importSubsetsFromSubsetMngr( );

    if( _displayManager )
      _displayManager->refresh( );
  }

  void MainWindow::openSubsetEventFile( const std::string& filePath,
                                        bool append )
  {
    if( filePath.empty( ) || !_subsetEventManager )
      return;

    if( !append )
      _subsetEventManager->clear( );

    _summary->clearEvents( );

    if( filePath.find( "json" ) != std::string::npos )
    {
      std::cout << "Loading JSON file: " << filePath << std::endl;
      _subsetEventManager->loadJSON( filePath );
    }
    else if( filePath.find( "h5" ) != std::string::npos )
    {
      std::cout << "Loading H5 file: " << filePath << std::endl;
      _subsetEventManager->loadH5( filePath );
      _autoCalculateCorrelations = true;
    }
    else
    {
      std::cout << "Subset Events file not found: " << filePath << std::endl;
    }
  }


  void MainWindow::openBlueConfigThroughDialog( void )
  {
  #ifdef VISIMPL_USE_SIMIL

     QString path = QFileDialog::getOpenFileName(
       this, tr( "Open BlueConfig" ), _lastOpenedFileName,
       tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ),
       nullptr, QFileDialog::DontUseNativeDialog );

     if( !path.isEmpty( ))
     {
       bool ok1, ok2;
       QInputDialog simTypeDialog;
       simil::TSimulationType simType;
       QStringList items = {"Spikes"};//, "Voltages"};

       QString text = QInputDialog::getItem(
         this, tr( "Please select simulation type" ),
         tr( "Type:" ), items, 0, false, &ok1 );

       if( !ok1 )
         return;

       if( text == items[0] )
       {
         simType = simil::TSimSpikes;
         ok2 = true;
       }
       else
       {
         simType = simil::TSimVoltages;

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

  void MainWindow::openCSVFilesThroughDialog( void )
  {
    QString pathNetwork = QFileDialog::getOpenFileName(
          this, tr( "Open CSV Network description file" ), _lastOpenedFileName,
          tr( "CSV (*.csv);; All files (*)" ),
          nullptr, QFileDialog::DontUseNativeDialog );

    if( !pathNetwork.isEmpty( ))
    {
      simil::TSimulationType simType = simil::TSimSpikes;

      QString pathActivity = QFileDialog::getOpenFileName(
            this, tr( "Open CSV Activity file" ), _lastOpenedFileName,
            tr( "CSV (*.csv);; All files (*)" ),
            nullptr, QFileDialog::DontUseNativeDialog );


      if ( !pathActivity.isEmpty( ))
      {
        _lastOpenedFileName = QFileInfo( pathNetwork ).path( );
        std::string networkFile = pathNetwork.toStdString( );
        std::string activityFile = pathActivity.toStdString( );
        openCSVFile( networkFile, simType, activityFile );
      }


    }
  }

  void MainWindow::openSubsetEventsFileThroughDialog( void )
  {
    QString filePath = QFileDialog::getOpenFileName(
              this, tr( "Open file containing subsets/events data" ),
              _lastOpenedSubsetsFileName,
              tr( "JSON (*.json);; All files (*)" ),
              nullptr, QFileDialog::DontUseNativeDialog );

    if( !filePath.isEmpty( ))
    {
      _lastOpenedSubsetsFileName = QFileInfo( filePath ).path( );

      openSubsetEventFile( filePath.toStdString( ), false );

      _summary->generateEventsRep( );
      _summary->importSubsetsFromSubsetMngr( );

      if( _displayManager )
        _displayManager->refresh( );

    }
  }

  void MainWindow::configurePlayer( void )
  {
    _startTimeLabel->setText(
        QString::number( (double)_player->startTime( )));

    _endTimeLabel->setText(
          QString::number( (double)_player->endTime( )));

  #ifdef SIMIL_USE_ZEROEQ
    _player->connectZeq( _zeqUri );

    _player->zeqEvents( )->frameReceived.connect(
        boost::bind( &MainWindow::UpdateSimulationSlider, this, _1 ));

    _player->zeqEvents( )->playbackOpReceived.connect(
        boost::bind( &MainWindow::ApplyPlaybackOperation, this, _1 ));
  #endif

  }

  void MainWindow::togglePlaybackDock( void )
  {
    if( _ui->actionTogglePlaybackDock->isChecked( ))
      _dockSimulation->show( );
    else
      _dockSimulation->close( );


    update( );
  }

  void MainWindow::showDisplayManagerWidget( void )
  {
    if( !_displayManager )
    {
      _displayManager = new DisplayManagerWidget( );
      _displayManager->init( _summary->eventWidgets(),
                             _summary->histogramWidgets( ));

      connect( _displayManager, SIGNAL( eventVisibilityChanged( unsigned int, bool )),
               _summary, SLOT( eventVisibility( unsigned int, bool )));

      connect( _displayManager, SIGNAL( subsetVisibilityChanged( unsigned int, bool )),
               _summary, SLOT( subsetVisibility( unsigned int, bool )));

      connect( _displayManager, SIGNAL( removeEvent( unsigned int )),
               _summary, SLOT( removeEvent( unsigned int )));

      connect( _displayManager, SIGNAL( removeHistogram( unsigned int )),
               _summary, SLOT( removeSubset( unsigned int )));

    }

    _displayManager->refresh( );

    _displayManager->show( );
  }

  void MainWindow::initPlaybackDock( )
  {
    _dockSimulation = new QDockWidget( );
    _dockSimulation->setMinimumHeight( 100 );
    _dockSimulation->setSizePolicy( QSizePolicy::MinimumExpanding,
                                    QSizePolicy::MinimumExpanding );

    _playing = false;
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
  //  QPushButton* repeatButton = new QPushButton( );
    _repeatButton = new QPushButton( );
    _repeatButton->setCheckable( true );
    _repeatButton->setChecked( false );

    _goToButton = new QPushButton( );
    _goToButton->setText( QString( "Play at..." ));

  //  QIcon playIcon;
  //  QIcon pauseIcon;
    QIcon stopIcon;
    QIcon nextIcon;
    QIcon prevIcon;
    QIcon repeatIcon;

    _playIcon.addFile( QStringLiteral( ":/icons/play.png" ), QSize( ),
                      QIcon::Normal, QIcon::Off );
    _pauseIcon.addFile( QStringLiteral( ":/icons/pause.png" ), QSize( ),
                       QIcon::Normal, QIcon::Off) ;
    stopIcon.addFile( QStringLiteral( ":/icons/stop.png" ), QSize( ),
                      QIcon::Normal, QIcon::Off );
    nextIcon.addFile( QStringLiteral( ":/icons/next.png" ), QSize( ),
                      QIcon::Normal, QIcon::Off );
    prevIcon.addFile( QStringLiteral( ":/icons/previous.png" ), QSize( ),
                      QIcon::Normal, QIcon::Off );
    repeatIcon.addFile( QStringLiteral( ":/icons/repeat.png" ), QSize( ),
                        QIcon::Normal, QIcon::Off );

    _playButton->setIcon( _playIcon );
    stopButton->setIcon( stopIcon );
    nextButton->setIcon( nextIcon );
    prevButton->setIcon( prevIcon );
    _repeatButton->setIcon( repeatIcon );

    _startTimeLabel = new QLabel( "" );
    _startTimeLabel->setSizePolicy( QSizePolicy::MinimumExpanding,
                                    QSizePolicy::Preferred );
    _endTimeLabel = new QLabel( "" );
    _endTimeLabel->setSizePolicy( QSizePolicy::Preferred,
                                    QSizePolicy::Preferred );

    unsigned int row = 2;
    dockLayout->addWidget( _startTimeLabel, row, 0, 1, 2 );
    dockLayout->addWidget( _simSlider, row, 1, 1, totalHSpan - 3 );
    dockLayout->addWidget( _endTimeLabel, row, totalHSpan - 2, 1, 1, Qt::AlignRight );

    row++;
    dockLayout->addWidget( _repeatButton, row, 7, 1, 1 );
    dockLayout->addWidget( prevButton, row, 8, 1, 1 );
    dockLayout->addWidget( _playButton, row, 9, 2, 2 );
    dockLayout->addWidget( stopButton, row, 11, 1, 1 );
    dockLayout->addWidget( nextButton, row, 12, 1, 1 );
    dockLayout->addWidget( _goToButton, row, 13, 1, 1 );

    connect( _playButton, SIGNAL( clicked( )),
             this, SLOT( PlayPause( )));

    connect( stopButton, SIGNAL( clicked( )),
               this, SLOT( Stop( )));

    connect( nextButton, SIGNAL( clicked( )),
               this, SLOT( GoToEnd( )));

    connect( prevButton, SIGNAL( clicked( )),
               this, SLOT( Restart( )));

    connect( _repeatButton, SIGNAL( clicked( )),
             this, SLOT( Repeat( )));

    connect( _simSlider, SIGNAL( sliderPressed( )),
             this, SLOT( PlayAt( )));

    connect( _goToButton, SIGNAL( clicked( )),
             this, SLOT( playAtButtonClicked( )));

    _dockSimulation->setWidget( content );
    this->addDockWidget( Qt::BottomDockWidgetArea,
                         _dockSimulation );

#ifdef VISIMPL_USE_ZEROEQ

#else

#endif
  }



  void MainWindow::initSummaryWidget( )
  {

    _summary = new visimpl::Summary( this, visimpl::T_STACK_EXPANDABLE );

    if( _simulationType == simil::TSimSpikes )
    {
      simil::SpikesPlayer* spikesPlayer =
          dynamic_cast< simil::SpikesPlayer* >( _player);

      _summary->Init( spikesPlayer->data( ));
      _summary->simulationPlayer( _player );

    }

    _stackLayout = new QGridLayout( );

    this->setCentralWidget( _summary );

    connect( _ui->actionAutoNamingSelections, SIGNAL( triggered( )),
             _summary, SLOT( toggleAutoNameSelections( )));

    _ui->actionFill_Plots->setChecked( true );
    connect( _ui->actionFill_Plots, SIGNAL( triggered( bool )),
             _summary, SLOT( fillPlots( bool )));

    connect( _summary, SIGNAL( histogramClicked( float )),
             this, SLOT( PlayAt( float )));

#ifdef VISIMPL_USE_ZEROEQ
    connect( _summary, SIGNAL( histogramClicked( visimpl::HistogramWidget* )),
               this, SLOT( HistogramClicked( visimpl::HistogramWidget* )));
#endif

    _ui->actionFocusOnPlayhead->setVisible( true );
    connect( _ui->actionFocusOnPlayhead, SIGNAL( triggered( )),
             _summary, SLOT( focusPlayback( )));



    if( _autoCalculateCorrelations )
    {
      calculateCorrelations( );
    }

    QTimer::singleShot( 0, _summary, SLOT( adjustSplittersSize( )));
  }

  void MainWindow::PlayPause( bool notify )
  {
    if( _playing )
      Pause( notify );
    else
      Play( notify );
  }

  void MainWindow::Play( bool notify )
  {

    if( _player )
    {
        _player->Play( );
        _playButton->setIcon( _pauseIcon );
        _playing = true;

        if( notify )
        {
  #ifdef VISIMPL_USE_ZEROEQ
        _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PLAY );
  #endif
        }
    }

  }

  void MainWindow::Pause( bool notify )
  {
    if( _player )
    {
      _player->Pause( );
      _playButton->setIcon( _playIcon );
      _playing = false;

      if( notify )
      {
    #ifdef VISIMPL_USE_ZEROEQ
      _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PAUSE );
    #endif
      }
    }
  }

  void MainWindow::Stop( bool notify )
  {
    if( _player )
    {
      _player->Stop( );
      _playButton->setIcon( _playIcon );
      _startTimeLabel->setText(
            QString::number( (double)_player ->startTime( )));
      _playing = false;
      if( notify )
      {
  #ifdef VISIMPL_USE_ZEROEQ
        _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::STOP );
  #endif
      }

    }
  }

  void MainWindow::Repeat( bool notify )
  {
    if( _player )
    {
      bool repeat = _repeatButton->isChecked( );
      _player->loop( repeat );

      if( notify )
      {
  #ifdef VISIMPL_USE_ZEROEQ
        _player ->zeqEvents( )->sendPlaybackOp( repeat ?
                                    zeroeq::gmrv::ENABLE_LOOP :
                                    zeroeq::gmrv::DISABLE_LOOP );
  #endif
      }

    }
  }

  void MainWindow::PlayAt( bool notify )
  {
    if( _player )
    {
      PlayAt( _simSlider->sliderPosition( ), notify );
    }
  }

  void MainWindow::PlayAt( float percentage, bool notify )
  {
    if( _player )
    {
      int sliderPos = percentage *
                      ( _simSlider->maximum( ) - _simSlider->minimum( )) +
                      _simSlider->minimum( );

      PlayAt( sliderPos, notify );
    }
  }

  void MainWindow::PlayAt( int sliderPosition, bool notify )
  {
    if( _player )
    {

      int value = _simSlider->value( );
      float percentage = float( value - _simSlider->minimum( )) /
                         float( _simSlider->maximum( ) - _simSlider->minimum( ));
      _simSlider->setSliderPosition( sliderPosition );

      _playButton->setIcon( _pauseIcon );

      _player->PlayAt( percentage );
      _playing = true;

      if( notify )
      {
  #ifdef VISIMPL_USE_ZEROEQ
      // Send event
      _player ->zeqEvents( )->sendFrame( _simSlider->minimum( ),
                             _simSlider->maximum( ),
                             sliderPosition );

      _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::PLAY );
  #endif
      }
    }
  }

  void MainWindow::Restart( bool notify )
  {
    if( _player )
    {
      bool playing = _playing;
      _player->Stop( );
      _playing = false;
      if( playing )
      {
       _player->Play( );
       _playing = true;
      }
      if( _playing )
        _playButton->setIcon( _pauseIcon );
      else
        _playButton->setIcon( _playIcon );

      if( notify )
      {
  #ifdef VISIMPL_USE_ZEROEQ
      _player->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::BEGIN );
  #endif
      }

    }
  }

  void MainWindow::GoToEnd( bool notify )
  {
    if( _player )
    {
      //TODO implement GOTOEND

      if( notify )
      {
  #ifdef VISIMPL_USE_ZEROEQ
      _player ->zeqEvents( )->sendPlaybackOp( zeroeq::gmrv::END );
  #endif
      }
    }
  }

  void MainWindow::UpdateSimulationSlider( float percentage )
  {
    double currentTime = percentage * ( _player->endTime( ) - _player->startTime( )) + _player->startTime( );

    _startTimeLabel->setText(

    QString::number( currentTime ));

    int total = _simSlider->maximum( ) - _simSlider->minimum( );

    int position = percentage * total;

    _simSlider->setSliderPosition( position );

    if( _summary )
      _summary->repaintHistograms( );

    if( _ui->actionFollowPlayhead->isChecked( ))
      _summary->focusPlayback( );
  }


  #ifdef VISIMPL_USE_ZEROEQ

  #ifdef VISIMPL_USE_GMRVLEX

    void MainWindow::ApplyPlaybackOperation( unsigned int playbackOp )
    {
      zeroeq::gmrv::PlaybackOperation operation =
          ( zeroeq::gmrv::PlaybackOperation ) playbackOp;

      switch( operation )
      {
        case zeroeq::gmrv::PLAY:
  //        std::cout << "Received play" << std::endl;
          Play( false );
          break;
        case zeroeq::gmrv::PAUSE:
          Pause( false );
  //        std::cout << "Received pause" << std::endl;
          break;
        case zeroeq::gmrv::STOP:
  //        std::cout << "Received stop" << std::endl;
          Stop( false );
          break;
        case zeroeq::gmrv::BEGIN:
  //        std::cout << "Received begin" << std::endl;
          Restart( false );
          break;
        case zeroeq::gmrv::END:
  //        std::cout << "Received end" << std::endl;
          GoToEnd( false );
          break;
        case zeroeq::gmrv::ENABLE_LOOP:
  //        std::cout << "Received enable loop" << std::endl;
          _zeqEventRepeat( true );
          break;
        case zeroeq::gmrv::DISABLE_LOOP:
  //        std::cout << "Received disable loop" << std::endl;
          _zeqEventRepeat( false );
          break;
        default:
          break;
      }

    }

    void MainWindow::_zeqEventRepeat( bool repeat )
    {
      _repeatButton->setChecked( repeat );
    }

    void MainWindow::HistogramClicked( visimpl::HistogramWidget* histogram )
    {
      const visimpl::GIDUSet* selection;

      if( histogram->filteredGIDs( ).size( ) == 0)
        selection = &_summary->gids( );
      else
        selection = &histogram->filteredGIDs( );

      std::vector< uint32_t > selected ( selection->begin( ),
                                     selection->end( ));

      std::cout << "Publishing selection of " << selected.size( ) << std::endl;
  //    std::vector< uint32_t > se( {1, 2, 3} );

  //     selectedIds( selected );
  //    selectedIds.setIds( selected );

  //    std::cout << "Sending selection of size " << selected.size( ) << std::endl;
      _publisher->publish( lexis::data::SelectedIDs( selected ));
    }

  #endif

  void MainWindow::_setZeqUri( const std::string& uri_ )
  {
    _zeqUri = uri_.empty( ) ? zeroeq::DEFAULT_SESSION : uri_;

    _zeqConnection = true;
    _subscriber = new zeroeq::Subscriber( _zeqUri );
    _publisher = new zeroeq::Publisher( _zeqUri );

    _subscriber->subscribe(
        lexis::data::SelectedIDs::ZEROBUF_TYPE_IDENTIFIER( ),
        [&]( const void* data_, const size_t size_ )
        { _onSelectionEvent( lexis::data::SelectedIDs::create( data_, size_ ));});

  //  pthread_create( &_subscriberThread, NULL, _Subscriber, _subscriber );
    _thread = new std::thread( [&]() { while( true ) _subscriber->receive( 10000 );});

  }

  //void* MainWindow::_Subscriber( void* subs )
  //{
  //  zeroeq::Subscriber* subscriber = static_cast< zeroeq::Subscriber* >( subs );
  //  while ( true )
  //  {
  //    subscriber->receive( 10000 );
  //  }
  //  pthread_exit( NULL );
  //}

  void MainWindow::_onSelectionEvent( lexis::data::ConstSelectedIDsPtr selected )
  {

  //  std::vector< unsigned int > selected =
  //      zeroeq::hbp::deserializeSelectedIDs( event_ );
    std::vector< uint32_t > ids = selected->getIdsVector( );
    std::cout << "Received selection with " << ids.size( ) << " elements" << std::endl;

    visimpl::GIDUSet selectedSet( ids.begin( ), ids.end( ));

    if( _summary )
    {
      visimpl::Selection selection;

      selection.gids = selectedSet;

      _summary->AddNewHistogram( selection, true );
    }

  }

  #endif

  void MainWindow::resizeEvent( QResizeEvent * event_ )
  {
    QMainWindow::resizeEvent( event_ );

    std::cout << "Window " << size( ).width( ) << "x" << size( ).height( ) << std::endl;
    if( _summary )
    std::cout << "Central " << _summary->size( ).width( ) << "x" << _summary->size( ).height( ) << std::endl;

    if( resizingEnabled )
    {
  //    unsigned int columnsWidth = width( ) / _columnsNumber;
  //    unsigned int currentWidth = width( ) - columnsWidth;
  //    unsigned int currentHeight =  _summary->heightPerRow( ) *
  //                                  _summary->histogramsNumber( );
  //
  //    _contentWidget->setMinimumWidth( width( ) );
  //    _contentWidget->setMinimumHeight( currentHeight + 2 );
  //
  //    std::cout << width( ) << " -> " << currentWidth << std::endl;
  ////    _summary->setMinimumWidth( currentWidth );
  //    _summary->resize( currentWidth, currentHeight );
    }
  }

  void MainWindow::playAtButtonClicked( void )
  {
    bool ok;
    double result =
        QInputDialog::getDouble( this, tr( "Set simulation time to play:"),
                                 tr( "Simulation time" ),
                                 ( double )_player->currentTime( ),
                                 ( double )_player->data( )->startTime( ),
                                 ( double )_player->data( )->endTime( ),
                                 3, &ok, Qt::Popup );

    if( ok )
    {
      float percentage = ( result - _player->data( )->startTime( )) /
          ( _player->data( )->endTime( ) - _player->data( )->startTime( ));

      percentage = std::max( 0.0f, std::min( 1.0f, percentage ));

      PlayAt( percentage, true );
    }
  }

  void MainWindow::loadComplete( void )
  {
    resizingEnabled = true;
    _summary->showMarker( false );
  }


  //void MainWindow::updateStackView( void )
  //{
  //  for( unsigned int i = 0 ; i < _summary->histogramsNumber( ); i++)
  //  {
  //    _summary->createSelection( i );
  //  }
  //}

  void MainWindow::addCorrelation( const std::string& subset )
  {
    _correlations.push_back( subset );
  }

  void MainWindow::calculateCorrelations( void )
  {
    visimpl::CorrelationComputer cc ( dynamic_cast< simil::SpikeData* >( _player->data( )));

    auto eventNames = _subsetEventManager->eventNames( );

    double deltaTime = 0.125;

    cc.configureEvents( eventNames, deltaTime );

    for( auto correl : _correlations )
    {
      auto result = cc.correlateSubset( correl, eventNames, deltaTime, 2600, 2900 );
    }

    for( auto correlationName : cc.correlationNames( ))
    {
      auto correlation = cc.correlation( correlationName );

      if( !correlation )
        continue;

      visimpl::Selection selection;
      selection.name = correlation->fullName;
      selection.gids = cc.getCorrelatedNeurons( correlation->fullName );

     _summary->AddNewHistogram( selection );
    }


  }

  void MainWindow::aboutDialog( void )
  {

    QString msj =
      QString( "<h2>ViSimpl - StackViz</h2>" ) +
      tr( "A multi-view visual analyzer of brain simulation data. " ) +
      "<br>" +
      tr( "Version " ) + stackviz::Version::getString( ).c_str( ) +
      tr( " rev (%1)<br>").arg(stackviz::Version::getRevision( )) +
      "<a href='https://gmrv.es/visimpl/'>https://gmrv.es/visimpl</a>" +
      "<h4>" + tr( "Build info:" ) + "</h4>" +
      "<ul>"

#ifdef VISIMPL_USE_DEFLECT
    "</li><li>Deflect " + DEFLECT_REV_STRING +
#else
    "</li><li>Deflect " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_FIRES
    "</li><li>FiReS " + FIRES_REV_STRING +
#else
    "</li><li>FiReS " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_GMRVLEX
    "</li><li>GmrvLex " + GMRVLEX_REV_STRING +
#else
    "</li><li>GmrvLex " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_NSOL
    "</li><li>Nsol " + NSOL_REV_STRING +
#else
    "</li><li>Nsol " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_PREFR
    "</li><li>prefr " + PREFR_REV_STRING +
#else
    "</li><li>prefr " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_RETO
    "</li><li>ReTo " + RETO_REV_STRING +
#else
    "</li><li>ReTo " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_SCOOP
    "</li><li>Scoop " + SCOOP_REV_STRING +
#else
    "</li><li>Scoop " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_SIMIL
    "</li><li>SimIL " + SIMIL_REV_STRING +
#else
    "</li><li>SimIL " + tr ("support not built.") +
#endif

#ifdef VISIMPL_USE_ZEROEQ
    "</li><li>ZeroEQ " + ZEROEQ_REV_STRING +
#else
    "</li><li>ZeroEQ " + tr ("support not built.") +
#endif

    "</li></ul>" +
    "<h4>" + tr( "Developed by:" ) + "</h4>" +
    "GMRV / URJC / UPM"
    "<br><a href='https://gmrv.es/gmrvvis'>https://gmrv.es/gmrvvis</a>"
    //"<br><a href='mailto:gmrv@gmrv.es'>gmrv@gmrv.es</a><br><br>"
    "<br>(C) 2015-2017<br><br>"
    "<a href='https://gmrv.es/gmrvvis'><img src=':/icons/logoGMRV.png'/></a>"
    "&nbsp;&nbsp;&nbsp;&nbsp;"
    "<a href='https://www.urjc.es'><img src=':/icons/logoURJC.png' /></a>"
    "&nbsp;&nbsp;&nbsp;&nbsp;"
    "<a href='https://www.upm.es'><img src=':/icons/logoUPM.png' /></a>";

    QMessageBox::about(this, tr( "About StackViz" ), msj );
  }

} // namespace stackviz
