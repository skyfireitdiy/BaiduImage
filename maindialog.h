#ifndef MAINDIGLOG_H
#define MAINDIGLOG_H

/**
  主界面逻辑
  */

#include <QDialog>
#include <QList>
#include <mutex>
#include <condition_variable>
#include <QNetworkReply>
#include <QLabel>
#include <json.hpp>
#include <vector>

namespace Ui {
class MainDialog;
}

/**
 * @brief 全局通信类，因为qt不允许在子线程中直接操作界面，所以子线程要在界面上显示一些消息鼻息通过信号槽的方式
 */
class GlobalMsg : public QObject{
    Q_OBJECT
signals:
    /**
     * @brief update_msg 更新状态消息，主界面有N个状态显示框，此处用于更新这些框的信息
     * @param msg 要显示的信息
     * @param index 第几个框
     */
    void update_msg(QString msg, int index);
    /**
     * @brief show_error 显示报错信息（弹窗）
     * @param error 报错信息
     */
    void show_error(QString error);
    /**
     * @brief new_succ 新的图片下载成功，此时主窗口会更新显示的数字
     */
    void new_succ();
};


/**
 * @brief 线程任务的参数
 */
struct Task{
    QString url;
};


/**
 * @brief tasks 任务列表
 */
extern QList<Task> tasks;
/**
 * @brief global_msg 用于通信的全局变量
 */
extern GlobalMsg global_msg;
/**
 * @brief mu 任务列表互斥访问的互斥量
 */
extern std::mutex mu;
/**
 * @brief cond 访问任务列表的条件变量
 */
extern std::condition_variable cond;

/**
 * @brief get_image_thread 获取图片的线程函数
 * @param index 与本线程绑定的状态标签序号，本线程的信息会显示在那个标签上
 */
void get_image_thread(int index);


/**
 * @brief 主窗口
 */
class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private slots:
    /**
     * @brief on_btn_ctl_clicked 开始按钮响应
     */
    void on_btn_ctl_clicked();

    /**
     * @brief on_btn_open_clicked 打开按钮响应
     */
    void on_btn_open_clicked();

    /**
     * @brief on_btn_preview_clicked 预览按钮
     */
    void on_btn_preview_clicked();

    /**
     * @brief on_btn_clear_clicked 清除缓存按钮
     */
    void on_btn_clear_clicked();

private:
    Ui::MainDialog *ui;

    /**
     * @brief labels 状态标签合集
     */
    std::vector<QLabel*>  labels;
    /**
     * @brief pic_count 图片数量，此处使用原子变量
     */
    std::atomic<int> pic_count = 0;

    /**
     * @brief last_key 最后搜索的关键字
     */
    QString last_key = "";
    /**
     * @brief last_index 最后搜索的图片索引
     */
    int last_index = 0;
};

#endif // MAINDIGLOG_H
