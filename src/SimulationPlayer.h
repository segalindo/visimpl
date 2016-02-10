/*
 * SimulationPlayer.h
 *
 *  Created on: 1 de dic. de 2015
 *      Author: sgalindo
 */

#ifndef __SIMULATIONPLAYER_H__
#define __SIMULATIONPLAYER_H__

#include <brion/brion.h>
#include <brain/brain.h>

#include <unordered_map>

namespace visimpl
{
  class SimulationPlayer
  {

  public:

    SimulationPlayer( const std::string& blueConfigFilePath,
                      bool loadData = true );

    virtual ~SimulationPlayer( );

    virtual void LoadData( void );

    virtual void Clear( void );

    virtual void Frame( void );

    virtual void Play( void );

    virtual void Pause( void );

    virtual void Stop( void );

    virtual void GoTo( float timeStamp );

    virtual void PlayAt( float percentage );

    virtual float GetRelativeTime( void );

    bool isPlaying( void );

    void deltaTime( float deltaTime );
    virtual float deltaTime( void );

  //  void startTime( float startTime );
    float startTime( void );

  //  void endTime( float endTime );
    float endTime( void );

    float currentTime( void );

    void loop( bool loop );
    bool loop( void );

    brion::BlueConfig* blueConfig( void );
    brain::Circuit* circuit( void );
    const brion::GIDSet& gids( void );
    brion::Vector3fs positions( void );

  protected:

    virtual void FrameProcess( void ) = 0;
    virtual void Finished( void );

    float _currentTime;
    float _previousTime;

    float _deltaTime;

    float _startTime;
    float _endTime;

    bool _playing;
    bool _loop;
    bool _finished;

    std::string _blueConfigPath;

    brion::BlueConfig* _blueConfig;
    brain::Circuit* _circuit;
    brion::GIDSet _gids;

  };

  typedef brion::Spikes::iterator SpikesIter;
  typedef brion::Spikes::const_iterator SpikesCIter;

  typedef std::pair< SpikesIter, SpikesIter > SpikesRange;
  typedef std::pair< SpikesCIter, SpikesCIter > SpikesCRange;

  class SpikesPlayer : public SimulationPlayer
  {
  public:

    SpikesPlayer( const std::string& blueConfigFilePath,
                  bool loadData = true );

    virtual void LoadData( void );
    virtual void Clear( void );
    virtual void PlayAt( float percentage );
    virtual void Stop( void );

    virtual const brion::Spikes& Spikes( void );

    SpikesCRange spikesAtTime( float time );

    SpikesCRange spikesBetween( float startTime, float endTime );

    SpikesCRange spikesNow( void );

  protected:

    virtual void FrameProcess( void );

    SpikesCIter _previousSpike;
    SpikesCIter _currentSpike;

    brion::SpikeReport* _spikeReport;
  };

  class VoltagesPlayer : public SimulationPlayer
  {
  public:

    VoltagesPlayer( const std::string& blueConfigFilePath,
                    const std::string& target,
                    bool loadData = true,
                    const std::pair< float, float>* range = nullptr );


    virtual void LoadData( const std::pair< float, float>* range = nullptr );
    virtual void Clear( void );
    virtual void Stop( void );
    virtual void PlayAt( float percentage );

    void deltaTime( float deltaTime );

    virtual float getVoltage( uint32_t gid );

    virtual float minVoltage( void );
    virtual float maxVoltage( void );

    virtual float getNormVoltageFactor( void );

    class iterator;

    iterator begin( void );
    iterator end( void );
    iterator find( uint32_t gid );

    class const_iterator;

    const_iterator begin( void ) const;
    const_iterator end( void ) const;
    const_iterator find( uint32_t gid ) const;

//    std::pair< const_iterator, const_iterator > voltagesNow( void ) const;

  protected:

     virtual void FrameProcess( void );

     std::unordered_map< uint32_t, unsigned int > _gidRef;

     std::string _report;

     brion::CompartmentReport* _voltReport;
     brion::floatsPtr _currentFrame;

     float _minVoltage;
     float _maxVoltage;
     float _normalizedVoltageFactor;
  };

  typedef VoltagesPlayer::iterator VoltIter;
  typedef VoltagesPlayer::const_iterator VoltCIter;
  typedef std::pair< VoltIter, VoltIter > VoltagesRange;
  typedef std::pair< VoltCIter, VoltCIter > VoltagesCRange;

  class VoltagesPlayer::iterator
  : public std::iterator<std::bidirectional_iterator_tag, float>
  {
    friend class VoltagesPlayer;
    friend class VoltagesPlayer::const_iterator;

  public:

    iterator( )
    : _current( nullptr )
    { }

    inline bool operator == ( const iterator& other )
    {
      return this->_current == other._current;
    }

    inline bool operator != ( const iterator& other )
    {
      return this->_current != other._current;
    }

    inline iterator operator ++ ( )
    {
      iterator temp(* this);
      ++_current;
      return temp;
    }

    inline iterator operator -- ( )
    {
      iterator temp(* this);
      --_current;
      return temp;
    }

    inline float& operator *( )
    {
      return *_current;
    }

    inline float* operator -> ( )
    {
      return _current;
    }

  protected:

    iterator( float* current )
    : _current( current )
    { }

    float* _current;

  };

  class VoltagesPlayer::const_iterator
  :  public std::iterator<std::bidirectional_iterator_tag, const float>
  {
    friend class VoltagesPlayer;
    friend class VoltagesPlayer::iterator;

  public:
    const_iterator( )
    : _current( nullptr )
    { }

    const_iterator( const VoltagesPlayer::iterator& i )
    : _current( i._current )
    { }

    inline bool operator == ( const const_iterator& other )
    {
      return this->_current == other._current;
    }

    inline bool operator != ( const const_iterator& other )
    {
      return this->_current != other._current;
    }

    inline const_iterator operator ++ ( )
    {
      const_iterator temp(* this);
      ++_current;
      return temp;
    }

    inline const_iterator operator -- ( )
    {
      const_iterator temp(* this);
      --_current;
      return temp;
    }

    inline const float& operator *( )
    {
      return *_current;
    }

    inline const float* operator -> ( )
    {
      return _current;
    }

  protected:

    const_iterator( float* current )
    : _current( current )
    { }

    float* _current;

  };

}
#endif /* SRC_SIMULATIONPLAYER_H_ */