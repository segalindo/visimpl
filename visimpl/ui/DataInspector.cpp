/*
 * Copyright (c) 2015-2020 GMRV/URJC.
 *
 * Authors: Aaron Sujar <aaron.sujar@urjc.es>
 *
 * This file is part of SimIL <https://github.com/gmrvvis/SimIL>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "DataInspector.h"

#include <QGridLayout>

DataInspector::DataInspector(const QString &title, QWidget *parent)
: QGroupBox(title,parent)
, _gidsize(0)
, _spikesize(0)
, _labelGIDs(nullptr)
, _labelSpikes(nullptr)
, _labelStartTime(nullptr)
, _labelEndTime(nullptr)
, _simPlayer(nullptr)
{
    _labelGIDs = new QLabel(QString::number(_gidsize));
    _labelSpikes = new QLabel(QString::number(_spikesize));
    _labelStartTime = new QLabel("0");
    _labelEndTime = new QLabel("0");
    QGridLayout* oiLayout = new QGridLayout( );
    oiLayout->setAlignment( Qt::AlignTop );
    oiLayout->addWidget( new QLabel( "Network Information:" ), 0, 0, 1, 1 );
    oiLayout->addWidget( _labelGIDs, 0, 1, 1, 3 );
    oiLayout->addWidget( new QLabel( "Simulation Spikes: " ), 1, 0, 1, 1 );
    oiLayout->addWidget( _labelSpikes, 1, 1, 1, 3 );
    oiLayout->addWidget( new QLabel( "Start Time: " ), 4, 0, 1, 1 );
    oiLayout->addWidget( _labelStartTime, 4, 1, 1, 3 );
    oiLayout->addWidget( new QLabel( "End Time: " ), 5, 0, 1, 1 );
    oiLayout->addWidget( _labelEndTime, 5, 1, 1, 3 );
    setLayout( oiLayout );
}

void DataInspector::addWidget( QWidget *widget, int row, int column,
                               int rowSpan, int columnSpan,
                               Qt::Alignment alignment )
{
    static_cast< QGridLayout* >( layout( ))->addWidget( widget,
                                                        row, column,
                                                        rowSpan, columnSpan,
                                                        alignment );
}

void DataInspector::setSimPlayer(simil::SimulationPlayer * simPlayer_)
{
    _simPlayer =simPlayer_;
}

void DataInspector::paintEvent(QPaintEvent *event)
{
  if( _simPlayer != nullptr )
  {
    bool updated = false;
    if (_simPlayer->gidsSize( ) != _gidsize )
    {
      updated = true;
      _gidsize = _simPlayer->gidsSize( );
      _labelGIDs->setText(QString::number( _gidsize ));

    }

    simil::SpikesPlayer* spkPlay = dynamic_cast< simil::SpikesPlayer* >( _simPlayer );

    if( spkPlay != nullptr )
    {
      if( spkPlay->spikesSize( ) != _spikesize )
      {
        updated = true;
        _spikesize = spkPlay->spikesSize( );
        _labelSpikes->setText( QString::number( _spikesize ));
        _labelStartTime->setText( QString::number( spkPlay->startTime( )));
        _labelEndTime->setText( QString::number( spkPlay->endTime( )));
      }
    }

    if (updated)
      emit simDataChanged( );
  }

  QGroupBox::paintEvent( event );
}
