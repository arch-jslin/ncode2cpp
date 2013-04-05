/*
Vector2 object
--------------

============
*/

#include <iostream>
#include <sstream>
#include <cmath>

#include "vector2.h"

Vector2::Vector2(const double &x_in, const double &y_in)
{
	x = x_in;
	y = y_in;
}

Vector2::~Vector2()
{
	//nothing to delete	
}

//(returns a formatted string containing x,y)
string Vector2::ToString()
{
	ostringstream oss;
	oss << "(" << x << "," << y << ")" << std::endl;
	return oss.str();
}


//----- these functions return Vector2s -----

//return a copy of this
Vector2 Vector2::clone()
{
	return Vector2(x, y);
}

//return this+v2
Vector2 Vector2::plus(const Vector2 &v2)
{
	return Vector2( x + v2.x, y + v2.y );
}

//return this-v2
Vector2 Vector2::minus(const Vector2 &v2)
{
	return Vector2( x - v2.x, y - v2.y );
}

//return the righthand normal of this 
Vector2 Vector2::normR()
{
	return Vector2( -y, x );	
}

//return the (unit) direction vector of this
Vector2 Vector2::dir()
{
	Vector2 v = clone();
	v.normalize();
	return v;
}

//return this projected _onto_ v2
Vector2 Vector2::proj(Vector2 v2)
{

	double den = v2.dot(v2);
	Vector2 v;
	if( den == 0 )
	{
		//zero-length v2
		//"WARNING! Vector2.proj() was given a zero-length projection vector!"
		v = clone();//not sure how to gracefully recover but, hopefully this will be okay
	}
	else
	{
		v = v2.clone();
		v.mult( dot(v2) / den );
	}
	
	return v;
}

//return the magnitude (absval) of this projected onto v2
double Vector2::projLen(Vector2 v2)
{

	double den = v2.dot(v2);
	if( den == 0 )
	{
		//zero-length v2
		//"WARNING! Vector2.projLen() was given a zero-length projection vector!"
		return 0;
	}
	else
	{
		return abs( dot(v2) / den );
	}
	
}


//----- these functions return scalars -----

//return the dotprod of this and v2
double Vector2::dot(const Vector2 &v2)
{
	return (x * v2.x) + (y * v2.y);
}

//return the crossprod of this and v2
//note that this is equivalent to the dotprod of this and the lefthand normal of v2
double Vector2::cross(const Vector2 &v2)
{
	return (x * v2.y) - (y * v2.x);
}

///return the length of this
double Vector2::len()
{
	return sqrt( (x*x) + (y*y) );
}


//----- these functions return nothing (they operate on this) -----

//change this to a duplicate of v2
void Vector2::copy(const Vector2 &v2)
{
	x = v2.x;
	y = v2.y;
}

//multiply this by a scalar s
void Vector2::mult(const double &s)
{
	x *= s;
	y *= s;
}

//convert this vector to a unit/direction vector
void Vector2::normalize()
{
	double L = len();
	if( L != 0 )
	{
		x /= L;
		y /= L;	
	}
	else
	{
		//"WARNING! Vector2.normalize() was called on a zero-length vector!"
	}
}

//add v2 to this
void Vector2::pluseq(const Vector2 &v2)
{
	x += v2.x;
	y += v2.y;
}

//subtract v2 from this
void Vector2::minuseq(const Vector2 &v2)
{
	x -= v2.x;
	y -= v2.y;
}


