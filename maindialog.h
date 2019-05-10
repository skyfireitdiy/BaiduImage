#ifndef MAINDIGLOG_H
#define MAINDIGLOG_H

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

class GlobalMsg : public QObject{
    Q_OBJECT
signals:
    void update_msg(QString msg, int index);
    void show_error(QString error);
    void new_succ();
};


struct Task{
    QString url;
};


class ThreadObject : public QObject
{
    Q_OBJECT
public slots:
    void on_reply(QNetworkReply *reply);
};



extern QList<Task> tasks;
extern GlobalMsg global_msg;
extern std::mutex mu;
extern std::condition_variable cond;

void run_task(const Task& task);
void get_image_thread(int index);


class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private slots:
    void on_btn_ctl_clicked();

    void on_btn_open_clicked();

    void on_btn_preview_clicked();

    void on_btn_clear_clicked();

private:
    Ui::MainDialog *ui;

    std::vector<QLabel*>  labels;
    std::atomic<int> pic_count = 0;

    QString last_key = "";
    int last_index = 0;
};

#endif // MAINDIGLOG_H
