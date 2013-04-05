//* tilemap.h *//

#ifndef TILEMAP_H
#define TILEMAP_H

#include <QWidget>

const int CHAR_PAD = 48;

class TileMapCell;
class Vector2;

class TileMap : public QWidget
{
	Q_OBJECT
	
private:


protected:
	//void paintEvent(QPaintEvent * /* event */);


public:

	int xw; //store tile halfwidths
	int yw;
	
	int tw; //store tile dimensions
	int th;
	int rows;
	int cols;
	int fullrows;
	int fullcols;
	
	int minX;//world bounds only include the inner part of the grid;
	int minY;//not the outside edges.
	int maxX;
	int maxY;
	
	std::vector< std::vector < TileMapCell* > > grid;

	TileMap(const int &rows_in, const int &cols_in, const int &xw_in, const int &yw_in, QWidget *parent = 0);
	~TileMap();

	void Build();
	void ClearGrid();
	
	TileMapCell* GetTile_S(const double &x, const double &y);
	TileMapCell* GetTile_V(const Vector2 &p);
	TileMapCell* GetTile_I(const int &i, const int &j);
	
	void GetIndex_S(Vector2 &v, const int &x, const int &y);
	void GetIndex_V(Vector2 &v, const Vector2 &p);
	
	std::string GetTileStates();
	void SetTileState(const int &i, const int &j, const char &ch);
	void SetTileStates(const std::string &instr);

};


#endif //TILEMAP_H
