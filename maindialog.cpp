#include "maindialog.h"
#include "ui_maindialog.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <thread>
#include <QEventLoop>
#include <QSslConfiguration>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QProcess>
#include "previewdlg.h"


GlobalMsg global_msg;
QList<Task> tasks;
std::mutex mu;
std::condition_variable cond;


MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    QDir dir;
    if(!dir.exists("data"))
    {
        dir.mkdir("data");
    }
    if(!dir.exists("data_save"))
    {
        dir.mkdir("data_save");
    }

    auto thread_count = std::thread::hardware_concurrency() * 2;
    auto layout = ui->group_status->layout();
    for(auto i=0;i<thread_count;++i)
    {
        auto new_lab = new QLabel(this);
        labels.push_back(new_lab);
        layout->addWidget(new_lab);
        new_lab->setText("测试");

        std::thread(get_image_thread, i).detach();
    }

    connect(&global_msg, &GlobalMsg::update_msg, [=](QString msg, int index){
        labels.at(index)->setText(msg);
    });

    connect(&global_msg, &GlobalMsg::show_error, [=](QString error){
        QMessageBox::critical(this, "错误", error);
    });

    connect(&global_msg, &GlobalMsg::new_succ, [=]{
        pic_count += 1;
        ui->lab_status->setText("已下载:" + QString::number(pic_count));
    });


}

MainDialog::~MainDialog()
{
    delete ui;
}

void get_image_thread(int index)
{
    QNetworkAccessManager manager;
    ThreadObject *th_obj = new ThreadObject;
    QSslConfiguration ssl_conf;
    ssl_conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    while(true)
    {
        Task t;
        {
            std::unique_lock<std::mutex> lck(mu);
            if(tasks.isEmpty())
            {
                emit global_msg.update_msg("完成", index);
                cond.wait(lck);
            }
            if(tasks.empty())
            {
                continue;
            }
            t = tasks.first();
            tasks.pop_front();
        }
        emit global_msg.update_msg("正在加载 -> " + t.url, index);
        // qDebug()<<"正在加载 -> " + t.url;
        QNetworkRequest req;
        req.setUrl(t.url);
        req.setSslConfiguration(ssl_conf);
        auto reply =manager.get(req);

        QEventLoop loop;
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
        loop.exec();
        if(reply->error()!=QNetworkReply::NoError)
        {
            qDebug()<<reply->errorString();
            reply->deleteLater();
            continue;
        }
        auto url = reply->url();
        auto file_name = url.fileName();
        QFile file("data\\" + file_name);
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
        reply->deleteLater();
        emit global_msg.new_succ();
    }
}


void ThreadObject::on_reply(QNetworkReply *reply)
{
    if(reply->error()!=QNetworkReply::NoError)
    {
        qDebug()<<reply->errorString();
        return;
    }
    auto url = reply->url();
    qDebug()<<url.fileName();
    auto data = reply->readAll();

}

void MainDialog::on_btn_ctl_clicked()
{
    pic_count = 0;
    auto keyword = ui->edt_keywords->text();
    if(keyword.isEmpty())
    {
        QMessageBox::warning(this, "警告", "请输入关键字");
        return ;
    }

    if(last_key != keyword)
    {
        last_index = 1;
    }
    last_key = keyword;

    std::thread([=](){
        int sn = last_index;
        int pn = 50;

        QNetworkAccessManager manager;
        QNetworkRequest req;
        QSslConfiguration ssl_conf;
        ssl_conf.setPeerVerifyMode(QSslSocket::VerifyNone);
        req.setSslConfiguration(ssl_conf);

        QString url_str;
        url_str=QString("https://image.so.com/j?src=srp&q=%1&sn=%2&pn=%3").arg(QString(keyword.toUtf8().toPercentEncoding())).arg(sn).arg(pn);

        last_index += pn;

        QUrl url(url_str);
        qDebug()<<url;
        req.setUrl(url);
        QEventLoop loop;
        auto reply = manager.get(req);
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)
), &loop, SLOT(quit()));
        loop.exec();
        if(reply->error() != QNetworkReply::NoError)
        {
            qDebug()<<reply->errorString();
            emit global_msg.show_error("发生错误");
            reply->deleteLater();
            return ;
        }
        auto data = nlohmann::json::parse(reply->readAll().toStdString());
        reply->deleteLater();
        auto results = data["list"];
        {
            std::unique_lock<std::mutex> lck(mu);
            for(auto &p:results)
            {
                // qDebug()<<QString::fromStdString(p["img"]);
                tasks.append(Task{QString::fromStdString(p["img"])});
            }
        }
        cond.notify_all();
    }).detach();
}

void MainDialog::on_btn_open_clicked()
{
    QProcess::startDetached("explorer.exe .\\data_save");
}

void MainDialog::on_btn_preview_clicked()
{
    PreviewDlg dlg;
    dlg.exec();
}

void MainDialog::on_btn_clear_clicked()
{
    if(QMessageBox::question(this, "清空", "确认清空缓存？") != QMessageBox::Yes)
    {
        return;
    }
    QDir dir("data");
    dir.setFilter(QDir::Files);
    auto files = dir.entryInfoList();
    for(auto &p: files)
    {
        QFile::remove(p.filePath());
    }
    QMessageBox::information(this, "成功", "缓存已清除");
}
