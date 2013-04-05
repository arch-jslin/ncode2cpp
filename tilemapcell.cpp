//* TileMapCell.cpp *//

#include <cmath>
#include <QPainter>
#include <QPainterPath>

#include "circle.h"
#include "vector2.h"
#include "tilemapcell.h"

//this object stores all the info for a tile; note that a lot of this is superfluous
//(i.e any non-empty cell (i.e ID > 0) doesn't need drag/grav, and empty cells don't
//really need position/xw/yw)	
				
					   
TileMapCell::TileMapCell(const int &i_in, const int &j_in, const int &x_in, const int &y_in, const int &xw_in, const int &yw_in, QWidget *parent)
	:QWidget(parent)
{
	
	setPalette(QColor(255,255,255, 0));
    setFixedSize( xw_in*2, yw_in*2 );
	
	ID = TID_EMPTY; //all tiles start empty
	CTYPE = CTYPE_EMPTY;
	i = i_in;//store the index fo this tile in the grid
	j = j_in;
	nU = NULL;//init neighbor info
	nD = NULL;
	nL = NULL;
	nR = NULL;
	
	//edge info; all edges start off
	//NOTE: the format of edges will change as we try different collision detection methods..
	eU = EID_OFF;
	eD = EID_OFF;
	eL = EID_OFF;
	eR = EID_OFF;
	
	gx = 0;		//setup environmetal properties
	gy = GRAV;
	d = DRAG;
	
//	next = null;// setup the cell's linkedlist of objects
//	prev = null;
//	objcounter = 0;//this is probably uselesss but should help while debugging..
	
	pos = Vector2(x_in, y_in);	//setup collision properties
	xw = xw_in;
	yw = yw_in;
	minx = pos.x - xw;
	maxx = pos.x + xw;
	miny = pos.y - yw;
	maxy = pos.y + yw;
	
	//this stores tile-specific collision information
	signx = 0;
	signy = 0;
	sx = 0;
	sy = 0;
	
	unbreakable = 0;
	
	move(static_cast<int>(pos.x-3), static_cast<int>(pos.y-3));
	

}


TileMapCell::~TileMapCell()
{
	//Hmmmm....	
}

//these functions inits a tile by linking it to it's neighbors
//note: border tiles have null neighbors (by default/as part of the tile-construction)
//so we should simplyt NOT link them..
void TileMapCell::LinkU( TileMapCell *t )
{
	nU = t;
}
void TileMapCell::LinkD( TileMapCell *t )
{
	nD = t;
}
void TileMapCell::LinkL( TileMapCell *t )
{
	nL = t;
}
void TileMapCell::LinkR( TileMapCell *t )
{
	nR = t;
}

/* ignored part -----
//debug helpers
TileMapCell.prototype.ToString = function()
{
	string str = "(" + this.i + "," + this.j + ")";
	return str;
}
--------------------- */

