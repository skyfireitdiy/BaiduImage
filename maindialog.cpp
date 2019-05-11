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

// 全局变量的定义
GlobalMsg global_msg;
QList<Task> tasks;
std::mutex mu;
std::condition_variable cond;


MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    // 判断需要的目录是否存在，不存在就创建
    QDir dir;
    if(!dir.exists("data"))
    {
        dir.mkdir("data");
    }
    if(!dir.exists("data_save"))
    {
        dir.mkdir("data_save");
    }

    // 根据cpu线程数量创建线程，因为是高io的网络通信，所以创建的数量为cpu线程数量的2倍
    auto thread_count = std::thread::hardware_concurrency() * 2;
    auto layout = ui->group_status->layout();
    for(auto i=0;i<thread_count;++i)
    {
        // 每个线程创建一个显示线程信息的标签，加到主界面
        auto new_lab = new QLabel(this);
        labels.push_back(new_lab);
        layout->addWidget(new_lab);
        new_lab->setText("测试");
        // 分离线程，使其和主线程独立
        std::thread(get_image_thread, i).detach();
    }

    // 更新标签信息关联到标签
    connect(&global_msg, &GlobalMsg::update_msg, [=](QString msg, int index){
        labels.at(index)->setText(msg);
    });

    // 错误信息弹框
    connect(&global_msg, &GlobalMsg::show_error, [=](QString error){
        QMessageBox::critical(this, "错误", error);
    });

    // 下载成功的时候，更新界面上显示的数量
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
    // http请求的客户端
    QNetworkAccessManager manager;
    // ssl配置，在访问https网站的时候需要忽略远程验证
    QSslConfiguration ssl_conf;
    ssl_conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    // 不断循环从列表中获取任务执行
    while(true)
    {
        Task t;
        {
            // 获得列表的锁
            std::unique_lock<std::mutex> lck(mu);
            // 如果队列空，就等待新任务加入的信号，在这之前线程进入休眠状态
            if(tasks.isEmpty())
            {
                emit global_msg.update_msg("完成", index);
                cond.wait(lck);
            }
            // 等待后，再次判断队列是否为空（可能任务被其他线程抢了），如果空了，就进入下一轮循环（继续等待）
            if(tasks.empty())
            {
                continue;
            }
            // 取出一个任务，并将它从列表中删除
            t = tasks.first();
            tasks.pop_front();
        }
        // 更新界面信息
        emit global_msg.update_msg("正在加载 -> " + t.url, index);
        // 构建一个http请求
        QNetworkRequest req;
        req.setUrl(t.url);
        req.setSslConfiguration(ssl_conf);
        auto reply =manager.get(req);

        // 将一部转换为同步（因为是在线程中，所以没什么影响），http成功返回或者发生错误的时候退出等待
        QEventLoop loop;
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
        loop.exec();
        // 如果发生错误，继续处理下一轮任务
        if(reply->error()!=QNetworkReply::NoError)
        {
            qDebug()<<reply->errorString();
            reply->deleteLater();
            continue;
        }
        // 否则根据url中的文件名，将图片存储在data文件夹内
        auto url = reply->url();
        auto file_name = url.fileName();
        QFile file("data\\" + file_name);
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
        reply->deleteLater();
        // 向主界面发消息，更新数字
        emit global_msg.new_succ();
    }
}



void MainDialog::on_btn_ctl_clicked()
{
    // 开始按钮被单击，重新计数
    pic_count = 0;
    // 判断关键字
    auto keyword = ui->edt_keywords->text();
    if(keyword.isEmpty())
    {
        QMessageBox::warning(this, "警告", "请输入关键字");
        return ;
    }
    // 如果和上次搜索的关键字一样，说明这次是继续搜索，索引从上次的地方开始，不一样说明是新的搜索，索引从1开始
    if(last_key != keyword)
    {
        last_index = 1;
    }
    // 更新最后一次搜索的关键字
    last_key = keyword;

    // 启动一个线程去获取图片列表
    std::thread([=](){
        // 起始索引和请求数量
        int sn = last_index;
        int pn = 50;

        // http客户端，ssl配置
        QNetworkAccessManager manager;
        QNetworkRequest req;
        QSslConfiguration ssl_conf;
        ssl_conf.setPeerVerifyMode(QSslSocket::VerifyNone);
        req.setSslConfiguration(ssl_conf);

        // 向360图片的接口发送请求
        QString url_str;
        url_str=QString("https://image.so.com/j?src=srp&q=%1&sn=%2&pn=%3").arg(QString(keyword.toUtf8().toPercentEncoding())).arg(sn).arg(pn);

        // 当前索引增加，下次可以从这里继续
        last_index += pn;

        QUrl url(url_str);
        req.setUrl(url);
        // 发出请求并等待
        QEventLoop loop;
        auto reply = manager.get(req);
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)
), &loop, SLOT(quit()));
        loop.exec();
        // 如果发生错误，弹出错误框
        if(reply->error() != QNetworkReply::NoError)
        {
            qDebug()<<reply->errorString();
            emit global_msg.show_error("发生错误");
            reply->deleteLater();
            return ;
        }
        // 解析返回的json字符串，获取到里面的list对象，list存储了每个图片的信息
        auto data = nlohmann::json::parse(reply->readAll().toStdString());
        reply->deleteLater();
        // 遍历list，获取每张图片的url，加入任务队列
        auto results = data["list"];
        {
            std::unique_lock<std::mutex> lck(mu);
            for(auto &p:results)
            {
                tasks.append(Task{QString::fromStdString(p["img"])});
            }
        }
        // 唤醒现在正在休眠的图片下载线程，告诉他们可以工作了
        cond.notify_all();
    }).detach();
}

void MainDialog::on_btn_open_clicked()
{
    // 调用资源管理器，打开data_save目录
    QProcess::startDetached("explorer.exe .\\data_save");
}

void MainDialog::on_btn_preview_clicked()
{
    // 显示预览对话框
    PreviewDlg dlg;
    dlg.exec();
}

void MainDialog::on_btn_clear_clicked()
{
    // 删除缓存询问
    if(QMessageBox::question(this, "清空", "确认清空缓存？") != QMessageBox::Yes)
    {
        return;
    }
    // 遍历data下的文件并删除
    QDir dir("data");
    dir.setFilter(QDir::Files);
    auto files = dir.entryInfoList();
    for(auto &p: files)
    {
        QFile::remove(p.filePath());
    }
    QMessageBox::information(this, "成功", "缓存已清除");
}
