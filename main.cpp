#include <QApplication>

#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication a (argc, argv);
    a.setApplicationName("JtvViewer");

    QStringList args = a.arguments();
    args.removeFirst();

    MainWindow* w = new MainWindow(args);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();

    return a.exec();
}
