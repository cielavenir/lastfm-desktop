/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <lastfm/global.h>
#include <lastfm/RadioStation>
#include <QListWidget>
#include <QTextEdit>

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "lib/unicorn/StylableWidget.h"
#include "lib/unicorn/AnimatedPushButton.h"
#include "lib/unicorn/UnicornSession.h"
#include "PlaylistMeta.h"
#include "RadioStationListModel.h"
#include "Services/RadioService.h"

class SideBySideLayout;
class NowPlayingState;

class MainWidget : public StylableWidget
{
    Q_OBJECT
    
public:
    MainWidget( QWidget* parent = 0 );
    
signals:
    void startRadio(RadioStation);
    void widgetChanged( QWidget* );
    void sessionChanged( unicorn::Session* );
    
public slots:
    void onStartRadio(RadioStation rs);
    void onShowMoreRecentStations();
    void onCombo();
    void onYourFriends();
    void onBack();
    void onBackDelete();
    void onMoveFinished();
    void onForward();

    void onUserGotFriends();
    void onUserGotRecentStations();

    void onFriendActivated(const QModelIndex& idx);
    void onSlideStarted( QLayoutItem* next, QLayoutItem* prev );

    void rawrql();

private:
    SideBySideLayout* m_layout;
    NowPlayingState* m_nowPlaying;
    QStringList m_friends;
    RadioStationListModel m_recentModel;

    QStringList m_rawrqlItems;
};


class NowPlayingState : public QObject
{
    Q_OBJECT

public:
    NowPlayingState(QObject* parent = 0)
        :QObject(parent)
        ,m_playing(false)
    {
    }

    bool isPlaying() const
    {
        return m_playing;
    }

public slots:
    void onTuningIn(RadioStation)
    {
        if (!m_playing) {
            m_playing = true;
            emit playingStarted();
            emit playingStateChange(m_playing);
        }
    }

    void onStopped()
    {
        if (m_playing) {
            m_playing = false;
            emit playingStopped();
            emit playingStateChange(m_playing);
        }
    }

signals:
    void playingStarted();
    void playingStopped();
    void playingStateChange(bool);

private:
    bool m_playing;
};


// BackForwardControls parents a child widget and attaches an optional
// back button at the top-left and an optional forward button at the 
// top-right.
//
#include "lib/unicorn/StylableWidget.h"
class BackForwardControls : public StylableWidget
{
    Q_OBJECT;

public:

    // if backLabel is an empty string, there is no back button.
    // if nowPlaying is null, there is no now-playing button.

    BackForwardControls(const QString& backLabel, const QString& mainLabel, NowPlayingState* nowPlaying, QWidget* child)
        :m_mainLabel(0)
    {
        connect( &RadioService::instance(), SIGNAL(tuningIn(RadioStation)), SLOT(onTuningIn(RadioStation)));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins( 0, 0, 0, 0 );
        QHBoxLayout* rowLayout = new QHBoxLayout();
        //rowLayout->setContentsMargins( 4, 1, 4, 1 );
        if (!backLabel.isNull()) {
            QPushButton* button = new QPushButton(backLabel);
            button->setObjectName("back");
            button->setToolTip( tr( "Go back" ) );
            connect(button, SIGNAL(clicked()), SIGNAL(back()));
            rowLayout->addWidget(button, 1, Qt::AlignLeft);
            rowLayout->addStrut( button->sizeHint().height() - 8);
        }

        // always create the main label
        m_mainLabel = new QLabel(mainLabel);
        rowLayout->addWidget(m_mainLabel, 1, Qt::AlignCenter);

        if (nowPlaying) {
            // a button which toggles enable/disabled state (use css to make it visible/invisible)
            QMovie* movie = new QMovie( ":/now_playing.mng" );
            QPushButton* button = new AnimatedPushButton( movie, tr("Now Playing"));
            button->setObjectName( "NowPlayingButton" );
            button->setVisible(nowPlaying->isPlaying());
            button->setToolTip( tr( "Go to the Now Playing view" ) );
            connect(button, SIGNAL(clicked()), SIGNAL(forward()));
            connect(nowPlaying, SIGNAL(playingStateChange(bool)), button, SLOT(setVisible(bool)));
            rowLayout->addWidget(button, 1, Qt::AlignRight);
            rowLayout->addStrut( button->sizeHint().height() - 8);
        } else {
            rowLayout->addStretch(1);            // need this to get the label centered
        }
        if (!backLabel.isNull() || nowPlaying) {
            QWidget* w = new StylableWidget(this);
            w->setContentsMargins( 0, 0, 0, 0 );
            w->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
            w->setObjectName( "BackForwardsRow" );
            w->setLayout( rowLayout );
            layout->addWidget( w );
        }

        layout->addWidget(child);
    }

public slots:
    void onSessionChanged( unicorn::Session *newSession )
    {
        QString lblText( m_mainLabel->text() );
        if( lblText.startsWith( newSession->userInfo().name() ) )
        {
            lblText.replace( QString( newSession->userInfo().name() + "'s" ), QString( "Your" ) );
        }
        else if ( !m_prevUsername.isEmpty() )
        {
            lblText.replace( QString( "Your" ), QString( m_prevUsername + "'s" ) );
        }
        m_mainLabel->setText( lblText );
        m_prevUsername = newSession->userInfo().name();
    }

private slots:

    void onTuningIn( const RadioStation& rs)
    {
        if ( m_mainLabel && !rs.title().isEmpty() )
        {
            m_mainLabel->setText(rs.title());
        }
    }

signals:
    void back();
    void forward();

private:
    QLabel* m_mainLabel;
    QString m_prevUsername;
};

#endif
