
/* circle.cpp */

#include "tilemapcell.h"
#include "pad.h"
#include "circle.h"

#include <cmath>
#include <QPainter>
#include <QSound>
#include <QRadialGradient>


Circle::Circle(Vector2 pos_in, const int &r_in, QWidget* parent)
	:QWidget(parent), sound("collision.wav")
{
	//OTYPE = OTYPE_CIRCLE;
        	
	OTYPE = 1; 
	
	pos = pos_in.clone();
	oldpos = pos.clone();
	r = abs(r_in);
	
	setPalette(QColor(255,255,255, 0));
    setFixedSize( r*2+3, r*2+3 );
}

/*------------ This has been substituted by QWidget's PaintEvent.
void Circle::Draw(rend)
{
	//rend.DrawCircle(this.pos, this.r);
}
----------------------------------------------------------------*/

void Circle::paintEvent(QPaintEvent * /* event */)
{
	QPainter painter(this);
	QRadialGradient gradient(QPointF(r*3/2,r*3/2), r, QPointF(r/3,r/3) );
	gradient.setColorAt(0.0, QColor(0,0,255) );
    gradient.setColorAt(1.0, QColor(255,0,0) );

	painter.setRenderHint(QPainter::Antialiasing, 1);
	painter.setPen(Qt::lightGray);
    painter.setBrush(gradient);
	painter.drawEllipse( QRect(1, 1, r*2+1, r*2+1) );
}


//=====================================
//simple physics functions

//(px,py) is projection vector, (dx,dy) is surface normal, obj is other object.

void Circle::ReportCollisionVsWorld(const double &px, const double &py, const double &dx, const double &dy, TileMapCell *obj)
{

	//collision reported to obj

	
	//calc velocity
	double vx = pos.x - oldpos.x;
	double vy = pos.y - oldpos.y;
	
	//find component of velocity parallel to collision normal
	double dp = (vx*dx + vy*dy);
	double nx = dp*dx;//project velocity onto collision normal
	
	double ny = dp*dy;//nx,ny is normal velocity
	
	double tx = vx-nx;//px,py is tangent velocity
	double ty = vy-ny;

	//we only want to apply collision response forces if the object is travelling into, and not out of, the collision
	double b,bx,by,f,fx,fy;
	if(dp < 0)
	{
		f = FRICTION;
		fx = tx*f;
		fy = ty*f;		
		
		b = 1+BOUNCE;//this bounce constant should be elsewhere, i.e inside the object/tile/etc..
		
		bx = (nx*b);
		by = (ny*b);
	
	}
	else
	{
		//moving out of collision, do not apply forces
		bx = by = fx = fy = 0;

	}


	pos.x += px;//project object out of collision
	pos.y += py;
	
	oldpos.x += px + bx + fx;//apply bounce+friction impulses which alter velocity
	oldpos.y += py + by + fy;
	
	if( obj != NULL ) {
		if( !obj->unbreakable ) {
			if( obj->HP > 1 ) {
				obj->HP -= 1;
				obj->update(); 
			}
			else {
				obj->Clear();
			}		
		}
	}
	
	if( sound.isFinished() )
		sound.play();
}


void Circle::IntegrateVerlet()
{
	double d = DRAG;
	double g = GRAV;
		
	double ox = oldpos.x; //we can't swap buffers since mcs/sticks point directly to vector2s..
	double oy = oldpos.y;
	
	double px,py;
	
	oldpos.x = px = pos.x;		//get vector values
	oldpos.y = py = pos.y;		//p = position  
					            //o = oldposition
	//integrate	
	pos.x += (d*px) - (d*ox);
	pos.y += (d*py) - (d*oy) + g;	
	
	move(static_cast<int>(pos.x), static_cast<int>(pos.y));
}



//this function detects a collision between a circle and an edge-based tilemap,
//and (if collision is found) calls ResolveCircleTile() to resolve the collision
//
//the main difference bewteen circle and aabb collision is that circles need to 
//collide against vertices as well as edges.
//
//since vertices in the tilemap are implicit, we need to do a bit more work than with AABBs.
//the logic is based upon the cell-index offset form the circle to the tile.
//
//if the circle is colliding with the current tile, we use the same logic as for the AABB.
//otherwise, we have to consider extra cases..

void Circle::CollideCirclevsPad    ( Pad *pad , TileMapCell *c )
{
	Vector2 posn = pos;
	double px,py,dx,dy;
	
	if( posn.y > 320 && posn.y < 360 ) {
		
		if( pos.x >= pad->x()-20 && pos.x <= pad->x()-13 + pad->width() ) {
			
			dx = posn.x - c->pos.x;
			dy = posn.y - c->pos.y;
			px = ( abs( dx ) + r ) - c->xw;
			py = ( abs( dy ) + r ) - c->yw;
			
			if( dy > 0 && py > 0 ) {
				ReportCollisionVsWorld(0, -py, 0, -1, NULL);
			}
		}
		else if( pos.x < pad->x()-20 ) {
			
			double vx = vx = pad->x()-20;
			double vy = vy = pad->y()-18;
			
			dx = pos.x - vx;//calc vert->circle vector		
			dy = pos.y - vy;
			
			double len = sqrt(dx*dx + dy*dy);
			double pen = r - len;
			
			if(0 < pen)
			{
				//vertex is in the circle; project outward
				if(len == 0)
				{
					//project out by 45deg
					dx = -1 / sqrt(2);
					dy = -1 / sqrt(2);
				}
				else
				{
					dx /= len;
					dy /= len;
				}

				ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, NULL);
			}
		}
		else if( pos.x > pad->x()-13 + pad->width() ) {
			
			double vx = pad->x()-13 + pad->width();
			double vy = pad->y()-18;
			
			dx = pos.x - vx;//calc vert->circle vector		
			dy = pos.y - vy;
			
			double len = sqrt(dx*dx + dy*dy);
			double pen = r - len;
			
			if(0 < pen)
			{
				//vertex is in the circle; project outward
				if(len == 0)
				{
					//project out by 45deg
					dx = -1 / sqrt(2);
					dy = -1 / sqrt(2);
				}
				else
				{
					dx /= len;
					dy /= len;
				}

				ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, NULL);
			}
		}
	}
}

