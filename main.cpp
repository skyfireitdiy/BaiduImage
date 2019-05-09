#include <QApplication>
#include <maindialog.h>

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);
    MainDialog main_dlg;
    main_dlg.show();
    return app.exec();
}
