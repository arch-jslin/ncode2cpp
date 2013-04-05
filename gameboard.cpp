#include <QTimer>
#include <ctime>
#include <QPainter>
#include <QSound>
#include <stdlib.h>

#include "gameboard.h"

#include "vector2.h"
#include "mybutton.h"
#include "pad.h"
#include "circle.h"
#include "tilemap.h"
#include "tilemapcell.h"



GameBoard::GameBoard(QWidget* parent)
		: QWidget(parent), 
		  bgm("bgm01.wav"), 
		  bgm2("bgm02.wav"), 
		  buttonicon("breakout.png"),
		  buttonicon2("replay.png")
{
	srand( time(0) );
    //Constructor
    //setPalette(QPalette(QColor(200, 200, 200)));
    setFixedSize(640,480);
    
    stage = 0;
    
    bg[0].load("bg.png");
	bg[1].load("bg2.png");
	bg[2].load("bg3.png");
	bg[3].load("bg4.png");
	bg[4].load("bg5.png");
    
    startgame = new MyButton( buttonicon, this );
    startgame->move(420,20);
    
    connect( startgame, SIGNAL(clicked()), this, SLOT(NextStage()) );
    
    replay = new MyButton( buttonicon2, this );
    replay->move(420,80);
    
    connect( replay, SIGNAL(clicked()), this, SLOT(Replay()) );
    
    pad = new Pad(this);
    pad->submove( 200,377 );
    
    tiles = new TileMap(8,8,TILERAD,TILERAD, this);//map is 10x10 tiles, minus a 1-tile border on each edge.

	//make a dynamic object
	demoObj = new Circle( Vector2(72, 90) , OBJRAD, this);
	demoObj->pos.x = 73.0;
	demoObj->pos.y = 92.0;
	
	connect( demoObj, SIGNAL(died()), this, SLOT(end()));
	
	tiles->Build();
	tiles->SetTileStates(MAPSTR[0]);
	    
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(EnterFrame()));
    timer->start(10);
    
    QSound::play("bgm01.wav");
    
	update();
}

GameBoard::~GameBoard()
{
    //Deconstructor
    timer->stop();
//    bgm.setLoops(0);
//    bgm.stop();
    
    delete tiles;
    delete pad;
    delete demoObj;
    delete timer;
}

void GameBoard::paintEvent(QPaintEvent * /* event */)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, 1);	
	
	painter.drawPixmap(QRectF(0,0,640,480), bg[stage], QRectF(0,0,640,480));
}

void GameBoard::EnterFrame()
{
	demoObj->IntegrateVerlet();
	demoObj->CollideCirclevsTileMap( tiles->GetTile_V(demoObj->pos) );
	demoObj->CollideCirclevsPad    ( pad, tiles->GetTile_V(demoObj->pos) );
}

void GameBoard::NextStage()
{
	if( stage < 4 ) {

		timer->stop();
		
		stage++; 
		if( stage == 4 ) 
			QSound::play("bgm01.wav");
		else
			QSound::play("bgm02.wav");
			
		demoObj->oldpos.x = 73.5;
		demoObj->oldpos.y = 91.5;
		demoObj->pos.x = 73.5 + (rand()%100-50.0) / 250.0;
		demoObj->pos.y = 91.5 + (rand()%100-50.0) / 250.0;
		
		tiles->SetTileStates(MAPSTR[stage]);
		    
	    connect(timer, SIGNAL(timeout()), this, SLOT(EnterFrame()));
	    timer->start(10);

		update();		
	}
	else {
		
		timer->stop();
		QSound::play("bgm01.wav");
		
		tiles->SetTileStates(MAPSTR[stage]);
		    
	    //connect(timer, SIGNAL(timeout()), this, SLOT(EnterFrame()));
	    //timer->start(10);
		
		update();
	}
	pad->setFocus();
}

void GameBoard::end()
{
	stage = 4;
	NextStage();
}

void GameBoard::Replay()
{
	stage = -1;
	NextStage();
}
	