#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QWidget>
#include <QPixmap>
#include <QSound>
#include <QPushButton>
#include <cmath>
#include <string>

using namespace std;

class TileMap;
//class MapLoader;
class Circle;
class QPixmap;
class Pad;
class MyButton;
class Vector2;

class QTimer;


class GameBoard : public QWidget
{
    Q_OBJECT
	
private:
	
	int stage;
	QPixmap bg[5], buttonicon, buttonicon2;
	QSound bgm, bgm2;
	MyButton *startgame, *replay;
	
private slots:
	void EnterFrame();
	
protected:
	void paintEvent(QPaintEvent * /* event */ );

public:
    GameBoard(QWidget* parent = 0);
    ~GameBoard();
    
    //void playSound(int track);
    
    Pad *pad;

	TileMap *tiles;

	Circle *demoObj;

	QTimer *timer;
    
public slots:
	void NextStage();
	void Replay();
	void end();

signals:

};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//app constants

const double MIN_F = 0;// of friction
const double MAX_F = 1;

const double MIN_B = 0;//bounce
const double MAX_B = 0.99;

const double MIN_G = 0;//grav
const double MAX_G = 1;

const int XMIN = 0;//these define the world bounds
const int XMAX = 400;
const int YMIN = 0;
const int YMAX = 400;

const int TILERAD = 20;
const int OBJRAD = 16;

const double OBJSPEED = 0.2;
const double MAXSPEED = 20;


//demo level
const string MAPSTR[] = { "0000000000000000000000000000000000000000000000000000000000000000",
						  "A6E00002000?E000000NA0070C0N00;10B0N00:10>0>L0060000F000@0GH0003",
						  "A3C0002100;?FNN00000000000000273692ACDEFGHI0000000000000@?:;0088",
						  "B0000012000;HHJKAAABB390000000000000083502030420000BBCCDDEEFF000" };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#endif  // GAMEBOARD_H