void Circle::CollideCirclevsTileMap( TileMapCell *c )
{
	Vector2 posn = pos;
	int rad = r;
	//var c = tiles.GetTile_V(pos);
	
	if( posn.y > 380 ) {
		emit died();
		return;
	}
	
	double tx = c->pos.x;
	double ty = c->pos.y;
	int txw = c->xw;
	int tyw = c->yw;
	

	double dx = (pos.x - tx);//tile->obj delta
	double dy = (pos.y - ty);
	
	if(0 < c->ID)
	{
		//current tile is full!! 
		//for now, move object to oldpos; later, we'll need to determine projection direction and resolve
		//the collision
	
		double px = (txw + rad) - abs(dx);//penetration depth in x	
		double py = (tyw + rad) - abs(dy);//pen depth in y

		ResolveCircleTile(px,py,0,0,this,c);
	}


	int oH, oV;
		
//test in y		
		int crossV = false;//these flags will indicate whether or not we should collide vs. the diagonal tile
		int hitV = false;
			
		dy = (pos.y - ty);//tile->obj delta
		double py = (abs(dy) + rad) - tyw;//pen depth in y
				
		if(0 < py)
		{
			crossV = true;
			
			int eV;
			TileMapCell *nV = NULL;
			//int oV;//store edge, neighbor, and cell offset
			
			if(dy < 0)
			{
				eV = c->eU;
				nV = c->nU;
				oV = 1;
			}
			else
			{
				eV = c->eD;
				nV = c->nD;
				oV = -1;
			}
				
			if(0 < eV)
			{
				//edge is solid or interesting
				if(eV == EID_SOLID)
				{
					//we're colliding with a solid edge; resolve right away
					hitV = COL_AXIS;
					ReportCollisionVsWorld(0,py*oV, 0, oV, nV);
				}
				else
				{
					//edge is interesting; resolve using tile-specific function
					hitV = ResolveCircleTile(0,py,0,oV,this,nV);
				}
			}


		}

//test in x
		int crossH = false;
		int hitH = false;
		
		//double dx = (pos.x - tx);//tile->obj delta	
		double px = (abs(dx) + rad) - txw;//penetration depth in x	

		if(0 < px)
		{
			crossH = true;//aabb crosses horizontal edge
	
			int eH;
			TileMapCell *nH = NULL;
			//int oH;
			
			if(dx < 0)
			{
				eH = c->eL;
				nH = c->nL;
				oH = 1;
			}
			else
			{
				eH = c->eR;
				nH = c->nR;
				oH = -1;			
			}
		
			if(0 < eH)
			{
				//edge is solid or interesting
				if(eH == EID_SOLID)
				{
					//we're colliding with a solid edge; resolve right away
					hitH = COL_AXIS;
					ReportCollisionVsWorld(px*oH, 0, oH, 0, nH);
				}
				else
				{
					//edge is interesting; resolve using tile-specific function
					hitH = ResolveCircleTile(px,0,oH,0,this,nH);
				}
			}
		}		
		
		//we should collide vs. the diagonal ONLY if the following are true:
		//-object overlaps cell edge vertically but wasn't moved vertically to resolve this
		//-object overlaps cell edge horizontally but wasn't moved horizontally to resolve this
		//-adx and ady should both NOT be 0; if they ARE 0, the object is in the center of the tile
		//and thus (since objects must be smaller than tiles) should never be able to collide
		//with the diagonal

		//NOTE: the diagonal testing is VERY inefficient!
		
		if((crossH)&&(hitH != COL_AXIS)&&(crossV)&&(hitV != COL_AXIS))
		{

			//we didn't collide with horiz or vert neighbors; test the diagonal neighbor
			//NOTE: just as we used the current cell's edges and the h/v neighbor's cell states
			//to test horizontally/vertically, we use the h/v neighbor's edges and the diagonal
			//neighb's cell state to test diagonally
			
			//note that we can assume that the object is hitting the diagonal edges. we KNOW this
			//due to the testD flag.

			TileMapCell *dTile = NULL;//this should hold a pointer to the diagonal neighbot, IF we're colliding
			//int hit = false;//flag to indocate if we should resolve collision or not
			
			int eH, eV;
			
			//the basic logic for a circle is: if EITHER edge is solid, we're colliding with the 
			//corner vertex and we need to project out. otherwise, if at least one edge is
			//non-off, we need to investigate.
			
			if((dx < 0) && (dy < 0))
			{
				//test top-left neighbor

				eH = c->nU->eL;
				eV = c->nL->eU;
				dTile = c->nU->nL;
				
			}
			else if((dx < 0) && (0 < dy))
			{
				//test bottom-left neighbor

				eH = c->nD->eL;
				eV = c->nL->eD;
				dTile = c->nD->nL;
				
			}			
			else if((0 < dx) && (0 < dy))
			{
				//test bottom-right neighbor
				
				eH = c->nD->eR;
				eV = c->nR->eD;
				dTile = c->nD->nR;

			}			
			else if((0 < dx) && (dy < 0))
			{
				//test top-right neighbor
				
				eH = c->nU->eR;
				eV = c->nR->eU;
				dTile = c->nU->nR;
				
			}
			else
			{
				//object is in exact center of current cell.. we should never have reached here because
				//they could never have touched any cell edges
				//"WARNING!! we're colliding vs. diagonal but the object is in the dead center of it's cell!!"
				eH = EID_OFF;
				eV = EID_OFF;
			}

			
			if(0 < (eH + eV))
			{
				//at least one edge is non-off
				if((eH == EID_SOLID) || (eV == EID_SOLID))
				{
					//at least one of the edges is solid; project out of the corresponding corner vertex
					double vx = dTile->pos.x + (oH*dTile->xw);
					double vy = dTile->pos.y + (oV*dTile->yw);
					
					double dx = pos.x - vx;//calc vert->circle vector		
					double dy = pos.y - vy;
					
					double len = sqrt(dx*dx + dy*dy);
					double pen = r - len;
					
					if(0 < pen)
					{
						//vertex is in the circle; project outward
						if(len == 0)
						{
							//project out by 45deg
							dx = oH / SQRT2;
							dy = oV / SQRT2;
						}
						else
						{
							dx /= len;
							dy /= len;
						}

						ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, dTile);
					}
				}
				else
				{
					//at least one of the edges is interesting; investigate
					//note that we need to update the penetration info since 
					//we may have projected the object horiz/vert
					
					dx = (pos.x - dTile->pos.x);//tile->obj delta
					dy = (pos.y - dTile->pos.y);					
					px = (abs(dx) + rad) - dTile->xw;//penetration depth in x	
					py = (abs(dy) + rad) - dTile->yw;//penetration depth in y
					
					ResolveCircleTile(px,py,oH,oV,this,dTile);
					
				}
			}
			else
			{
				//both edges are off; do nothing
			}

		}
}



//this function resolves the collision between an object and a tile,
//based on the tile type. (x,y) is the  projection vector.
//this function returns true IF it moves the object by the specified
//projection vector

//NOTE: the cell offset vector (oH,oV) relative to the _tile_, i.e if the
//circle is being tested against the tile to its left, oH = 1

/* --------------- IGNORE THESE...   JUST FOR REFERENCES ----------------------
Proj_CircleTile = new Object();//hash object to hold tile-specific collision functions
Proj_CircleTile[CTYPE_FULL] = ProjCircle_Full;
Proj_CircleTile[CTYPE_45DEG] = ProjCircle_45Deg;
Proj_CircleTile[CTYPE_CONCAVE] = ProjCircle_Concave;
Proj_CircleTile[CTYPE_CONVEX] = ProjCircle_Convex;
Proj_CircleTile[CTYPE_22DEGs] = ProjCircle_22DegS;
Proj_CircleTile[CTYPE_22DEGb] = ProjCircle_22DegB;
Proj_CircleTile[CTYPE_67DEGs] = ProjCircle_67DegS;
Proj_CircleTile[CTYPE_67DEGb] = ProjCircle_67DegB;
Proj_CircleTile[CTYPE_HALF] = ProjCircle_Half;
------------------------------------------------------------------ */

