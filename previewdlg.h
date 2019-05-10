#ifndef PREVIEWDLG_H
#define PREVIEWDLG_H

#include <QDialog>

namespace Ui {
class PreviewDlg;
}

class PreviewDlg : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewDlg(QWidget *parent = nullptr);
    ~PreviewDlg();

private slots:
    void on_btn_watch_clicked();

    void on_btn_save_clicked();

private:
    Ui::PreviewDlg *ui;
    QString selected_file;
};

#endif // PREVIEWDLG_H
