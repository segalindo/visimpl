/*
 * @file  MainWindow.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QLabel>
#include <QGridLayout>
#include <QScrollArea>
#include <QCheckBox>

// #include "SimulationPlayer.h"
#include <sumrice/sumrice.h>
#include <simil/simil.h>
// #include "SimulationSummaryWidget.h"

// #include "EditorTF/TransferFunctionEditor.h"

#ifdef VISIMPL_USE_ZEROEQ
#include <zeroeq/zeroeq.h>
#include <thread>
#ifdef VISIMPL_USE_LEXIS
#include <lexis/lexis.h>
#endif
#ifdef VISIMPL_USE_GMRVLEX
#include <gmrvlex/gmrvlex.h>
#endif

#endif

#include "ui_stackviz.h"

#include "DisplayManagerWidget.h"

namespace Ui
{
class MainWindow;
}

namespace stackviz
{

  class MainWindow
    : public QMainWindow
  {
    Q_OBJECT

  public:
    explicit MainWindow( QWidget* parent = 0 );
    ~MainWindow( void );

    void init( const std::string& zeqUri = "" );
    void showStatusBarMessage ( const QString& message );

    void openBlueConfig( const std::string& fileName,
                         simil::TSimulationType simulationType,
                         const std::string& report,
                         const std::string& subsetEventFile = "" );

    void openHDF5File( const std::string& networkFile,
                       simil::TSimulationType simulationType,
                       const std::string& activityFile = "",
                       const std::string& subsetEventFile = "" );

    void openCSVFile( const std::string& networkFile,
                      simil::TSimulationType simulationType,
                      const std::string& activityFile = "",
                      const std::string& subsetEventFile = "" );


    void openSubsetEventFile( const std::string& fileName,
                              bool append = false );

  public slots:

    void openBlueConfigThroughDialog( void );
    void openCSVFilesThroughDialog( void );
    void openSubsetEventsFileThroughDialog( void );

    void aboutDialog( void );
    void togglePlaybackDock( void );
    void showDisplayManagerWidget( void );

    void PlayPause( bool notify = true );
    void Play( bool notify = true );
    void Pause( bool notify = true );
    void Stop( bool notify = true );
    void Repeat( bool notify = true);
    void PlayAt( bool notify = true );
    void PlayAt( int, bool notify = true );
    void PlayAt( float, bool notify = true );
    void Restart( bool notify = true );
    void GoToEnd( bool notify = true );

    void UpdateSimulationSlider( float percentage );

    void addCorrelation( const std::string& subset );

    void calculateCorrelations( void );

  protected slots:

  #ifdef VISIMPL_USE_GMRVLEX

    void HistogramClicked( visimpl::HistogramWidget* );
    void ApplyPlaybackOperation( unsigned int playbackOp );
    void _zeqEventRepeat( bool repeat );

  #endif

    void playAtButtonClicked( void );

    void loadComplete( void );


  protected:

    void resizeEvent( QResizeEvent* event );
    void configurePlayer( void );

    void initSummaryWidget( void );
    void initPlaybackDock( void );


  #ifdef VISIMPL_USE_ZEROEQ

    void _onSelectionEvent( lexis::data::ConstSelectedIDsPtr selected );
    void _setZeqUri( const std::string& );

  #endif

    Ui::MainWindow* _ui;

    QString _lastOpenedFileName;
    QString _lastOpenedSubsetsFileName;

    simil::TSimulationType _simulationType;

    visimpl::Summary* _summary;
    QTimer _selectionsTimer;

    simil::SimulationPlayer* _player;
    simil::SimulationData* _data;
    simil::SubsetEventManager* _subsetEventManager;

    bool _autoAddAvailableSubsets;
    bool _autoCalculateCorrelations;
    bool _autoAddEvents;
    bool _autoAddEventSubset;

    // Playback Control
    QDockWidget* _dockSimulation;
    QPushButton* _playButton;
    CustomSlider* _simSlider;
    QPushButton* _repeatButton;
    QPushButton* _goToButton;
    bool _playing;

    QIcon _playIcon;
    QIcon _pauseIcon;

    QLabel* _startTimeLabel;
    QLabel* _endTimeLabel;

    DisplayManagerWidget* _displayManager;

  #ifdef VISIMPL_USE_ZEROEQ

    bool _zeqConnection;

    std::string _zeqUri;

    servus::URI _uri;
    zeroeq::Subscriber* _subscriber;
    zeroeq::Publisher* _publisher;

    std::thread* _thread;

  #endif

  private:

    QWidget* _contentWidget;
    QGridLayout* _stackLayout;
    unsigned int _columnsNumber;
    bool resizingEnabled;
    std::vector< QCheckBox* > _checkBoxes;

    std::vector< std::string > _correlations;

  };


} // namespace stackviz
