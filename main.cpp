#include <QApplication>
#include "gameboard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GameBoard gameboard;
    gameboard.show();

    return app.exec();
}
