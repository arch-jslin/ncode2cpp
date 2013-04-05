
//* Class Vector2 Definitions *//

#ifndef VECTOR2_H
#define VECTOR2_H

#include <string>

using namespace std;

class Vector2
{
private:
	

public:
	
	double x,y;

	Vector2() { x = y = 0 ; }
	Vector2(const double &x_in, const double &y_in);  //ctor
	~Vector2(); //dector
	
	string ToString();
	
	Vector2 clone();
	Vector2 plus(const Vector2 &v2);
	Vector2 minus(const Vector2 &v2);
	Vector2 normR();
	Vector2 dir();
	Vector2 proj(Vector2 v2);
	
	double projLen(Vector2 v2);
	double dot(const Vector2 &v2);
	double cross(const Vector2 &v2);
	double len();
	
	void copy(const Vector2 &v2);
	void mult(const double &s);
	void normalize();
	void pluseq(const Vector2 &v2);
	void minuseq(const Vector2 &v2);
	
};

#endif   // VECTOR2_H