int Circle::ResolveCircleTile(const double &x, const double &y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{
	if( 0 < t->ID )
	{
		switch( t->CTYPE ) {
			case CTYPE_FULL:
				return ProjCircle_Full(x,y,oH,oV,obj,t);
				break;
			case CTYPE_45DEG:
				return ProjCircle_45Deg(x,y,oH,oV,obj,t);
				break;
			case CTYPE_CONCAVE:
				return ProjCircle_Concave(x,y,oH,oV,obj,t);
				break;
			case CTYPE_CONVEX:
				return ProjCircle_Convex(x,y,oH,oV,obj,t);
				break;
			case CTYPE_22DEGs:
				return ProjCircle_22DegS(x,y,oH,oV,obj,t);
				break;
			case CTYPE_22DEGb:
				return ProjCircle_22DegB(x,y,oH,oV,obj,t);
				break;
			case CTYPE_67DEGs:
				return ProjCircle_67DegS(x,y,oH,oV,obj,t);
				break;
			case CTYPE_67DEGb:
				return ProjCircle_67DegB(x,y,oH,oV,obj,t);
				break;
			case CTYPE_HALF:
				return ProjCircle_Half(x,y,oH,oV,obj,t);
				break;
			default:
			//do nothing;
				break;
		}
	}
	else
	{
		//"ResolveCircleTile() was called with an empty (or unknown) tile!)"
		return false;
	}
	return false;
}


int Circle::ProjCircle_Full(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{
	//if we're colliding vs. the current cell, we need to project along the
	//smallest penetration vector.
	//if we're colliding vs. horiz. or vert. neighb, we simply project horiz/vert
	//if we're colliding diagonally, we need to collide vs. tile corner
		
		if(oH == 0)
		{
			if(oV == 0)
			{

				//collision with current cell
				if(x < y)
				{					
					//penetration in x is smaller; project in x
					double dx = obj->pos.x - t->pos.x;//get sign for projection along x-axis
					
			
					
					//NOTE: should we handle the delta == 0 case?! and how? (project towards oldpos?)
					if(dx < 0)
					{
						obj->ReportCollisionVsWorld(-x,0,-1,0,t);
						return COL_AXIS;
					}
					else
					{
						obj->ReportCollisionVsWorld(x,0,1,0,t);
						return COL_AXIS;
					}
				}
				else
				{		
					//penetration in y is smaller; project in y		
					double dy = obj->pos.y - t->pos.y;//get sign for projection along y-axis

					//NOTE: should we handle the delta == 0 case?! and how? (project towards oldpos?)					
					if(dy < 0)
					{
						obj->ReportCollisionVsWorld(0,-y,0,-1,t);
						return COL_AXIS;
					}
					else
					{
						obj->ReportCollisionVsWorld(0,y,0,1,t);
						return COL_AXIS;
					}				
				}					
			}
			else
			{
				//collision with vertical neighbor
				obj->ReportCollisionVsWorld(0,y*oV,0,oV,t);

				return COL_AXIS;
			}
		}
		else if(oV == 0)
		{
			//collision with horizontal neighbor
			obj->ReportCollisionVsWorld(x*oH,0,oH,0,t);
			return COL_AXIS;
		}
		else
		{			
			//diagonal collision
			
			//get diag vertex position
			double vx = t->pos.x + (oH*t->xw);
			double vy = t->pos.y + (oV*t->yw);
			
			double dx = obj->pos.x - vx;//calc vert->circle vector		
			double dy = obj->pos.y - vy;
			
			double len = sqrt(dx*dx + dy*dy);
			double pen = obj->r - len;
			if(0 < pen)
			{
				//vertex is in the circle; project outward
				if(len == 0)
				{
					//project out by 45deg
					dx = oH / SQRT2;
					dy = oV / SQRT2;
				}
				else
				{
					dx /= len;
					dy /= len;
				}
				
				obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
				
				return COL_OTHER;
			}
		}
		
		return COL_NONE;
}


int Circle::ProjCircle_Half(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{

	//if obj is in a neighbor pointed at by the halfedge normal,
	//we'll never collide (i.e if the normal is (0,1) and the obj is in the DL.D, or R neighbors)
	//
	//if obj is in a neigbor perpendicular to the halfedge normal, it might
	//collide with the halfedge-vertex, or with the halfedge side.
	//
	//if obj is in a neigb pointing opposite the halfedge normal, obj collides with edge
	//
	//if obj is in a diagonal (pointing away from the normal), obj collides vs vertex
	//
	//if obj is in the halfedge cell, it collides as with aabb

	int signx = t->signx;
	int signy = t->signy;

	int celldp = (oH*signx + oV*signy);//this tells us about the configuration of cell-offset relative to tile normal
	if(0 < celldp)
	{
		//obj is in "far" (pointed-at-by-normal) neighbor of halffull tile, and will never hit
		return COL_NONE;
	}
	else if(oH == 0)
	{
		if(oV == 0)
		{
			//colliding with current tile
			int r = obj->r;
			double ox = (obj->pos.x - (signx*r)) - t->pos.x;//this gives is the coordinates of the innermost
			double oy = (obj->pos.y - (signy*r)) - t->pos.y;//point on the circle, relative to the tile center
			
	
			//we perform operations analogous to the 45deg tile, except we're using 
			//an axis-aligned slope instead of an angled one..
			double sx = signx;
			double sy = signy;
			
			//if the dotprod of (ox,oy) and (sx,sy) is negative, the corner is in the slope
			//and we need toproject it out by the magnitude of the projection of (ox,oy) onto (sx,sy)
			double dp = (ox*sx) + (oy*sy);
			if(dp < 0)
			{
				//collision; project delta onto slope and use this to displace the object
				sx *= -dp;//(sx,sy) is now the projection vector
				sy *= -dp;		
				
				
				double lenN = sqrt(sx*sx + sy*sy);
				double lenP = sqrt(x*x + y*y);
	
				if(lenP < lenN)
				{
					obj->ReportCollisionVsWorld(x,y,x/lenP, y/lenP, t );

					return COL_AXIS;
				}
				else
				{		
					obj->ReportCollisionVsWorld(sx,sy,t->signx,t->signy, t);

					return COL_OTHER;
				}
				return true;
			}			
			
		}
		else
		{
			//colliding vertically

			if(celldp == 0)
			{
	
				int r = obj->r;
				double dx = obj->pos.x - t->pos.x;
						
				//we're in a cell perpendicular to the normal, and can collide vs. halfedge vertex
				//or halfedge side
				if((dx*signx) < 0)
				{
					//collision with halfedge side
					obj->ReportCollisionVsWorld(0,y*oV,0,oV,t);
					
					return COL_AXIS;						
				}
				else
				{
					//collision with halfedge vertex
					double dy = obj->pos.y - (t->pos.y + oV*t->yw);//(dx,dy) is now the vector from the appropriate halfedge vertex to the circle
					
					double len = sqrt(dx*dx + dy*dy);
					double pen = r - len;
					if(0 < pen)
					{
						//vertex is in the circle; project outward
						if(len == 0)
						{
							//project out by 45deg
							dx = signx / SQRT2;
							dy = oV / SQRT2;
						}
						else
						{
							dx /= len;
							dy /= len;
						}
							
						obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
						
						return COL_OTHER;
					}					
					
				}
			}
			else
			{
				//due to the first conditional (celldp >0), we know we're in the cell "opposite" the normal, and so
				//we can only collide with the cell edge
				//collision with vertical neighbor
				obj->ReportCollisionVsWorld(0,y*oV,0,oV,t);
				
				return COL_AXIS;
			}
			
		}
	}
	else if(oV == 0)
	{
		//colliding horizontally
		if(celldp == 0)
		{
	
			int r = obj->r;
			double dy = obj->pos.y - t->pos.y;
						
			//we're in a cell perpendicular to the normal, and can collide vs. halfedge vertex
			//or halfedge side
			if((dy*signy) < 0)
			{
				//collision with halfedge side
				obj->ReportCollisionVsWorld(x*oH,0,oH,0,t);
				
				return COL_AXIS;						
			}
			else
			{
				//collision with halfedge vertex
				double dx = obj->pos.x - (t->pos.x + oH*t->xw);//(dx,dy) is now the vector from the appropriate halfedge vertex to the circle
					
				double len = sqrt(dx*dx + dy*dy);
				double pen = r - len;
				if(0 < pen)
				{
					//vertex is in the circle; project outward
					if(len == 0)
					{
						//project out by 45deg
						dx = signx / SQRT2;
						dy = oV / SQRT2;
					}
					else
					{
						dx /= len;
						dy /= len;
					}
							
					obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
					
					return COL_OTHER;
				}					
					
			}
		}
		else
		{			
			//due to the first conditional (celldp >0), we know w're in the cell "opposite" the normal, and so
			//we can only collide with the cell edge
			obj->ReportCollisionVsWorld(x*oH, 0, oH, 0, t);
			
			return COL_AXIS;
		}		
	}
	else
	{		
		//colliding diagonally; we know, due to the initial (celldp >0) test which has failed
		//if we've reached this point, that we're in a diagonal neighbor on the non-normal side, so
		//we could only be colliding with the cell vertex, if at all.

		//get diag vertex position
		double vx = t->pos.x + (oH*t->xw);
		double vy = t->pos.y + (oV*t->yw);
			
		double dx = obj->pos.x - vx;//calc vert->circle vector		
		double dy = obj->pos.y - vy;
			
		double len = sqrt(dx*dx + dy*dy);
		double pen = obj->r - len;
		if(0 < pen)
		{
			//vertex is in the circle; project outward
			if(len == 0)
			{
				//project out by 45deg
				dx = oH / SQRT2;
				dy = oV / SQRT2;
			}
			else
			{
				dx /= len;
				dy /= len;
			}

			obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
			
			return COL_OTHER;
		}		
		
	}
	
	return COL_NONE;
	
}


int Circle::ProjCircle_45Deg(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{

	//if we're colliding diagonally:
	//	-if obj is in the diagonal pointed to by the slope normal: we can't collide, do nothing
	//  -else, collide vs. the appropriate vertex
	//if obj is in this tile: perform collision as for aabb-ve-45deg
	//if obj is horiz OR very neighb in direction of slope: collide only vs. slope
	//if obj is horiz or vert neigh against direction of slope: collide vs. face
	
	int signx = t->signx;
	int signy = t->signy;	
	
	if(oH == 0)
	{
		if(oV == 0)
		{
			//colliding with current tile

			double sx = t->sx;
			double sy = t->sy;
			
			double lenP;

			double ox = (obj->pos.x - (sx*obj->r)) - t->pos.x;//this gives is the coordinates of the innermost
			double oy = (obj->pos.y - (sy*obj->r)) - t->pos.y;//point on the circle, relative to the tile center	

			//if the dotprod of (ox,oy) and (sx,sy) is negative, the innermost point is in the slope
			//and we need toproject it out by the magnitude of the projection of (ox,oy) onto (sx,sy)
			double dp = (ox*sx) + (oy*sy);		
			if(dp < 0)
			{
				//collision; project delta onto slope and use this as the slope penetration vector
				sx *= -dp;//(sx,sy) is now the penetration vector
				sy *= -dp;		
				
				//find the smallest axial projection vector
				if(x < y)
				{					
					//penetration in x is smaller
					lenP = x;
					y = 0;
					
					//get sign for projection along x-axis		
					if((obj->pos.x - t->pos.x) < 0)
					{
						x *= -1;
					}
				}
				else
				{		
					//penetration in y is smaller
					lenP = y;
					x = 0;
					
					//get sign for projection along y-axis		
					if((obj->pos.y - t->pos.y)< 0)
					{
						y *= -1;
					}			
				}

				double lenN = sqrt(sx*sx + sy*sy);
							
				if(lenP < lenN)
				{
					obj->ReportCollisionVsWorld(x,y,x/lenP, y/lenP, t);
					
					return COL_AXIS;
				}
				else
				{
					obj->ReportCollisionVsWorld(sx,sy,t->sx,t->sy,t);
					
					return COL_OTHER;
				}
			}			

		}
		else
		{
			//colliding vertically
			if((signy*oV) < 0)
			{
				//colliding with face/edge
				obj->ReportCollisionVsWorld(0,y*oV,0,oV,t);
				
				return COL_AXIS;
			}
			else
			{
				//we could only be colliding vs the slope OR a vertex
				//look at the vector form the closest vert to the circle to decide

				double sx = t->sx;
				double sy = t->sy;

				double ox = obj->pos.x - (t->pos.x - (signx*t->xw));//this gives is the coordinates of the innermost
				double oy = obj->pos.y - (t->pos.y + (oV*t->yw));//point on the circle, relative to the closest tile vert	

				//if the component of (ox,oy) parallel to the normal's righthand normal
				//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
				//then we project by the vertex, otherwise by the normal.
				//note that this is simply a VERY tricky/weird method of determining 
				//if the circle is in side the slope/face's voronoi region, or that of the vertex.											  
				double perp = (ox*-sy) + (oy*sx);
				if(0 < (perp*signx*signy))
				{
					//collide vs. vertex
					double len = sqrt(ox*ox + oy*oy);
					double pen = obj->r - len;
					if(0 < pen)
					{
						//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
						ox /= len;
						oy /= len;

						obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
						
						return COL_OTHER;
					}					
				}
				else
				{
					//collide vs. slope
					
					//if the component of (ox,oy) parallel to the normal is less than the circle radius, we're
					//penetrating the slope. note that this method of penetration calculation doesn't hold
					//in general (i.e it won't work if the circle is in the slope), but works in this case
					//because we know the circle is in a neighboring cell
					double dp = (ox*sx) + (oy*sy);
					double pen = obj->r - abs(dp);//note: we don't need the abs because we know the dp will be positive, but just in case..
					if(0 < pen)
					{
						//collision; circle out along normal by penetration amount
						obj->ReportCollisionVsWorld(sx*pen, sy*pen, sx, sy, t);
						
						return COL_OTHER;
					}
				}
			}
		}		
	}
	else if(oV == 0)
	{
		//colliding horizontally
		if((signx*oH) < 0)
		{
			//colliding with face/edge
			obj->ReportCollisionVsWorld(x*oH, 0, oH, 0, t);
			
			return COL_AXIS;
		}
		else
		{
				//we could only be colliding vs the slope OR a vertex
				//look at the vector form the closest vert to the circle to decide

				double sx = t->sx;
				double sy = t->sy;

				double ox = obj->pos.x - (t->pos.x + (oH*t->xw));//this gives is the coordinates of the innermost
				double oy = obj->pos.y - (t->pos.y - (signy*t->yw));//point on the circle, relative to the closest tile vert	

				//if the component of (ox,oy) parallel to the normal's righthand normal
				//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
				//then we project by the normal, otherwise by the vertex.
				//(NOTE: this is the opposite logic of the vertical case;
				// for vertical, if the perp prod and the slope's slope agree, it's outside.
				// for horizontal, if the perp prod and the slope's slope agree, circle is inside.
				//  ..but this is only a property of flahs' coord system (i.e the rules might swap
				// in righthanded systems))
				//note that this is simply a VERY tricky/weird method of determining 
				//if the circle is in side the slope/face's voronio region, or that of the vertex.											  
				double perp = (ox*-sy) + (oy*sx);
				if((perp*signx*signy) < 0)
				{
					//collide vs. vertex
					double len = sqrt(ox*ox + oy*oy);
					double pen = obj->r - len;
					if(0 < pen)
					{
						//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
						ox /= len;
						oy /= len;

						obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
						
						return COL_OTHER;
					}					
				}
				else
				{
					//collide vs. slope
					
					//if the component of (ox,oy) parallel to the normal is less than the circle radius, we're
					//penetrating the slope. note that this method of penetration calculation doesn't hold
					//in general (i.e it won't work if the circle is in the slope), but works in this case
					//because we know the circle is in a neighboring cell
					double dp = (ox*sx) + (oy*sy);
					double pen = obj->r - abs(dp);//note: we don't need the abs because we know the dp will be positive, but just in case..
					if(0 < pen)
					{
						//collision; circle out along normal by penetration amount
						obj->ReportCollisionVsWorld(sx*pen, sy*pen, sx, sy, t);
						
						return COL_OTHER;
					}
				}			
		}
	}
	else
	{
		//colliding diagonally
		if( 0 < ((signx*oH) + (signy*oV)) ) 
		{
			//the dotprod of slope normal and cell offset is strictly positive,
			//therefore obj is in the diagonal neighb pointed at by the normal, and
			//it cannot possibly reach/touch/penetrate the slope
			return COL_NONE;
		}
		else
		{
			//collide vs. vertex
			//get diag vertex position
			double vx = t->pos.x + (oH*t->xw);
			double vy = t->pos.y + (oV*t->yw);
			
			double dx = obj->pos.x - vx;//calc vert->circle vector		
			double dy = obj->pos.y - vy;
			
			double len = sqrt(dx*dx + dy*dy);
			double pen = obj->r - len;
			if(0 < pen)
			{
				//vertex is in the circle; project outward
				if(len == 0)
				{
					//project out by 45deg
					dx = oH / SQRT2;
					dy = oV / SQRT2;
				}
				else
				{
					dx /= len;
					dy /= len;
				}

				obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
				return COL_OTHER;
			}
	
		}
	
	}

	return COL_NONE;
}


int Circle::ProjCircle_Concave(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{

	//if we're colliding diagonally:
	//	-if obj is in the diagonal pointed to by the slope normal: we can't collide, do nothing
	//  -else, collide vs. the appropriate vertex
	//if obj is in this tile: perform collision as for aabb
	//if obj is horiz OR very neighb in direction of slope: collide vs vert
	//if obj is horiz or vert neigh against direction of slope: collide vs. face

	int signx = t->signx;
	int signy = t->signy;

	if(oH == 0)
	{
		if(oV == 0)
		{
			//colliding with current tile
			
				double ox = (t->pos.x + (signx*t->xw)) - obj->pos.x;//(ox,oy) is the vector from the circle to 
				double oy = (t->pos.y + (signy*t->yw)) - obj->pos.y;//tile-circle's center
				
				double lenP;
		
				int twid = t->xw*2;
				double trad = sqrt(twid*twid + 0);//this gives us the radius of a circle centered on the tile's corner and extending to the opposite edge of the tile;
												//note that this should be precomputed at compile-time since it's constant
				
				double len = sqrt(ox*ox + oy*oy);
				double pen = (len + obj->r) - trad;

				if(0 < pen)
				{
					//find the smallest axial projection vector
					if(x < y)
					{					
						//penetration in x is smaller
						lenP = x;
						y = 0;
						
						//get sign for projection along x-axis		
						if((obj->pos.x - t->pos.x) < 0)
						{
							x *= -1;
						}
					}
					else
					{		
						//penetration in y is smaller
						lenP = y;
						x = 0;
						
						//get sign for projection along y-axis		
						if((obj->pos.y - t->pos.y)< 0)
						{
							y *= -1;
						}			
					}

					
					if(lenP < pen)
					{
						obj->ReportCollisionVsWorld(x,y,x/lenP, y/lenP, t);
						
						return COL_AXIS;
					}
					else
					{
						//we can assume that len >0, because if we're here then
						//(len + obj->r) > trad, and since obj->r <= trad
						//len MUST be > 0
						ox /= len;
						oy /= len;

						obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
						
						return COL_OTHER;
					}
				}
				else
				{
					return COL_NONE;
				}

		}
		else
		{
			//colliding vertically
			if((signy*oV) < 0)
			{			
				//colliding with face/edge
				obj->ReportCollisionVsWorld(0,y*oV, 0, oV, t);
				
				return COL_AXIS;
			}
			else
			{
				//we could only be colliding vs the vertical tip

				//get diag vertex position
				double vx = t->pos.x - (signx*t->xw);
				double vy = t->pos.y + (oV*t->yw);
				
				double dx = obj->pos.x - vx;//calc vert->circle vector		
				double dy = obj->pos.y - vy;
				
				double len = sqrt(dx*dx + dy*dy);
				double pen = obj->r - len;
				if(0 < pen)
				{
					//vertex is in the circle; project outward
					if(len == 0)
					{
						//project out vertically
						dx = 0;
						dy = oV;
					}
					else
					{
						dx /= len;
						dy /= len;
					}

					obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
					
					return COL_OTHER;
				}
			}
		}		
	}
	else if(oV == 0)
	{
		//colliding horizontally
		if((signx*oH) < 0)
		{
			//colliding with face/edge
			obj->ReportCollisionVsWorld(x*oH, 0, oH, 0, t);
			
			return COL_AXIS;
		}
		else
		{
				//we could only be colliding vs the horizontal tip

				//get diag vertex position
				double vx = t->pos.x + (oH*t->xw);
				double vy = t->pos.y - (signy*t->yw);
				
				double dx = obj->pos.x - vx;//calc vert->circle vector		
				double dy = obj->pos.y - vy;
				
				double len = sqrt(dx*dx + dy*dy);
				double pen = obj->r - len;
				if(0 < pen)
				{
					//vertex is in the circle; project outward
					if(len == 0)
					{
						//project out horizontally
						dx = oH;
						dy = 0;
					}
					else
					{
						dx /= len;
						dy /= len;
					}

					obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
					
					return COL_OTHER;
				}	
		}
	}
	else
	{
		//colliding diagonally
		if( 0 < ((signx*oH) + (signy*oV)) ) 
		{
			//the dotprod of slope normal and cell offset is strictly positive,
			//therefore obj is in the diagonal neighb pointed at by the normal, and
			//it cannot possibly reach/touch/penetrate the slope
			return COL_NONE;
		}
		else
		{
			//collide vs. vertex
			//get diag vertex position
			double vx = t->pos.x + (oH*t->xw);
			double vy = t->pos.y + (oV*t->yw);
			
			double dx = obj->pos.x - vx;//calc vert->circle vector		
			double dy = obj->pos.y - vy;
			
			double len = sqrt(dx*dx + dy*dy);
			double pen = obj->r - len;
			if(0 < pen)
			{
				//vertex is in the circle; project outward
				if(len == 0)
				{
					//project out by 45deg
					dx = oH / SQRT2;
					dy = oV / SQRT2;
				}
				else
				{
					dx /= len;
					dy /= len;
				}

				obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
				
				return COL_OTHER;
			}
	
		}
	
	}

	return COL_NONE;
	
}


int Circle::ProjCircle_Convex(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{
	//if the object is horiz AND/OR vertical neighbor in the normal (signx,signy)
	//direction, collide vs. tile-circle only.
	//if we're colliding diagonally:
	//  -else, collide vs. the appropriate vertex
	//if obj is in this tile: perform collision as for aabb
	//if obj is horiz or vert neigh against direction of slope: collide vs. face

	int signx = t->signx;
	int signy = t->signy;

	if(oH == 0)
	{
		if(oV == 0)
		{
			//colliding with current tile
				
				
				double ox = obj->pos.x - (t->pos.x - (signx*t->xw));//(ox,oy) is the vector from the tile-circle to 
				double oy = obj->pos.y - (t->pos.y - (signy*t->yw));//the circle's center
				
				double lenP;
		
				int twid = t->xw*2;
				double trad = sqrt(twid*twid + 0);//this gives us the radius of a circle centered on the tile's corner and extending to the opposite edge of the tile;
												//note that this should be precomputed at compile-time since it's constant
				
				double len = sqrt(ox*ox + oy*oy);
				double pen = (trad + obj->r) - len;

				if(0 < pen)
				{
					//find the smallest axial projection vector
					if(x < y)
					{					
						//penetration in x is smaller
						lenP = x;
						y = 0;
						
						//get sign for projection along x-axis		
						if((obj->pos.x - t->pos.x) < 0)
						{
							x *= -1;
						}
					}
					else
					{		
						//penetration in y is smaller
						lenP = y;
						x = 0;
						
						//get sign for projection along y-axis		
						if((obj->pos.y - t->pos.y)< 0)
						{
							y *= -1;
						}			
					}

					
					if(lenP < pen)
					{
						obj->ReportCollisionVsWorld(x, y, x/lenP, y/lenP, t);
						
						return COL_AXIS;
					}
					else
					{
						//note: len should NEVER be == 0, because if it is, 
						//projeciton by an axis shoudl always be shorter, and we should
						//never arrive here
						ox /= len;
						oy /= len;
						
						obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
						
						return COL_OTHER;
						
					}
				}
		}
		else
		{
			//colliding vertically
			if((signy*oV) < 0)
			{
				//colliding with face/edge
				obj->ReportCollisionVsWorld(0, y*oV, 0, oV, t);
				
				return COL_AXIS;
			}
			else
			{
				//obj in neighboring cell pointed at by tile normal;
				//we could only be colliding vs the tile-circle surface

				double ox = obj->pos.x - (t->pos.x - (signx*t->xw));//(ox,oy) is the vector from the tile-circle to 
				double oy = obj->pos.y - (t->pos.y - (signy*t->yw));//the circle's center
		
				int twid = t->xw*2;
				double trad = sqrt(twid*twid + 0);//this gives us the radius of a circle centered on the tile's corner and extending to the opposite edge of the tile;
												//note that this should be precomputed at compile-time since it's constant
				
				double len = sqrt(ox*ox + oy*oy);
				double pen = (trad + obj->r) - len;

				if(0 < pen)
				{

					//note: len should NEVER be == 0, because if it is, 
					//obj is not in a neighboring cell!
					ox /= len;
					oy /= len;
						
					obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
					
					return COL_OTHER;
				}
			}
		}		
	}
	else if(oV == 0)
	{
		//colliding horizontally
		if((signx*oH) < 0)
		{
			//colliding with face/edge
			obj->ReportCollisionVsWorld(x*oH, 0, oH, 0, t);
			
			return COL_AXIS;
		}
		else
		{
				//obj in neighboring cell pointed at by tile normal;
				//we could only be colliding vs the tile-circle surface

				double ox = obj->pos.x - (t->pos.x - (signx*t->xw));//(ox,oy) is the vector from the tile-circle to 
				double oy = obj->pos.y - (t->pos.y - (signy*t->yw));//the circle's center
		
				double twid = t->xw*2;
				double trad = sqrt(twid*twid + 0);//this gives us the radius of a circle centered on the tile's corner and extending to the opposite edge of the tile;
												//note that this should be precomputed at compile-time since it's constant
				
				double len = sqrt(ox*ox + oy*oy);
				double pen = (trad + obj->r) - len;
		
				if(0 < pen)
				{

					//note: len should NEVER be == 0, because if it is, 
					//obj is not in a neighboring cell!
					ox /= len;
					oy /= len;

					obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
					
					return COL_OTHER;
				}	
		}
	}
	else
	{
		//colliding diagonally
		if( 0 < ((signx*oH) + (signy*oV)) ) 
		{
				//obj in diag neighb cell pointed at by tile normal;
				//we could only be colliding vs the tile-circle surface

				double ox = obj->pos.x - (t->pos.x - (signx*t->xw));//(ox,oy) is the vector from the tile-circle to 
				double oy = obj->pos.y - (t->pos.y - (signy*t->yw));//the circle's center
		
				double twid = t->xw*2;
				double trad = sqrt(twid*twid + 0);//this gives us the radius of a circle centered on the tile's corner and extending to the opposite edge of the tile;
												//note that this should be precomputed at compile-time since it's constant
				
				double len = sqrt(ox*ox + oy*oy);
				double pen = (trad + obj->r) - len;
				
				if(0 < pen)
				{

					//note: len should NEVER be == 0, because if it is, 
					//obj is not in a neighboring cell!
					ox /= len;
					oy /= len;

					obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
					
					return COL_OTHER;
				}
		}
		else
		{
			//collide vs. vertex
			//get diag vertex position
			double vx = t->pos.x + (oH*t->xw);
			double vy = t->pos.y + (oV*t->yw);
			
			double dx = obj->pos.x - vx;//calc vert->circle vector		
			double dy = obj->pos.y - vy;
			
			double len = sqrt(dx*dx + dy*dy);
			double pen = obj->r - len;
			if(0 < pen)
			{
				//vertex is in the circle; project outward
				if(len == 0)
				{
					//project out by 45deg
					dx = oH / SQRT2;
					dy = oV / SQRT2;
				}
				else
				{
					dx /= len;
					dy /= len;
				}

				obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
				
				return COL_OTHER;
			}
	
		}
	
	}

	return COL_NONE;
	
}


int Circle::ProjCircle_22DegS(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{
	
	//if the object is in a cell pointed at by signy, no collision will ever occur
	//otherwise,
	//
	//if we're colliding diagonally:
	//  -collide vs. the appropriate vertex
	//if obj is in this tile: collide vs slope or vertex
	//if obj is horiz neighb in direction of slope: collide vs. slope or vertex
	//if obj is horiz neighb against the slope:
	//   if(distance in y from circle to 90deg corner of tile < 1/2 tileheight, collide vs. face)
	//   else(collide vs. corner of slope) (vert collision with a non-grid-aligned vert)
	//if obj is vert neighb against direction of slope: collide vs. face

	int signx = t->signx;
	int signy = t->signy;

	if(0 < (signy*oV))
	{
		//object will never collide vs tile, it can't reach that far
		
		return COL_NONE;
	}
	else if(oH == 0)
	{
		if(oV == 0)
		{
			//colliding with current tile
			//we could only be colliding vs the slope OR a vertex
			//look at the vector form the closest vert to the circle to decide
	
			double sx = t->sx;
			double sy = t->sy;
			
			int r = obj->r;
			double ox = obj->pos.x - (t->pos.x - (signx*t->xw));//this gives is the coordinates of the innermost
			double oy = obj->pos.y - t->pos.y;//point on the circle, relative to the tile corner	
		
			//if the component of (ox,oy) parallel to the normal's righthand normal
			//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
			//then we project by the vertex, otherwise by the normal or axially.
			//note that this is simply a VERY tricky/weird method of determining 
			//if the circle is in side the slope/face's voronio region, or that of the vertex.
				
			double perp = (ox*-sy) + (oy*sx);
			if(0 < (perp*signx*signy))
			{
				//collide vs. vertex
				double len = sqrt(ox*ox + oy*oy);
				double pen = r - len;
				if(0 < pen)
				{
					//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
					ox /= len;
					oy /= len;

					obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
					
					return COL_OTHER;
				}					
			}
			else
			{
				//collide vs. slope or vs axis
				ox -= r*sx;//this gives us the vector from  
				oy -= r*sy;//a point on the slope to the innermost point on the circle
		
				//if the dotprod of (ox,oy) and (sx,sy) is negative, the point on the circle is in the slope
				//and we need toproject it out by the magnitude of the projection of (ox,oy) onto (sx,sy)
				double dp = (ox*sx) + (oy*sy);
				
				double lenP;
				
				if(dp < 0)
				{
					//collision; project delta onto slope and use this to displace the object
					sx *= -dp;//(sx,sy) is now the projection vector
					sy *= -dp;		
						
					double lenN = sqrt(sx*sx + sy*sy);
			
					//find the smallest axial projection vector
					if(x < y)
					{					
						//penetration in x is smaller
						lenP = x;
						y = 0;	
						//get sign for projection along x-axis		
						if((obj->pos.x - t->pos.x) < 0)
						{
							x *= -1;
						}
					}
					else
					{		
						//penetration in y is smaller
						lenP = y;
						x = 0;	
						//get sign for projection along y-axis		
						if((obj->pos.y - t->pos.y)< 0)
						{
							y *= -1;
						}			
					}

					if(lenP < lenN)
					{
						obj->ReportCollisionVsWorld(x,y,x/lenP, y/lenP, t);

						return COL_AXIS;
					}
					else
					{				
						obj->ReportCollisionVsWorld(sx,sy,t->sx,t->sy,t);

						return COL_OTHER;
					}
			
				}
			}
			
		}
		else
		{
			//colliding vertically; we can assume that (signy*oV) < 0
			//due to the first conditional far above

			obj->ReportCollisionVsWorld(0,y*oV, 0, oV, t);
				
			return COL_AXIS;
		}		
	}
	else if(oV == 0)
	{
		//colliding horizontally
		if((signx*oH) < 0)
		{
			//colliding with face/edge OR with corner of wedge, depending on our position vertically
				
			//collide vs. vertex
			//get diag vertex position
			double vx = t->pos.x - (signx*t->xw);
			double vy = t->pos.y;
					
			double dx = obj->pos.x - vx;//calc vert->circle vector		
			double dy = obj->pos.y - vy;
					
			if((dy*signy) < 0)
			{
				//colliding vs face
				obj->ReportCollisionVsWorld(x*oH, 0, oH, 0, t);
				
				return COL_AXIS;					
			}
			else
			{
				//colliding vs. vertex
					
				double len = sqrt(dx*dx + dy*dy);
				double pen = obj->r - len;
				if(0 < pen)
				{
					//vertex is in the circle; project outward
					if(len == 0)
					{
						//project out by 45deg
						dx = oH / SQRT2;
						dy = oV / SQRT2;
					}
					else
					{
						dx /= len;
						dy /= len;
					}

					obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
					
					return COL_OTHER;
				}
			}
		}
		else
		{
			//we could only be colliding vs the slope OR a vertex
			//look at the vector form the closest vert to the circle to decide
	
			double sx = t->sx;
			double sy = t->sy;
				
			double ox = obj->pos.x - (t->pos.x + (oH*t->xw));//this gives is the coordinates of the innermost
			double oy = obj->pos.y - (t->pos.y - (signy*t->yw));//point on the circle, relative to the closest tile vert	
	
			//if the component of (ox,oy) parallel to the normal's righthand normal
			//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
			//then we project by the normal, otherwise by the vertex.
			//(NOTE: this is the opposite logic of the vertical case;
			// for vertical, if the perp prod and the slope's slope agree, it's outside.
			// for horizontal, if the perp prod and the slope's slope agree, circle is inside.
			//  ..but this is only a property of flahs' coord system (i.e the rules might swap
			// in righthanded systems))
			//note that this is simply a VERY tricky/weird method of determining 
			//if the circle is in side the slope/face's voronio region, or that of the vertex.											  
			double perp = (ox*-sy) + (oy*sx);
			if((perp*signx*signy) < 0)
			{
				//collide vs. vertex
				double len = sqrt(ox*ox + oy*oy);
				double pen = obj->r - len;
				if(0 < pen)
				{
					//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
					ox /= len;
					oy /= len;

					obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
					
					return COL_OTHER;
				}					
			}
			else
			{
				//collide vs. slope
						
				//if the component of (ox,oy) parallel to the normal is less than the circle radius, we're
				//penetrating the slope. note that this method of penetration calculation doesn't hold
				//in general (i.e it won't work if the circle is in the slope), but works in this case
				//because we know the circle is in a neighboring cell
				double dp = (ox*sx) + (oy*sy);
				double pen = obj->r - abs(dp);//note: we don't need the abs because we know the dp will be positive, but just in case..				

				if(0 < pen)
				{
					//collision; circle out along normal by penetration amount
					obj->ReportCollisionVsWorld(sx*pen, sy*pen, sx, sy, t);
					
					return COL_OTHER;
				}
			}
		}
	}
	else
	{

		//colliding diagonally; due to the first conditional above,
		//obj is vertically offset against slope, and offset in either direction horizontally

		//collide vs. vertex
		//get diag vertex position
		double vx = t->pos.x + (oH*t->xw);
		double vy = t->pos.y + (oV*t->yw);
			
		double dx = obj->pos.x - vx;//calc vert->circle vector		
		double dy = obj->pos.y - vy;
			
		double len = sqrt(dx*dx + dy*dy);
		double pen = obj->r - len;
		if(0 < pen)
		{
			//vertex is in the circle; project outward
			if(len == 0)
			{
				//project out by 45deg
				dx = oH / SQRT2;
				dy = oV / SQRT2;
			}
			else
			{
				dx /= len;
				dy /= len;
			}

			obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
			
			return COL_OTHER;
		}
	}

	return COL_NONE;

}


int Circle::ProjCircle_22DegB(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{

	//if we're colliding diagonally:
	//  -if we're in the cell pointed at by the normal, collide vs slope, else
	//  collide vs. the appropriate corner/vertex
	//
	//if obj is in this tile: collide as with aabb
	//
	//if obj is horiz or vertical neighbor AGAINST the slope: collide with edge
	//
	//if obj is horiz neighb in direction of slope: collide vs. slope or vertex or edge
	//
	//if obj is vert neighb in direction of slope: collide vs. slope or vertex

	int signx = t->signx;
	int signy = t->signy;

	if(oH == 0)
	{
		if(oV == 0)
		{
			//colliding with current cell

			double sx = t->sx;
			double sy = t->sy;
			
			double lenP;
	
			int r = obj->r;
			double ox = (obj->pos.x - (sx*r)) - (t->pos.x - (signx*t->xw));//this gives is the coordinates of the innermost
			double oy = (obj->pos.y - (sy*r)) - (t->pos.y + (signy*t->yw));//point on the AABB, relative to a point on the slope
		
			//if the dotprod of (ox,oy) and (sx,sy) is negative, the point on the circle is in the slope
			//and we need toproject it out by the magnitude of the projection of (ox,oy) onto (sx,sy)
			double dp = (ox*sx) + (oy*sy);
					
			if(dp < 0)
			{
				//collision; project delta onto slope and use this to displace the object
				sx *= -dp;//(sx,sy) is now the projection vector
				sy *= -dp;		
							
				double lenN = sqrt(sx*sx + sy*sy);
				
				//find the smallest axial projection vector
				if(x < y)
				{					
					//penetration in x is smaller
					lenP = x;
					y = 0;	
					//get sign for projection along x-axis		
					if((obj->pos.x - t->pos.x) < 0)
					{
						x *= -1;
					}
				}
				else
				{		
					//penetration in y is smaller
					lenP = y;
					x = 0;	
					//get sign for projection along y-axis		
					if((obj->pos.y - t->pos.y)< 0)
					{
						y *= -1;
					}			
				}
	
				if(lenP < lenN)
				{
					obj->ReportCollisionVsWorld(x, y, x/lenP, y/lenP, t);
					
					return COL_AXIS;
				}
				else
				{			
					obj->ReportCollisionVsWorld(sx, sy, t->sx, t->sy, t);
			
					return COL_OTHER;
				}	
			}					
		}
		else
		{
			//colliding vertically
			
			if((signy*oV) < 0)
			{
				//colliding with face/edge
				obj->ReportCollisionVsWorld(0, y*oV, 0, oV, t);
				
				return COL_AXIS;
			}
			else
			{
				//we could only be colliding vs the slope OR a vertex
				//look at the vector form the closest vert to the circle to decide

				double sx = t->sx;
				double sy = t->sy;
				
				double ox = obj->pos.x - (t->pos.x - (signx*t->xw));//this gives is the coordinates of the innermost
				double oy = obj->pos.y - (t->pos.y + (signy*t->yw));//point on the circle, relative to the closest tile vert	

				//if the component of (ox,oy) parallel to the normal's righthand normal
				//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
				//then we project by the vertex, otherwise by the normal.
				//note that this is simply a VERY tricky/weird method of determining 
				//if the circle is in side the slope/face's voronio region, or that of the vertex.											  
				double perp = (ox*-sy) + (oy*sx);
				if(0 < (perp*signx*signy))
				{
					//collide vs. vertex
					double len = sqrt(ox*ox + oy*oy);
					double pen = obj->r - len;
					if(0 < pen)
					{
						//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
						ox /= len;
						oy /= len;

						obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
						
						return COL_OTHER;
					}					
				}
				else
				{
					//collide vs. slope
					
					//if the component of (ox,oy) parallel to the normal is less than the circle radius, we're
					//penetrating the slope. note that this method of penetration calculation doesn't hold
					//in general (i.e it won't work if the circle is in the slope), but works in this case
					//because we know the circle is in a neighboring cell
					double dp = (ox*sx) + (oy*sy);
					double pen = obj->r - abs(dp);//note: we don't need the abs because we know the dp will be positive, but just in case..
					if(0 < pen)
					{
						//collision; circle out along normal by penetration amount
						obj->ReportCollisionVsWorld(sx*pen, sy*pen,sx, sy, t);
						
						return COL_OTHER;
					}
				}
			}
		}
	}
	else if(oV == 0)
	{
		//colliding horizontally
		
		if((signx*oH) < 0)
		{
			//colliding with face/edge
			obj->ReportCollisionVsWorld(x*oH, 0, oH, 0, t);
			
			return COL_AXIS;
		}
		else
		{
			//colliding with edge, slope, or vertex
		
			double ox = obj->pos.x - (t->pos.x + (signx*t->xw));//this gives is the coordinates of the innermost
			double oy = obj->pos.y - t->pos.y;//point on the circle, relative to the closest tile vert	
				
			if((oy*signy) < 0)
			{
				//we're colliding with the halfface
				obj->ReportCollisionVsWorld(x*oH, 0, oH, 0, t);
				
				return COL_AXIS;			
			}
			else
			{
				//colliding with the vertex or slope

				double sx = t->sx;
				double sy = t->sy;
								
				//if the component of (ox,oy) parallel to the normal's righthand normal
				//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
				//then we project by the slope, otherwise by the vertex.
				//note that this is simply a VERY tricky/weird method of determining 
				//if the circle is in side the slope/face's voronio region, or that of the vertex.											  
				double perp = (ox*-sy) + (oy*sx);
				if((perp*signx*signy) < 0)
				{
					//collide vs. vertex
					double len = sqrt(ox*ox + oy*oy);
					double pen = obj->r - len;
					if(0 < pen)
					{
						//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
						ox /= len;
						oy /= len;
	
						obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
						
						return COL_OTHER;
					}					
				}
				else
				{
					//collide vs. slope
						
					//if the component of (ox,oy) parallel to the normal is less than the circle radius, we're
					//penetrating the slope. note that this method of penetration calculation doesn't hold
					//in general (i.e it won't work if the circle is in the slope), but works in this case
					//because we know the circle is in a neighboring cell
					double dp = (ox*sx) + (oy*sy);
					double pen = obj->r - abs(dp);//note: we don't need the abs because we know the dp will be positive, but just in case..
					if(0 < pen)
					{
						//collision; circle out along normal by penetration amount
						obj->ReportCollisionVsWorld(sx*pen, sy*pen, t->sx, t->sy, t);
						
						return COL_OTHER;
					}
				}	
			}
		}
	}
	else
	{
		//colliding diagonally
		if( 0 < ((signx*oH) + (signy*oV)) ) 
		{
			//the dotprod of slope normal and cell offset is strictly positive,
			//therefore obj is in the diagonal neighb pointed at by the normal.
			
			//collide vs slope

			//we should really precalc this at compile time, but for now, fuck it
			double slen = sqrt(2*2 + 1*1);//the raw slope is (-2,-1)
			double sx = (signx*1) / slen;//get slope _unit_ normal;
			double sy = (signy*2) / slen;//raw RH normal is (1,-2)
	
			int r = obj->r;
			double ox = (obj->pos.x - (sx*r)) - (t->pos.x - (signx*t->xw));//this gives is the coordinates of the innermost
			double oy = (obj->pos.y - (sy*r)) - (t->pos.y + (signy*t->yw));//point on the circle, relative to a point on the slope
		
			//if the dotprod of (ox,oy) and (sx,sy) is negative, the point on the circle is in the slope
			//and we need toproject it out by the magnitude of the projection of (ox,oy) onto (sx,sy)
			double dp = (ox*sx) + (oy*sy);
					
			if(dp < 0)
			{
				//collision; project delta onto slope and use this to displace the object	
				//(sx,sy)*-dp is the projection vector
				obj->ReportCollisionVsWorld(-sx*dp, -sy*dp, t->sx, t->sy, t);
				
				return COL_OTHER;
			}
			return COL_NONE;
		}
		else
		{
			//collide vs the appropriate vertex
			double vx = t->pos.x + (oH*t->xw);
			double vy = t->pos.y + (oV*t->yw);
			
			double dx = obj->pos.x - vx;//calc vert->circle vector		
			double dy = obj->pos.y - vy;
			
			double len = sqrt(dx*dx + dy*dy);
			double pen = obj->r - len;
			if(0 < pen)
			{
				//vertex is in the circle; project outward
				if(len == 0)
				{
					//project out by 45deg
					dx = oH / SQRT2;
					dy = oV / SQRT2;
				}
				else
				{
					dx /= len;
					dy /= len;
				}

				obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);

				return COL_OTHER;
			}
				
		}		
	}
	
	return COL_NONE;
}


int Circle::ProjCircle_67DegS(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{
	//if the object is in a cell pointed at by signx, no collision will ever occur
	//otherwise,
	//
	//if we're colliding diagonally:
	//  -collide vs. the appropriate vertex
	//if obj is in this tile: collide vs slope or vertex or axis
	//if obj is vert neighb in direction of slope: collide vs. slope or vertex
	//if obj is vert neighb against the slope:
	//   if(distance in y from circle to 90deg corner of tile < 1/2 tileheight, collide vs. face)
	//   else(collide vs. corner of slope) (vert collision with a non-grid-aligned vert)
	//if obj is horiz neighb against direction of slope: collide vs. face

	int signx = t->signx;
	int signy = t->signy;

	if(0 < (signx*oH))
	{
		//object will never collide vs tile, it can't reach that far

		return COL_NONE;
	}
	else if(oH == 0)
	{
		if(oV == 0)
		{
			//colliding with current tile
			//we could only be colliding vs the slope OR a vertex
			//look at the vector form the closest vert to the circle to decide
	
			double sx = t->sx;
			double sy = t->sy;
			
			int r = obj->r;
			double ox = obj->pos.x - t->pos.x;//this gives is the coordinates of the innermost
			double oy = obj->pos.y - (t->pos.y - (signy*t->yw));//point on the circle, relative to the tile corner	
		
			//if the component of (ox,oy) parallel to the normal's righthand normal
			//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
			//then we project by the normal or axis, otherwise by the corner/vertex
			//note that this is simply a VERY tricky/weird method of determining 
			//if the circle is in side the slope/face's voronoi region, or that of the vertex.
				
			double perp = (ox*-sy) + (oy*sx);
			if((perp*signx*signy) < 0)
			{
				//collide vs. vertex
				double len = sqrt(ox*ox + oy*oy);
				double pen = r - len;
				if(0 < pen)
				{
					//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
					ox /= len;
					oy /= len;

					obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
					return COL_OTHER;
				}					
			}
			else
			{
				//collide vs. slope or vs axis
				ox -= r*sx;//this gives us the vector from  
				oy -= r*sy;//a point on the slope to the innermost point on the circle
				
				double lenP;
		
				//if the dotprod of (ox,oy) and (sx,sy) is negative, the point on the circle is in the slope
				//and we need toproject it out by the magnitude of the projection of (ox,oy) onto (sx,sy)
				double dp = (ox*sx) + (oy*sy);
				
				if(dp < 0)
				{
					//collision; project delta onto slope and use this to displace the object
					sx *= -dp;//(sx,sy) is now the projection vector
					sy *= -dp;		
						
					double lenN = sqrt(sx*sx + sy*sy);
			
					//find the smallest axial projection vector
					if(x < y)
					{					
						//penetration in x is smaller
						lenP = x;
						y = 0;	
						//get sign for projection along x-axis		
						if((obj->pos.x - t->pos.x) < 0)
						{
							x *= -1;
						}
					}
					else
					{		
						//penetration in y is smaller
						lenP = y;
						x = 0;	
						//get sign for projection along y-axis		
						if((obj->pos.y - t->pos.y)< 0)
						{
							y *= -1;
						}			
					}

					if(lenP < lenN)
					{
						obj->ReportCollisionVsWorld(x,y,x/lenP, y/lenP, t);
						
						return COL_AXIS;
					}
					else
					{		
						obj->ReportCollisionVsWorld(sx,sy,t->sx,t->sy,t);
						
						return COL_OTHER;
					}	
				}
			}
			
		}
		else
		{
			//colliding vertically
			
			if((signy*oV) < 0)
			{
				//colliding with face/edge OR with corner of wedge, depending on our position vertically
					
				//collide vs. vertex
				//get diag vertex position
				double vx = t->pos.x;
				double vy = t->pos.y - (signy*t->yw);
						
				double dx = obj->pos.x - vx;//calc vert->circle vector		
				double dy = obj->pos.y - vy;
						
				if((dx*signx) < 0)
				{	
					//colliding vs face
					obj->ReportCollisionVsWorld(0, y*oV, 0, oV, t);
					
					return COL_AXIS;					
				}
				else
				{
					//colliding vs. vertex
						
					double len = sqrt(dx*dx + dy*dy);
					double pen = obj->r - len;
					if(0 < pen)
					{
						//vertex is in the circle; project outward
						if(len == 0)
						{
							//project out by 45deg
							dx = oH / SQRT2;
							dy = oV / SQRT2;
						}
						else
						{
							dx /= len;
							dy /= len;
						}
							
						obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
						
						return COL_OTHER;
					}
				}
			}
			else
			{
				//we could only be colliding vs the slope OR a vertex
				//look at the vector form the closest vert to the circle to decide
		
				double sx = t->sx;
				double sy = t->sy;
					
				double ox = obj->pos.x - (t->pos.x - (signx*t->xw));//this gives is the coordinates of the innermost
				double oy = obj->pos.y - (t->pos.y + (oV*t->yw));//point on the circle, relative to the closest tile vert	
		
				//if the component of (ox,oy) parallel to the normal's righthand normal
				//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
				//then we project by the vertex, otherwise by the normal.
				//note that this is simply a VERY tricky/weird method of determining 
				//if the circle is in side the slope/face's voronio region, or that of the vertex.											  
				double perp = (ox*-sy) + (oy*sx);
				if(0 < (perp*signx*signy))
				{
					//collide vs. vertex
					double len = sqrt(ox*ox + oy*oy);
					double pen = obj->r - len;
					if(0 < pen)
					{
						//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
						ox /= len;
						oy /= len;

						obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
						
						return COL_OTHER;
					}					
				}
				else
				{
					//collide vs. slope
							
					//if the component of (ox,oy) parallel to the normal is less than the circle radius, we're
					//penetrating the slope. note that this method of penetration calculation doesn't hold
					//in general (i.e it won't work if the circle is in the slope), but works in this case
					//because we know the circle is in a neighboring cell
					double dp = (ox*sx) + (oy*sy);
					double pen = obj->r - abs(dp);//note: we don't need the abs because we know the dp will be positive, but just in case..				

					if(0 < pen)
					{
						//collision; circle out along normal by penetration amount
						obj->ReportCollisionVsWorld(sx*pen, sy*pen, t->sx, t->sy, t);
						
						return COL_OTHER;
					}
				}
			}
		}		
	}
	else if(oV == 0)
	{
		//colliding horizontally; we can assume that (signy*oV) < 0
		//due to the first conditional far above

			obj->ReportCollisionVsWorld(x*oH, 0, oH, 0, t);
			
			return COL_AXIS;
	}
	else
	{		
		//colliding diagonally; due to the first conditional above,
		//obj is vertically offset against slope, and offset in either direction horizontally

		//collide vs. vertex
		//get diag vertex position
		double vx = t->pos.x + (oH*t->xw);
		double vy = t->pos.y + (oV*t->yw);
			
		double dx = obj->pos.x - vx;//calc vert->circle vector		
		double dy = obj->pos.y - vy;
			
		double len = sqrt(dx*dx + dy*dy);
		double pen = obj->r - len;
		if(0 < pen)
		{
			//vertex is in the circle; project outward
			if(len == 0)
			{
				//project out by 45deg
				dx = oH / SQRT2;
				dy = oV / SQRT2;
			}
			else
			{
				dx /= len;
				dy /= len;
			}

			obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);
			
			return COL_OTHER;
		}
	}

	return COL_NONE;

}


int Circle::ProjCircle_67DegB(double x, double y, const int &oH, const int &oV, Circle *obj, TileMapCell *t)
{
	//if we're colliding diagonally:
	//  -if we're in the cell pointed at by the normal, collide vs slope, else
	//  collide vs. the appropriate corner/vertex
	//
	//if obj is in this tile: collide as with aabb
	//
	//if obj is horiz or vertical neighbor AGAINST the slope: collide with edge
	//
	//if obj is vert neighb in direction of slope: collide vs. slope or vertex or halfedge
	//
	//if obj is horiz neighb in direction of slope: collide vs. slope or vertex

	int signx = t->signx;
	int signy = t->signy;

	if(oH == 0)
	{
		if(oV == 0)
		{
			//colliding with current cell

			double sx = t->sx;
			double sy = t->sy;
			
			double lenP;
	
			int r = obj->r;
			double ox = (obj->pos.x - (sx*r)) - (t->pos.x + (signx*t->xw));//this gives is the coordinates of the innermost
			double oy = (obj->pos.y - (sy*r)) - (t->pos.y - (signy*t->yw));//point on the AABB, relative to a point on the slope
		
			//if the dotprod of (ox,oy) and (sx,sy) is negative, the point on the circle is in the slope
			//and we need toproject it out by the magnitude of the projection of (ox,oy) onto (sx,sy)
			double dp = (ox*sx) + (oy*sy);
					
			if(dp < 0)
			{
				//collision; project delta onto slope and use this to displace the object
				sx *= -dp;//(sx,sy) is now the projection vector
				sy *= -dp;		
							
				double lenN = sqrt(sx*sx + sy*sy);
				
				//find the smallest axial projection vector
				if(x < y)
				{					
					//penetration in x is smaller
					lenP = x;
					y = 0;	
					//get sign for projection along x-axis		
					if((obj->pos.x - t->pos.x) < 0)
					{
						x *= -1;
					}
				}
				else
				{		
					//penetration in y is smaller
					lenP = y;
					x = 0;	
					//get sign for projection along y-axis		
					if((obj->pos.y - t->pos.y)< 0)
					{
						y *= -1;
					}			
				}
	
				if(lenP < lenN)
				{
					obj->ReportCollisionVsWorld(x,y,x/lenP, y/lenP, t);
					
					return COL_AXIS;
				}
				else
				{
					obj->ReportCollisionVsWorld(sx, sy, t->sx, t->sy, t);
					
					return COL_OTHER;
				}
	
			}					
		}
		else
		{
			//colliding vertically
		
			if((signy*oV) < 0)
			{
				//colliding with face/edge
				obj->ReportCollisionVsWorld(0, y*oV, 0, oV, t);

				return COL_AXIS;
			}
			else
			{
				//colliding with edge, slope, or vertex
			
				double ox = obj->pos.x - t->pos.x;//this gives is the coordinates of the innermost
				double oy = obj->pos.y - (t->pos.y + (signy*t->yw));//point on the circle, relative to the closest tile vert	
					
				if((ox*signx) < 0)
				{
					//we're colliding with the halfface
					obj->ReportCollisionVsWorld(0, y*oV, 0, oV, t);

					return COL_AXIS;			
				}
				else
				{
					//colliding with the vertex or slope

					double sx = t->sx;
					double sy = t->sy;
									
					//if the component of (ox,oy) parallel to the normal's righthand normal
					//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
					//then we project by the vertex, otherwise by the slope.
					//note that this is simply a VERY tricky/weird method of determining 
					//if the circle is in side the slope/face's voronio region, or that of the vertex.											  
					double perp = (ox*-sy) + (oy*sx);
					if(0 < (perp*signx*signy))
					{
						//collide vs. vertex
						double len = sqrt(ox*ox + oy*oy);
						double pen = obj->r - len;
						if(0 < pen)
						{
							//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
							ox /= len;
							oy /= len;
		
							obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
							
							return COL_OTHER;
						}					
					}
					else
					{
						//collide vs. slope
							
						//if the component of (ox,oy) parallel to the normal is less than the circle radius, we're
						//penetrating the slope. note that this method of penetration calculation doesn't hold
						//in general (i.e it won't work if the circle is in the slope), but works in this case
						//because we know the circle is in a neighboring cell
						double dp = (ox*sx) + (oy*sy);
						double pen = obj->r - abs(dp);//note: we don't need the abs because we know the dp will be positive, but just in case..
						if(0 < pen)
						{
							//collision; circle out along normal by penetration amount
							obj->ReportCollisionVsWorld(sx*pen, sy*pen, sx, sy, t);
							
							return COL_OTHER;
						}
					}	
				}
			}
		}
	}
	else if(oV == 0)
	{
		//colliding horizontally
			
		if((signx*oH) < 0)
		{
			//colliding with face/edge
			obj->ReportCollisionVsWorld(x*oH, 0, oH, 0, t);
			
			return COL_AXIS;
		}
		else
		{
			//we could only be colliding vs the slope OR a vertex
			//look at the vector form the closest vert to the circle to decide

			double slen = sqrt(2*2 + 1*1);//the raw slope is (-2,-1)
			double sx = (signx*2) / slen;//get slope _unit_ normal;
			double sy = (signy*1) / slen;//raw RH normal is (1,-2)
				
			double ox = obj->pos.x - (t->pos.x + (signx*t->xw));//this gives is the coordinates of the innermost
			double oy = obj->pos.y - (t->pos.y - (signy*t->yw));//point on the circle, relative to the closest tile vert	

			//if the component of (ox,oy) parallel to the normal's righthand normal
			//has the same sign as the slope of the slope (the sign of the slope's slope is signx*signy)
			//then we project by the slope, otherwise by the vertex.
			//note that this is simply a VERY tricky/weird method of determining 
			//if the circle is in side the slope/face's voronio region, or that of the vertex.											  
			double perp = (ox*-sy) + (oy*sx);
			if((perp*signx*signy) < 0)
			{
				//collide vs. vertex
				double len = sqrt(ox*ox + oy*oy);
				double pen = obj->r - len;
				if(0 < pen)
				{
					//note: if len=0, then perp=0 and we'll never reach here, so don't worry about div-by-0
					ox /= len;
					oy /= len;

					obj->ReportCollisionVsWorld(ox*pen, oy*pen, ox, oy, t);
					
					return COL_OTHER;
				}					
			}
			else
			{
				//collide vs. slope
					
				//if the component of (ox,oy) parallel to the normal is less than the circle radius, we're
				//penetrating the slope. note that this method of penetration calculation doesn't hold
				//in general (i.e it won't work if the circle is in the slope), but works in this case
				//because we know the circle is in a neighboring cell
				double dp = (ox*sx) + (oy*sy);
				double pen = obj->r - abs(dp);//note: we don't need the abs because we know the dp will be positive, but just in case..
				if(0 < pen)
				{
					//collision; circle out along normal by penetration amount
					obj->ReportCollisionVsWorld(sx*pen, sy*pen, t->sx, t->sy, t);
					
					return COL_OTHER;
				}
			}
		}
	}
	else
	{
		//colliding diagonally
		if( 0 < ((signx*oH) + (signy*oV)) ) 
		{
			//the dotprod of slope normal and cell offset is strictly positive,
			//therefore obj is in the diagonal neighb pointed at by the normal.
			
			//collide vs slope

			double sx = t->sx;
			double sy = t->sy;
	
			int r = obj->r;
			double ox = (obj->pos.x - (sx*r)) - (t->pos.x + (signx*t->xw));//this gives is the coordinates of the innermost
			double oy = (obj->pos.y - (sy*r)) - (t->pos.y - (signy*t->yw));//point on the circle, relative to a point on the slope
		
			//if the dotprod of (ox,oy) and (sx,sy) is negative, the point on the circle is in the slope
			//and we need toproject it out by the magnitude of the projection of (ox,oy) onto (sx,sy)
			double dp = (ox*sx) + (oy*sy);
					
			if(dp < 0)
			{
				//collision; project delta onto slope and use this to displace the object	
				//(sx,sy)*-dp is the projection vector

				obj->ReportCollisionVsWorld(-sx*dp, -sy*dp, t->sx, t->sy, t);

				return COL_OTHER;
			}
			return COL_NONE;
		}
		else
		{
			
			//collide vs the appropriate vertex
			double vx = t->pos.x + (oH*t->xw);
			double vy = t->pos.y + (oV*t->yw);
			
			double dx = obj->pos.x - vx;//calc vert->circle vector		
			double dy = obj->pos.y - vy;
			
			double len = sqrt(dx*dx + dy*dy);
			double pen = obj->r - len;
			if(0 < pen)
			{
				//vertex is in the circle; project outward
				if(len == 0)
				{
					//project out by 45deg
					dx = oH / SQRT2;
					dy = oV / SQRT2;
				}
				else
				{
					dx /= len;
					dy /= len;
				}

				obj->ReportCollisionVsWorld(dx*pen, dy*pen, dx, dy, t);

				return COL_OTHER;
			}
				
		}		
	}
	
	return COL_NONE;
}
