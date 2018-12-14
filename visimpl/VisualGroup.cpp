/*
 * @file	VisualGroup.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#include "VisualGroup.h"

namespace visimpl
{

  static float invRGBInt = 1.0f / 255;

  VisualGroup::VisualGroup( )
  : _name( "" )
  , _cluster( nullptr )
  , _model( nullptr )
  , _source( nullptr )
  , _active( false )
  { }

  VisualGroup::VisualGroup( const std::string& name )
  : _name( name )
  , _cluster( nullptr )
  , _model( nullptr )
  , _source( nullptr )
  , _active( false )
  { }

  VisualGroup::~VisualGroup( )
  {

  }

  void VisualGroup::gids( const GIDUSet& gids_ )
  {
    _gids = gids_;
  }

  const GIDUSet& VisualGroup::gids( void ) const
  {
    return _gids;
  }

  void VisualGroup::model( prefr::Model* model_ )
  {
    assert( model_ );

    _model = model_;

    if( _cluster )
      _cluster->setModel( _model );
  }

  prefr::Model* VisualGroup::model( void ) const
  {
    return _model;
  }

  void VisualGroup::active( bool state, bool updateSourceState )
  {
    _active = state;

    if( updateSourceState )
      _source->active( state );
  }

  void VisualGroup::cluster( prefr::Cluster* cluster_ )
  {
    assert( cluster_ );

    _cluster = cluster_;

    if( _model )
      _cluster->setModel( _model );

    if( _source )
      _cluster->setSource( _source );
  }

  prefr::Cluster* VisualGroup::cluster( void ) const
  {
    return _cluster;
  }

  void VisualGroup::source( SourceMultiPosition* source_ )
  {
    assert( source_ );

    _source = source_;
    _source->restart( );
    _source->active( _active );

    if( _cluster )
      _cluster->setSource( _source );
  }

  SourceMultiPosition* VisualGroup::source( void ) const
  {
    return _source;
  }

  bool VisualGroup::active( void ) const
  {
    return _active;
  }

  void VisualGroup::colorMapping( const TTransferFunction& colors )
   {

     prefr::vectortvec4 gcolors;

     for( auto c : colors )
     {
       glm::vec4 gColor( c.second.red( ) * invRGBInt,
                         c.second.green( ) * invRGBInt,
                         c.second.blue( ) * invRGBInt,
                         c.second.alpha( ) * invRGBInt );

       gcolors.Insert( c.first, gColor );
     }

     _model->color = gcolors;

   }

  TTransferFunction VisualGroup::colorMapping( void ) const
   {
     TTransferFunction result;

     prefr::vectortvec4 colors = _model->color;

     auto timeValue = _model->color.times.begin( );
     for( auto c : _model->color.values )
     {
       QColor color( c.r * 255, c.g * 255, c.b * 255, c.a * 255 );
       result.push_back( std::make_pair( *timeValue, color ));

       ++timeValue;
     }


     return result;
   }

   void VisualGroup::sizeFunction( const TSizeFunction& sizes )
   {

     utils::InterpolationSet< float > newSize;
     for( auto s : sizes )
     {
       newSize.Insert( s.first, s.second );
     }
     _model->size = newSize;

   }

   TSizeFunction VisualGroup::sizeFunction( void ) const
   {
     TSizeFunction result;
     auto sizeValue = _model->size.values.begin( );
     for( auto s : _model->size.times)
     {
       result.push_back( std::make_pair( s, *sizeValue ));
       ++sizeValue;
     }

     return result;
   }

}



