#ifndef PAD_H
#define PAD_H

#include <QWidget>

class Pad : public QWidget
{
    Q_OBJECT
	
private:
	QRegion region;
	
	
protected:
	void paintEvent(QPaintEvent *event);
	void keyPressEvent(QKeyEvent *event);

public:
    Pad(QWidget* parent = 0);
    ~Pad();
    
    QRegion getRegion();
    void submove(int x, int y);
    
public slots:


signals:

	
};

#endif  // PAD_H