void TileMapCell::paintEvent(QPaintEvent * /* event */)
{
	QPainterPath path;
	path.setFillRule( Qt::OddEvenFill );
	
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, 1);
	painter.setPen(Qt::NoPen);
	
	if( !unbreakable ) {
		if( color_t == 0 )  painter.setBrush( QBrush( QColor(128, 128, 0, 255*HP/2 )) );
		if( color_t == 1 )  painter.setBrush( QBrush( QColor(128, 0, 0,   255*HP/4 )) );
		if( color_t == 2 )  painter.setBrush( QBrush( QColor(0, 0, 128,   255*HP/8 )) );
	}
	else painter.setBrush( Qt::darkGray );
	
    switch( ID ) {
    	case TID_FULL:
    		path.moveTo(0,0);
			path.lineTo(xw*2, 0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_45DEGpn://45-degree triangle, whose normal is (+ve,-ve)
			path.moveTo(0,0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_45DEGnn://(+ve,+ve)
			path.moveTo(0, yw*2);
			path.lineTo(xw*2, 0);
			path.lineTo(xw*2, yw*2);
			path.closeSubpath();
			break;
		case TID_45DEGnp://(-ve,+ve)
			path.moveTo(0, 0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(xw*2, 0);
			path.closeSubpath();
			break;
		case TID_45DEGpp://(-ve,-ve)
			path.moveTo(xw*2, 0);
			path.lineTo(0, yw*2);
			path.lineTo(0, 0);
			path.closeSubpath();
			break;
		case TID_CONCAVEpn://1/4-circle cutout
			path.arcTo(0, -yw*2, xw*4, yw*4, 180, 360);
			path.moveTo(0,0);
			path.lineTo(xw*2, 0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_CONCAVEnn:
			path.arcTo(-xw*2, -yw*2, xw*4, yw*4, 270, 360);
			path.moveTo(0,0);
			path.lineTo(xw*2, 0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_CONCAVEnp:
			path.arcTo(-xw*2, 0, xw*4, yw*4, 0, 360);
			path.moveTo(0,0);
			path.lineTo(xw*2, 0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_CONCAVEpp:
			path.arcTo(0, 0, xw*4, yw*4, 90, 360);
			path.moveTo(0,0);
			path.lineTo(xw*2, 0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_CONVEXpn://1/4/circle
			path.arcTo(-xw*2, 0, xw*4, yw*4, 0, 360);
			path.closeSubpath();
			break;
		case TID_CONVEXnn:
			path.arcTo(0, 0, xw*4, yw*4, 90, 360);
			path.closeSubpath();
			break;
		case TID_CONVEXnp:
			path.arcTo(0, -yw*2, xw*4, yw*4, 180, 360);
			path.closeSubpath();
			break;
		case TID_CONVEXpp:
			path.arcTo(-xw*2, -yw*2, xw*4, yw*4, 270, 360);
			path.closeSubpath();
			break;
		case TID_22DEGpnS://22.5 degree slope
			path.moveTo(0, yw);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_22DEGnnS:
			path.moveTo(xw*2, yw);
			path.lineTo(0, yw*2);
			path.lineTo(xw*2, yw*2);
			path.closeSubpath();
			break;
		case TID_22DEGnpS:
			path.moveTo(0, 0);
			path.lineTo(xw*2, yw);
			path.lineTo(xw*2, 0);
			path.closeSubpath();
			break;
		case TID_22DEGppS:
			path.moveTo(xw*2, 0);
			path.lineTo(0, yw);
			path.lineTo(0, 0);
			path.closeSubpath();
			break;
		case TID_22DEGpnB:
			path.moveTo(0, 0);
			path.lineTo(xw*2, yw);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_22DEGnnB:
			path.moveTo(xw*2, 0);
			path.lineTo(0, yw);
			path.lineTo(0, yw*2);
			path.lineTo(xw*2, yw*2);
			path.closeSubpath();
			break;
		case TID_22DEGnpB:
			path.moveTo(xw*2, yw*2);
			path.lineTo(0, yw);
			path.lineTo(0, 0);
			path.lineTo(xw*2, 0);
			path.closeSubpath();
			break;
		case TID_22DEGppB:
			path.moveTo(0, 0);
			path.lineTo(0, yw*2);
			path.lineTo(xw*2, yw);
			path.lineTo(xw*2, 0);
			path.closeSubpath();
			break;
		case TID_67DEGpnS://67.5 degree slope
			path.moveTo(0, 0);
			path.lineTo(xw, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_67DEGnnS:
			path.moveTo(xw*2, 0);
			path.lineTo(xw, yw*2);
			path.lineTo(xw*2, yw*2);
			path.closeSubpath();
			break;
		case TID_67DEGnpS:
			path.moveTo(xw, 0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(xw*2, 0);
			path.closeSubpath();
			break;
		case TID_67DEGppS:
			path.moveTo(0, 0);
			path.lineTo(xw, 0);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_67DEGpnB:
			path.moveTo(0, 0);
			path.lineTo(xw, 0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_67DEGnnB:
			path.moveTo(xw*2, 0);
			path.lineTo(xw, 0);
			path.lineTo(0, yw*2);
			path.lineTo(xw*2, yw*2);
			path.closeSubpath();
			break;
		case TID_67DEGnpB:
			path.moveTo(0, 0);
			path.lineTo(xw, yw*2);
			path.lineTo(xw*2, yw*2);
			path.lineTo(xw*2, 0);
			path.closeSubpath();
			break;
		case TID_67DEGppB:
			path.moveTo(0, 0);
			path.lineTo(xw*2, 0);
			path.lineTo(xw, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_HALFd://half-full tiles
			path.moveTo(0, yw);
			path.lineTo(xw*2, yw);
			path.lineTo(xw*2, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		case TID_HALFr:
			path.moveTo(xw, 0);
			path.lineTo(xw*2, 0);
			path.lineTo(xw*2, yw*2);
			path.lineTo(xw, yw*2);
			path.closeSubpath();
			break;
		case TID_HALFu:
			path.moveTo(0, 0);
			path.lineTo(xw*2, 0);
			path.lineTo(xw*2, yw);
			path.lineTo(0, yw);
			path.closeSubpath();
			break;
		case TID_HALFl:
			path.moveTo(0, 0);
			path.lineTo(xw, 0);
			path.lineTo(xw, yw*2);
			path.lineTo(0, yw*2);
			path.closeSubpath();
			break;
		default:
			break;
   	}
   	
   	painter.drawPath( path );
}

/* ------- Ignored
void TileMapCell::Draw()
{
}
----------- */
//these functions are used to update the cell
//note: ID is assumed to NOT be "empty" state..
//if it IS the empty state, the tile clears itself

void TileMapCell::SetState(const int &ID_in)
{
	if(ID_in == TID_EMPTY)
	{
		Clear();
	}
	else
	{
		//set tile state to a non-emtpy value, and update it's edges and those of the neighbors
		int ran = rand()%12;           //random color
	
		if( ran >= 10 )      { HP = 8;  color_t = 2; }
		else if( ran >= 6 )  { HP = 4;  color_t = 1; }
		else                 { HP = 2;  color_t = 0; }
		ID = ID_in;
		UpdateType();
		UpdateEdges();    //IMPORTANT ********* this also draws *********
		UpdateNeighbors();//broadcasts changes to neighboring cells
	}	

}
void TileMapCell::Clear()
{
	//tile was on, turn it off
	ID = TID_EMPTY;
	UpdateType();
	UpdateEdges();//we don't reall need to do this, as this tile's edge states are based only on it's neighbors' states, not on iself
	UpdateNeighbors();
	
	//Draw();
	update();
}

//this function updates neighbor's edge states
//(i.e if this tile is activated, it's neighbor's edges must be updated to reflect the change..)
//note that we could simply call UpdateEdges() on all neighbors (so we don't duplicate code/etc..)
//this is very inefficient, but for now we care more about ease of implementation
void TileMapCell::UpdateNeighbors()
{
	if(nU != NULL)
	{
		nU->UpdateEdges();
	}
	if(nD != NULL)
	{
		nD->UpdateEdges();
	}
	if(nL != NULL)
	{
		nL->UpdateEdges();
	}
	if(nR != NULL)
	{
		nR->UpdateEdges();
	}	
	
}


//this converts a tile from implicitly-defined (via ID), to explicit (via properties)
void TileMapCell::UpdateType()
{
	if(0 < ID)
	{
		//tile is non-empty; collide
		if(ID < CTYPE_45DEG)
		{
			//TID_FULL
			CTYPE = CTYPE_FULL;
			signx = 0;
			signy = 0;
			sx = 0;
			sy = 0;
		}
		else if(ID < CTYPE_CONCAVE)
		{

			//45deg
			CTYPE = CTYPE_45DEG;
			if(ID == TID_45DEGpn)
			{
				signx = 1;
				signy = -1;
				sx = signx / SQRT2;//get slope _unit_ normal
				sy = signy / SQRT2;//since normal is (1,-1), length is sqrt(1*1 + -1*-1) = sqrt(2)				
			}
			else if(ID == TID_45DEGnn)
			{
				signx = -1;
				signy = -1;
				sx = signx / SQRT2;//get slope _unit_ normal
				sy = signy / SQRT2;//since normal is (1,-1), length is sqrt(1*1 + -1*-1) = sqrt(2)				
			}
			else if(ID == TID_45DEGnp)
			{
				signx = -1;
				signy = 1;
				sx = signx / SQRT2;//get slope _unit_ normal
				sy = signy / SQRT2;//since normal is (1,-1), length is sqrt(1*1 + -1*-1) = sqrt(2)				
			}
			else if(ID == TID_45DEGpp)
			{
				signx = 1;
				signy = 1;
				sx = signx / SQRT2;//get slope _unit_ normal
				sy = signy / SQRT2;//since normal is (1,-1), length is sqrt(1*1 + -1*-1) = sqrt(2)				
			}				
			else
			{
				return;
			}				
		}
		else if(ID < CTYPE_CONVEX)
		{

			//concave
			CTYPE = CTYPE_CONCAVE;
			if(ID == TID_CONCAVEpn)
			{
				signx = 1;
				signy = -1;
				sx = 0;
				sy = 0;
			}
			else if(ID == TID_CONCAVEnn)
			{
				signx = -1;
				signy = -1;
				sx = 0;
				sy = 0;
			}
			else if(ID == TID_CONCAVEnp)
			{
				signx = -1;
				signy = 1;
				sx = 0;
				sy = 0;
			}	
			else if(ID == TID_CONCAVEpp)
			{
				signx = 1;
				signy = 1;
				sx = 0;
				sy = 0;
			}
			else
			{
				return;
			}
		}
		else if(ID < CTYPE_22DEGs)
		{
					
			//convex
			CTYPE = CTYPE_CONVEX;
			if(ID == TID_CONVEXpn)
			{
				signx = 1;
				signy = -1;
				sx = 0;
				sy = 0;
			}
			else if(ID == TID_CONVEXnn)
			{
				signx = -1;
				signy = -1;
				sx = 0;
				sy = 0;
			}
			else if(ID == TID_CONVEXnp)
			{
				signx = -1;
				signy = 1;
				sx = 0;
				sy = 0;
			}
			else if(ID == TID_CONVEXpp)
			{
				signx = 1;
				signy = 1;
				sx = 0;
				sy = 0;
			}
			else
			{
				return;
			}
		}
		else if(ID < CTYPE_22DEGb)
		{
											
			//22deg small
			CTYPE = CTYPE_22DEGs;
			if(ID == TID_22DEGpnS)
			{
				signx = 1;
				signy = -1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*1) / slen;
				sy = (signy*2) / slen;				
			}
			else if(ID == TID_22DEGnnS)
			{
				signx = -1;
				signy = -1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*1) / slen;
				sy = (signy*2) / slen;
			}	
			else if(ID == TID_22DEGnpS)
			{
				signx = -1;
				signy = 1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*1) / slen;
				sy = (signy*2) / slen;
			}
			else if(ID == TID_22DEGppS)
			{
				signx = 1;
				signy = 1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*1) / slen;
				sy = (signy*2) / slen;
			}
			else
			{
				return;
			}							
		}
		else if(ID < CTYPE_67DEGs)
		{
														
			//22deg big
			CTYPE = CTYPE_22DEGb;
			if(ID == TID_22DEGpnB)
			{
				signx = 1;
				signy = -1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*1) / slen;
				sy = (signy*2) / slen;	
			}
			else if(ID == TID_22DEGnnB)
			{
				signx = -1;
				signy = -1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*1) / slen;
				sy = (signy*2) / slen;	
			}
			else if(ID == TID_22DEGnpB)
			{
				signx = -1;
				signy = 1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*1) / slen;
				sy = (signy*2) / slen;	
			}
			else if(ID == TID_22DEGppB)
			{
				signx = 1;
				signy = 1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*1) / slen;
				sy = (signy*2) / slen;	
			}
			else
			{
				return;
			}								
		}
		else if(ID < CTYPE_67DEGb)
		{
															
			//67deg small
			CTYPE = CTYPE_67DEGs;
			if(ID == TID_67DEGpnS)
			{
				signx = 1;
				signy = -1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*2) / slen;
				sy = (signy*1) / slen;	
			}
			else if(ID == TID_67DEGnnS)
			{
				signx = -1;
				signy = -1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*2) / slen;
				sy = (signy*1) / slen;
			}
			else if(ID == TID_67DEGnpS)
			{
				signx = -1;
				signy = 1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*2) / slen;
				sy = (signy*1) / slen;
			}	
			else if(ID == TID_67DEGppS)
			{
				signx = 1;
				signy = 1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*2) / slen;
				sy = (signy*1) / slen;
			}
			else
			{
				return;
			}									
		}
		else if(ID < CTYPE_HALF)
		{
								
			//67deg big
			CTYPE = CTYPE_67DEGb;
			if(ID == TID_67DEGpnB)
			{
				signx = 1;
				signy = -1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*2) / slen;
				sy = (signy*1) / slen;
			}
			else if(ID == TID_67DEGnnB)
			{
				signx = -1;
				signy = -1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*2) / slen;
				sy = (signy*1) / slen;
			}
			else if(ID == TID_67DEGnpB)
			{
				signx = -1;
				signy = 1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*2) / slen;
				sy = (signy*1) / slen;
			}	
			else if(ID == TID_67DEGppB)
			{
				signx = 1;
				signy = 1;
				double slen = sqrt(2*2 + 1*1);
				sx = (signx*2) / slen;
				sy = (signy*1) / slen;
			}	
			else
			{
				return;
			}
		}
		else
		{
			//half-full tile
			CTYPE = CTYPE_HALF;
			if(ID == TID_HALFd)
			{
				signx = 0;
				signy = -1;
				sx = signx;
				sy = signy;
			}
			else if(ID == TID_HALFu)
			{
				signx = 0;
				signy = 1;
				sx = signx;
				sy = signy;
			}
			else if(ID == TID_HALFl)
			{
				signx = 1;
				signy = 0;
				sx = signx;
				sy = signy;
			}
			else if(ID == TID_HALFr)
			{
				signx = -1;
				signy = 0;
				sx = signx;
				sy = signy;
			}
			else
			{
				return;
			}										
										
		}

	}
	else
	{
		//TID_EMPTY
		CTYPE = CTYPE_EMPTY;
		signx = 0;
		signy = 0;
		sx = 0;
		sy = 0;
	}		
	
	update();
}

