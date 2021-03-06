#include "previewdlg.h"
#include "ui_previewdlg.h"
#include <QDir>
#include <QPushButton>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>

PreviewDlg::PreviewDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreviewDlg)
{
    ui->setupUi(this);
    // 遍历data下的文件
    QDir dir("data");
    dir.setFilter(QDir::Files);
    auto files = dir.entryInfoList();
    for(int i=0;i<files.size();++i)
    {
        // 每个文件创建一个按钮，按钮的背景设置为对应的图片，高度设置为100
        auto tmp_btn = new QPushButton();
        tmp_btn->setStyleSheet(QString("border-image:url(%1);").arg(files[i].filePath()));
        tmp_btn->setMinimumHeight(100);
        // 当按钮被点击的时候，将当前选中设置为点击的图片路径
        connect(tmp_btn, &QPushButton::clicked, [=]{
            selected_file = files[i].filePath();
            ui->lab_tip->setText("当前已选择:" + selected_file);
        });
        ui->lay_preview->addWidget(tmp_btn, i/3, i%3);
    }
}

PreviewDlg::~PreviewDlg()
{
    delete ui;
}

void PreviewDlg::on_btn_watch_clicked()
{
    if(selected_file.isEmpty())
    {
        return;
    }
    // 调用系统默认图片查看器
    QProcess::startDetached(QString("cmd /c \"%1\"").arg(selected_file.replace('/','\\')));
}

void PreviewDlg::on_btn_save_clicked()
{
    if(selected_file.isEmpty())
    {
        return;
    }
    QFileInfo file(selected_file);
    //将选中的图片移动到data_save目录下
    QFile::rename(selected_file, "data_save\\" + file.fileName());
    QMessageBox::information(this, "成功", "保存成功");
}
