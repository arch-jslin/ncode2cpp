#ifndef MYBUTTON_H
#define MYBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QPixmap>

class MyButton : public QPushButton
{
    Q_OBJECT
	
private:
	QPixmap buttonicon;
	
protected:
	void paintEvent(QPaintEvent *event);

public:
    MyButton(QPixmap bg, QWidget* parent = 0);
    
public slots:
	void slide();
	void slideback();
    
};

#endif  
