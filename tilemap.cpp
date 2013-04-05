//* TileMap.cpp *//

/*
this object manages a grid of static AABB tiles.
*/

//NOTE: the tilemap is now transparently padded; it is 2 rows and columns larger than
//any other module knows; the extra rows/cols are a solid border.
//
//however, all client calls can remain the same since the tilemap handles the changes..
#include <QWidget>
#include <QPainter>
#include <vector>
#include <string>

#include "tilemapcell.h"
#include "vector2.h"

#include "tilemap.h"

using namespace std;

//rows/cols are the integer # of cells in each dimentsion; xw, yw are the halfwidths of each cell
TileMap::TileMap(const int &rows_in, const int &cols_in, const int &xw_in, const int &yw_in, QWidget *parent)
	:QWidget(parent)
{	
	move(0,0);
	xw = xw_in; //store tile halfwidths
	yw = yw_in;
	
	tw = 2*xw; //store tile dimensions
	th = 2*yw;
	rows = rows_in;
	cols = cols_in;
	fullrows = rows+2;
	fullcols = cols+2;
	
	minX = tw;//world bounds only include the inner part of the grid;
	minY = th;//not the outside edges.
	maxX = tw + (rows* tw);
	maxY = th + (cols* th);
	
}

TileMap::~TileMap()
{
	//Well... I suppose there's nothing to delete for now.
}

//Build the TileMap
void TileMap::Build()
{	
	//these are here just to make it easier to move the code from where it was..
	
	int x = xw;
	int y = yw;
	
	std::vector < TileMapCell * > temp;
	
	//build raw tiles		
	for( int i = 0; i < fullcols; i++ )
	{
		temp.clear();
		for( int j = 0; j < fullrows; j++ )
		{
			TileMapCell *cell;
			cell = new TileMapCell(i,j,x,y,xw,yw, this);
			temp.push_back( cell );
			y += th;		
		}
		x += tw;
		y = yw;
		grid.push_back( temp );
	}				

	//link right
	for( int i = 0; i < (fullcols-1) ; i++ )
	{
		for( int j = 0; j < fullrows; j++ )
		{
			grid[i][j]->LinkR( grid[i+1][j] );
		}
	}		

	//link left
	for( int i = 1; i < fullcols; i++)
	{
		for( int j = 0; j < fullrows; j++)
		{
			grid[i][j]->LinkL( grid[i-1][j] );
		}
	}		

	//link down
	for( int i = 0; i < fullcols; i++)
	{
		for( int j = 0; j < (fullrows-1); j++)
		{
			grid[i][j]->LinkD( grid[i][j+1] );
		}
	}			

	//link up
	for( int i = 0; i < fullcols; i++)
	{
		for( int j = 1; j < fullrows; j++)
		{
			grid[i][j]->LinkU( grid[i][j-1] );
		}
	}			

	//fill top border tiles	
	for( int i = 0; i < fullcols; i++)
	{
		grid[i][0]->unbreakable = 1;	
		grid[i][0]->SetState(TID_FULL);		
	}
/* --- Bottom border are off.
	//fill bottom border tiles
	for( int i = 0; i < fullcols; i++)
	{
		grid[i][fullrows-1]->unbreakable = 1;
		grid[i][fullrows-1]->SetState(TID_FULL);
	}
*/
	//fill left border tiles
	for( int i = 0; i < fullrows; i++)
	{
		grid[0][i]->unbreakable = 1;
		grid[0][i]->SetState(TID_FULL);
	}
	
	//fill right border tiles		
	for( int i = 0; i < fullrows; i++)
	{
		grid[fullcols-1][i]->unbreakable = 1;
		grid[fullcols-1][i]->SetState(TID_FULL);
	}
	
}

//empties the grid
void TileMap::ClearGrid()
{
	
	for( int i = 0; i < fullcols; i++ )
	{
		grid[i].clear();
	}
	grid.clear();
	
}

	
//-------------------------------- tile access operators -----------------------

//returns a referance to the tile touching point x,y; scalar version
TileMapCell* TileMap::GetTile_S(const double &x, const double &y)
{
	return grid[ (int)(x / tw) ][ (int)(y / th) ];
}
//vector version
TileMapCell* TileMap::GetTile_V(const Vector2 &p)
{
	if( grid[ (int)(p.x/tw) ][ (int)(p.y/th) ] != NULL )
		return grid[ (int)(p.x/tw) ][ (int)(p.y/th) ];
	
	return NULL;
}
//index-based version
TileMapCell* TileMap::GetTile_I(const int &i, const int &j)
{
	return grid[i][j]; //note!! this will break if i or j is out of bounds!!!
}

//fills vector v with grid coordinates (i.e the cell index) of the tile at point x,y (scalar version)
void TileMap::GetIndex_S(Vector2 &v, const int &x, const int &y)
{
	v.x = (int)(x / tw);
	v.y = (int)(y / th);
}
void TileMap::GetIndex_V(Vector2 &v, const Vector2 &p)
{
	v.x = (int)(p.x/tw);
	v.y = (int)(p.y/th);
}

//these functions tokenize the tile states; currently they're assuming that
//the current state of the tilemap (i.e dimensions) are constant, i.e
//theyr'e not saved/loaded with the tile states.
//
//later we should change this.
//
//NOTE: the char with code 0 is apparently backspace or something.. so we pad the charcodes to get them in
//		a "normal" range.. (alphanumerics start at 48, upper case at 65)
//NOTE: the "\" works with input text, but acts like an escape character in code!! so, avoid it (ascii#92)

//returns a string of ascii characters, where each char is the tokenized description of a
//tile in the tilemap


std::string TileMap::GetTileStates()
{

	string output = "";

	for(int i = 1; i < cols+1; i++)
	{
		for(int j = 1; j < rows+1; j++)
		{
			output += grid[i][j]->ID + CHAR_PAD;			
		}
	}	
	
	return output;
}

//sets a single tile state
void TileMap::SetTileState(const int &i, const int &j, const char &ch)
{
	
	grid[i+1][j+1]->SetState( ch - CHAR_PAD );
}

//each char in the string is assumed to be a tokenized tile-type ID
void TileMap::SetTileStates(const string &instr)
{
	
	for(int i = 0; i < cols; i++)
	{
		for(int j = 0; j < rows; j++)
		{
			grid[i+1][j+1]->SetState( instr[ i*cols + j ] - CHAR_PAD );
		}
	}	
}
