//* tilemapcell.h *//

#ifndef TILEMAPCELL_H
#define TILEMAPCELL_H

#include "vector2.h"

#include <QWidget>

//TILETYPE ENUMERATION
enum TILE_ID {
	TID_EMPTY = 0,
	TID_FULL = 1,//fullAABB tile
	TID_45DEGpn = 2,//45-degree triangle, whose normal is (+ve,-ve)
	TID_45DEGnn = 3,//(+ve,+ve)
	TID_45DEGnp = 4,//(-ve,+ve)
	TID_45DEGpp = 5,//(-ve,-ve)
	TID_CONCAVEpn = 6,//1/4-circle cutout
	TID_CONCAVEnn = 7,
	TID_CONCAVEnp = 8,
	TID_CONCAVEpp = 9,
	TID_CONVEXpn = 10,//1/4/circle
	TID_CONVEXnn = 11,
	TID_CONVEXnp = 12,
	TID_CONVEXpp = 13,
	TID_22DEGpnS = 14,//22.5 degree slope
	TID_22DEGnnS = 15,
	TID_22DEGnpS = 16,
	TID_22DEGppS = 17,
	TID_22DEGpnB = 18,
	TID_22DEGnnB = 19,
	TID_22DEGnpB = 20,
	TID_22DEGppB = 21,
	TID_67DEGpnS = 22,//67.5 degree slope
	TID_67DEGnnS = 23,
	TID_67DEGnpS = 24,
	TID_67DEGppS = 25,
	TID_67DEGpnB = 26,
	TID_67DEGnnB = 27,
	TID_67DEGnpB = 28,
	TID_67DEGppB = 29,
	TID_HALFd = 30,//half-full tiles
	TID_HALFr = 31,
	TID_HALFu = 32,
	TID_HALFl = 33
};


//collision shape  "types"
enum COLLISION_TYPE {
	CTYPE_EMPTY = 0,
	CTYPE_FULL = 1,
	CTYPE_45DEG = 2,
	CTYPE_CONCAVE = 6,
	CTYPE_CONVEX = 10,
	CTYPE_22DEGs = 14,
	CTYPE_22DEGb = 18,
	CTYPE_67DEGs = 22,
	CTYPE_67DEGb = 26,
	CTYPE_HALF = 30
};

//---- edge state enumeration
enum EDGE_ID {
	EID_OFF = 0,
	EID_INTERESTING = 1,
	EID_SOLID = 2
};

class Vector2;

class TileMapCell : public QWidget
{
	
	Q_OBJECT
	
private:



protected:
	void paintEvent(QPaintEvent * /* event */);
		

public:

	int ID; //all tiles start empty
	int CTYPE;
	int i;//store the index fo this tile in the grid
	int j;
	
	TileMapCell *nU;//init neighbor info
	TileMapCell *nD;
	TileMapCell *nL;
	TileMapCell *nR;
	
	//edge info; all edges start off
	//NOTE: the format of edges will change as we try different collision detection methods..
	int eU;
	int eD;
	int eL;
	int eR;
	
	double gx;		//setup environmetal properties
	double gy;
	double d;
	
//	next = null;// setup the cell's linkedlist of objects
//	prev = null;
//	objcounter = 0;//this is probably uselesss but should help while debugging..
	
	Vector2 pos;	//setup collision properties
	int xw;
	int yw;
	double minx;
	double maxx;
	double miny;
	double maxy;
	
	//this stores tile-specific collision information
	int signx;
	int signy;
	double sx;
	double sy;
	
	int color_t;
	int HP;
	int unbreakable;


	TileMapCell(const int &i_in, const int &j_in, const int &x_in, const int &y_in, const int &xw_in, const int &yw_in, QWidget *parent = 0);
	~TileMapCell();
	
	void LinkU( TileMapCell *t );
	void LinkD( TileMapCell *t );
	void LinkL( TileMapCell *t );
	void LinkR( TileMapCell *t );

	//void Draw(); //This might be substituted by QWidget's PaintEvent.
	
	void SetState(const int &ID_in);
	void Clear();
	void UpdateNeighbors();
	void UpdateType();
	void UpdateEdges();
	
};

#endif
