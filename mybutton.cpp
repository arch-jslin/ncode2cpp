
#include "mybutton.h"

#include <QPushButton>
#include <QPainter>
#include <QPixmap>
#include <QWidget>


MyButton::MyButton(QPixmap bg, QWidget* parent)
	:QPushButton("123", parent), buttonicon(bg)
{
	setFixedSize( 200, 60 );
	connect( this, SIGNAL(pressed()), this, SLOT(slide()));
	connect( this, SIGNAL(released()), this, SLOT(slideback()));
}

void MyButton::paintEvent(QPaintEvent * /* event */)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, 1);	
	
	painter.drawPixmap(QRectF(20,10,150+20,38+10), buttonicon, QRectF(0,0,150,38));
}

void MyButton::slide()
{
	move( x()+2, y()+2 );
}

void MyButton::slideback()
{
	move( x()-2, y()-2 );
}