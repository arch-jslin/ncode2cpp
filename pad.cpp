#include <QPainter>
#include <QKeyEvent>

#include "pad.h"

Pad::Pad(QWidget* parent)
		: QWidget(parent)
{
    //Constructor
    setPalette(QColor(255,255,255));
    setFixedSize( 72, 5 );
    
    setFocus();
}

Pad::~Pad()
{
    //Deconstructor
}

void Pad::paintEvent(QPaintEvent * /* event */)
{
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::darkGray);
	painter.drawRect( rect() );
}

void Pad::keyPressEvent(QKeyEvent *event)
{
	switch( event->key() )
	{
		case Qt::Key_Left:
			if( x() > 60 )
				submove(x()-5, y());
			break;
			
		case Qt::Key_Right:
			if( x() < 300 )
				submove(x()+5, y());
		    break;
		    
		default:
			event->ignore();
	}
}

QRegion Pad::getRegion()
{
	return region;
}

void Pad::submove(int x, int y)
{
    move(x, y);	
    region = QRegion( geometry(), QRegion::Rectangle );
}