//* UPDATE EDGES -------------------------------------------------------- *//

void TileMapCell::UpdateEdges()
{

//the rules for determining edge state are quite complicated.

	TileMapCell *n = NULL;
	
	n = nU;
	
	if( n != NULL ) {
		
		if( ID == TID_EMPTY )
		{
			if(n->ID == TID_EMPTY)
			{	
				//edge is off
				eU = EID_OFF;
			}
			else if(n->ID == TID_FULL)
			{
				eU = EID_SOLID;
			}
			else if(((n->signy*-1) <= 0) || n->ID == TID_67DEGpnS || n->ID == TID_67DEGnnS)
			{
				//nieghbor's surface points towards us; edge is interesting
				eU = EID_INTERESTING;
				//note that the <= is supposed to flag edges sharesd with half-fulls as interesting, but it 
				//might also have unwanted negative sideeffects
			}
			else
			{
				//neighbors surface points away; edge is solid
				eU = EID_SOLID;
			}			
		}
		else if( ID == TID_FULL )
		{
			//edge will either be off or interesting
			if(n->ID == TID_FULL)
			{
				eU = EID_OFF;
			}
			else if(n->ID == TID_EMPTY)
			{
				eU = EID_OFF;
			}
			else if( ((n->signy*-1) <= 0) || n->ID == TID_67DEGpnS || n->ID == TID_67DEGnnS )
			{
				eU = EID_INTERESTING;
			}
			else
			{
				eU = EID_OFF;
			}
		}
		else
		{
			//edges pointed at by this cell's normal can be off, interesting, or solid.	
			//edges opposite this cell's normal are off or interesting
			if(0 <= (signy*-1))
			{
				if(n->ID == TID_EMPTY)
				{
					eU = EID_OFF;
				}
				else if(n->ID == TID_FULL)
				{
					eU = EID_SOLID;
				}
				else if((n->signy*-1) <= 0 || n->ID == TID_67DEGpnS || n->ID == TID_67DEGnnS)
				{
					//nieghbor's surface points towards us; edge is interesting
					eU = EID_INTERESTING;
					//note that the <= is supposed to flag edges sharesd with half-fulls as interesting, but it 
					//might also have unwanted negative sideeffects
				}
				else
				{
					//neighbors surface points away; edge is solid
					eU = EID_SOLID;
				}
			}
			else
			{
	
				//edges pointing away from the cell normal can be off or interesting,
				//OR solid if the cell is 22/67 small
				
				if(ID == TID_67DEGppS || ID == TID_67DEGnpS)
				{
					if(n->ID == TID_EMPTY)
					{
						eU = EID_OFF;
					}
					else if(n->ID == TID_FULL)
					{
						eU = EID_SOLID;
					}				
					else if((n->signy*-1) <= 0 || n->ID == TID_67DEGpnS || n->ID == TID_67DEGnnS)
					{
						eU = EID_INTERESTING;
					}
					else if(0 < (n->signy*-1) || n->ID == TID_FULL)
					{
						eU = EID_SOLID;
					}
					else
					{
						eU = EID_OFF;
					}				
				}
				else 
				{
						
					if(n->ID == TID_FULL)
					{
						eU = EID_OFF;
					}
					else if(n->ID == TID_EMPTY)
					{
						eU = EID_OFF;
					}
					else if((n->signy*-1) <= 0 || n->ID == TID_67DEGpnS || n->ID == TID_67DEGnnS)
					{
						eU = EID_INTERESTING;
					}
					else
					{
						eU = EID_OFF;
					}
				}
			}	
		}
	}

	//Downside Neighbor
    n = nD;
    
    if( n != NULL ) {
    
		if(ID == TID_EMPTY)
		{
			if(n->ID == TID_EMPTY)
			{
				
				//edge is off
				eD = EID_OFF;
			}		
			else if(n->ID == TID_FULL)
			{
				eD = EID_SOLID;
			}
			else if((n->signy*1) <= 0 || n->ID == TID_67DEGppS || n->ID == TID_67DEGnpS)
			{
				//nieghbor's surface points towards us; edge is interesting
				eD = EID_INTERESTING;
				//note that the <= is supposed to flag edges sharesd with half-fulls as interesting, but it 
				//might also have unwanted negative sideeffects
			}
			else
			{
				//neighbors surface points away; edge is solid
				eD = EID_SOLID;
			}			
		}
		else if(ID == TID_FULL)
		{
			//edge will either be off or interesting
			if(n->ID == TID_FULL)
			{
				eD = EID_OFF;
			}
			else if(n->ID == TID_EMPTY)
			{
				eD = EID_OFF;
			}		
			else if((n->signy*1) <= 0 || n->ID == TID_67DEGppS || n->ID == TID_67DEGnpS)
			{
				eD = EID_INTERESTING;
			}
			else
			{
				eD = EID_OFF;
			}
		}
		else
		{
			//edges pointed at by this cell's normal can be off, interesting, or solid.	
			//edges opposite this cell's normal are off or interesting
			if(0 <= (signy*1))
			{
				if(n->ID == TID_EMPTY)
				{
					eD = EID_OFF;
				}
				else if(n->ID == TID_FULL)
				{
					eD = EID_SOLID;
				}
				else if((n->signy*1) <= 0 || n->ID == TID_67DEGppS || n->ID == TID_67DEGnpS)
				{
					//nieghbor's surface points towards us; edge is interesting
					eD = EID_INTERESTING;
					//note that the <= is supposed to flag edges sharesd with half-fulls as interesting, but it 
					//might also have unwanted negative sideeffects
				}
				else
				{
					//neighbors surface points away; edge is solid
					eD = EID_SOLID;
				}
			}
			else
			{
	
				if(ID == TID_67DEGpnS || ID == TID_67DEGnnS)
				{
					if(n->ID == TID_EMPTY)
					{
						eD = EID_OFF;
					}
					else if(n->ID == TID_FULL)
					{
						eD = EID_SOLID;
					}				
					else if((n->signy*1) <= 0 || n->ID == TID_67DEGppS || n->ID == TID_67DEGnpS)
					{
						eD = EID_INTERESTING;
					}
					else if(0 < (n->signy*1) || n->ID == TID_FULL)
					{
						eD = EID_SOLID;
					}
					else
					{
						eD = EID_OFF;
					}				
				}
				else 
				{
				
					
					if(n->ID == TID_FULL)
					{
						eD = EID_OFF;
					}
					else if(n->ID == TID_EMPTY)
					{
						eD = EID_OFF;
					}			
					else if((n->signy*1) <= 0 || n->ID == TID_67DEGppS || n->ID == TID_67DEGnpS)
					{
						eD = EID_INTERESTING;
					}
					else
					{
						eD = EID_OFF;
					}
				}
			}	
		}
	}

	//Rightside Neighbor
	n = nR;
	
	if( n != NULL ) {
		
		if(ID == TID_EMPTY)
		{
			if(n->ID == TID_EMPTY)
			{
				
				//edge is off
				eR = EID_OFF;
			}		
			else if(n->ID == TID_FULL)
			{
				eR = EID_SOLID;
			}
			else if((n->signx*1) <= 0 || n->ID == TID_22DEGpnS || n->ID == TID_22DEGppS)
			{
				//nieghbor's surface points towards us; edge is interesting
				eR = EID_INTERESTING;
				//note that the <= is supposed to flag edges sharesd with half-fulls as interesting, but it 
				//might also have unwanted negative sideeffects
			}
			else
			{
				//neighbors surface points away; edge is solid
				eR = EID_SOLID;
			}			
		}
		else if(ID == TID_FULL)
		{
			//edge will either be off or interesting
			if(n->ID == TID_FULL)
			{
				eR = EID_OFF;
			}
			else if(n->ID == TID_EMPTY)
			{
				eR = EID_OFF;
			}		
			else if((n->signx*1) <= 0 || n->ID == TID_22DEGpnS || n->ID == TID_22DEGppS)
			{
				eR = EID_INTERESTING;
			}
			else
			{
				eR = EID_OFF;
			}
		}
		else
		{
			//edges pointed at by this cell's normal can be off, interesting, or solid.	
			//edges opposite this cell's normal are off or interesting
			if(0 <= (signx*1))
			{
					//DEBUG
					//var mc = CreateMC("EMPTY_MC","j");
					//mc.lineStyle(2,0x228822,100);
					///mc.moveTo(pos.x, pos.y);
					//mc.lineTo(pos.x, pos.y - yw);			
				
				if(n->ID == TID_EMPTY)
				{
					eR = EID_OFF;
				}
				else if(n->ID == TID_FULL)
				{
					eR = EID_SOLID;
				}
				else if((n->signx*1) <= 0 || n->ID == TID_22DEGpnS || n->ID == TID_22DEGppS)
				{
					//nieghbor's surface points towards us; edge is interesting
					eR = EID_INTERESTING;
					//note that the <= is supposed to flag edges sharesd with half-fulls as interesting, but it 
					//might also have unwanted negative sideeffects
				}
				else
				{
					//neighbors surface points away; edge is solid
					eR = EID_SOLID;
				}
			}
			else
			{
	
				if(ID == TID_22DEGnnS || ID == TID_22DEGnpS)
				{
									
					if(n->ID == TID_EMPTY)
					{
						eR = EID_OFF;
						
						//DEBUG
						//var mc = CreateMC("EMPTY_MC","j");
						//mc.lineStyle(2,0x882222,100);
						//mc.moveTo(pos.x, pos.y);
						//mc.lineTo(pos.x - xw, pos.y - yw);					
						
					}
					else if(n->ID == TID_FULL)
					{
						eR = EID_SOLID;	
						
					}				
					else if((n->signx*1) <= 0 || n->ID == TID_22DEGpnS || n->ID == TID_22DEGppS)
					{
						eR = EID_INTERESTING;					
						
					}
					else if(n->ID == TID_FULL || (0 < (n->signx*1)) )
					{
						eR = EID_SOLID;					
						
					}
					else
					{
						
						eR = EID_OFF;
					}				
					
				}
				else 
				{
				
					if(n->ID == TID_FULL)
					{
						eR = EID_OFF;
					}
					else if(n->ID == TID_EMPTY)
					{
						eR = EID_OFF;
					}			
					else if((n->signx*1) <= 0 || n->ID == TID_22DEGpnS || n->ID == TID_22DEGppS)
					{
						eR = EID_INTERESTING;
					}
					else
					{
						eR = EID_OFF;
					}
				}
			}	
		}
	}


	//Leftside Neighbor
	n = nL;

	if( n != NULL ) {
	
		if(ID == TID_EMPTY)
		{
			if(n->ID == TID_EMPTY)
			{
				
				//edge is off
				eL = EID_OFF;
			}		
			else if(n->ID == TID_FULL)
			{
				eL = EID_SOLID;
			}
			else if((n->signx*-1) <= 0 || n->ID == TID_22DEGnnS || n->ID == TID_22DEGnpS)
			{
				//nieghbor's surface points towards us; edge is interesting
				eL = EID_INTERESTING;
				//note that the <= is supposed to flag edges sharesd with half-fulls as interesting, but it 
				//might also have unwanted negative sideeffects
			}
			else
			{
				//neighbors surface points away; edge is solid
				eL = EID_SOLID;
			}			
		}
		else if(ID == TID_FULL)
		{
			//edge will either be off or interesting
			if(n->ID == TID_FULL)
			{
				eL = EID_OFF;
			}
			else if(n->ID == TID_EMPTY)
			{
				eL = EID_OFF;
			}		
			else if((n->signx*-1) <= 0 || n->ID == TID_22DEGnnS || n->ID == TID_22DEGnpS)
			{
				eL = EID_INTERESTING;
			}
			else
			{
				eL = EID_OFF;
			}
		}
		else
		{
			//edges pointed at by this cell's normal can be off, interesting, or solid.	
			//edges opposite this cell's normal are off or interesting
			if(0 <= (signx*-1))
			{
				if(n->ID == TID_EMPTY)
				{
					eL = EID_OFF;
				}
				else if(n->ID == TID_FULL)
				{
					eL = EID_SOLID;
				}
				else if((n->signx*-1) <= 0 || n->ID == TID_22DEGnnS || n->ID == TID_22DEGnpS)
				{
					//nieghbor's surface points towards us; edge is interesting
					eL = EID_INTERESTING;
					//note that the <= is supposed to flag edges sharesd with half-fulls as interesting, but it 
					//might also have unwanted negative sideeffects
				}
				else
				{
					//neighbors surface points away; edge is solid
					eL = EID_SOLID;
				}
			}
			else
			{
				if(ID == TID_22DEGpnS || ID == TID_22DEGppS)
				{
					if(n->ID == TID_EMPTY)
					{
						eL = EID_OFF;
					}
					else if(n->ID == TID_FULL)
					{
						eL = EID_SOLID;
					}
					else if((n->signx*-1) <= 0 || n->ID == TID_22DEGnnS || n->ID == TID_22DEGnpS)
					{
						eL = EID_INTERESTING;
					}
					else if(0 < (n->signx*-1) || n->ID == TID_FULL)
					{
						eL = EID_SOLID;
					}
					else
					{
						eL = EID_OFF;
					}				
					
				}
				else 
				{			
				
					if(n->ID == TID_FULL)
					{
						eL = EID_OFF;
					}
					else if(n->ID == TID_EMPTY)
					{
						eL = EID_OFF;
					}			
					else if((n->signx*-1) <= 0 || n->ID == TID_22DEGnnS || n->ID == TID_22DEGnpS)
					{
						eL = EID_INTERESTING;
					}
					else
					{
						eL = EID_OFF;
					}
				}
			}	
		}
	}


	//update the cells graphics
	//Draw();
	update();
}
