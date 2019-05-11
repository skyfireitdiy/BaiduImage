#include <QApplication>
#include <maindialog.h>

/**
 * @brief qMain 入口函数，创建app实例，创建主窗口，显示主窗口，开始消息循环
 * @param argc 命令行参数数量
 * @param argv 命令行参数内容
 * @return 0
 */
int main(int argc, char ** argv)
{
    QApplication app(argc, argv);
    MainDialog main_dlg;
    main_dlg.show();
    return app.exec();
}
