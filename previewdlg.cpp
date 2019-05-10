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
    QDir dir("data");
    dir.setFilter(QDir::Files);
    auto files = dir.entryInfoList();
    for(int i=0;i<files.size();++i)
    {
        auto tmp_btn = new QPushButton();
        tmp_btn->setStyleSheet(QString("border-image:url(%1);").arg(files[i].filePath()));
        tmp_btn->setMinimumHeight(100);
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
    QProcess::startDetached(QString("cmd /c \"%1\"").arg(selected_file.replace('/','\\')));
}

void PreviewDlg::on_btn_save_clicked()
{
    if(selected_file.isEmpty())
    {
        return;
    }
    QFileInfo file(selected_file);
    QFile::rename(selected_file, "data_save\\" + file.fileName());
    QMessageBox::information(this, "成功", "保存成功");
}
