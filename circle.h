
/* circle.h */

#ifndef CIRCLE_H
#define CIRCLE_H

#include <QWidget>
#include <QSound>
#include <cmath>
#include "vector2.h"

//these are used to report which type of collision was resolved
enum COLLISION_RESOLVE {
	COL_NONE = 0,//no collision was found/resolved
	COL_AXIS = 1,//collision was resolved along the x or y axis..
	COL_OTHER = 2//tile-specific axis was used to repolve collision (i.e slope normal, etc.)
};
															  
//basically, these flags are uysed to indicate if an object has been moved.
//COL_NONE means that it hasn't been moved.
//COL_AXIS means that it has been moved so that it is no longer colliding with
//one of the cell edges it was previously colliding with
//COL_OTHER means it has been moved, but we don't know how (i.e it might still
//be colliding with cell edges)														

//int OTYPE_CIRCLE = 1;

const double GRAV = 0.0;//.3 is a bit much, .1 is a bit "on the moon"..
const double DRAG = 0.999999;//0 means full drag, 1 is no drag
const double BOUNCE = 1;//must be in [0,1], where 1 means full bounce. but 1 seems to incite "the flubber effect" so use 0.9 as a practical upper bound
const double FRICTION = 0.00;

const double SQRT2 = sqrt(2.0);

class Vector2;
class TileMapCell;
class Pad;
class QSound;

class Circle : public QWidget
{
	
	Q_OBJECT;
	
private:


protected:
	void paintEvent(QPaintEvent * /* event */);
	
signals:
	void died();

public:

	int OTYPE;

	Vector2 pos;
	Vector2 oldpos;
	int r;

	QSound sound;

	Circle(Vector2 pos_in, const int &r_in, QWidget* parent = 0);
	~Circle() { }
	
	//void Draw(/*rend*/);//------------ This has been substituted by QWidget's PaintEvent.
	
	void ReportCollisionVsWorld(const double &px, const double &py, const double &dx, const double &dy, TileMapCell *obj);
	void IntegrateVerlet();
	void CollideCirclevsTileMap( TileMapCell *c );
	
	void CollideCirclevsPad    ( Pad *pad, TileMapCell *c );

	int ResolveCircleTile(const double &x, const double &y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);
	
	int ProjCircle_Full(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);
	int ProjCircle_45Deg(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);
	int ProjCircle_Concave(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);
	int ProjCircle_Convex(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);
	int ProjCircle_22DegS(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);
	int ProjCircle_22DegB(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);
	int ProjCircle_67DegS(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);
	int ProjCircle_67DegB(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);
	int ProjCircle_Half(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t);

};


#endif
