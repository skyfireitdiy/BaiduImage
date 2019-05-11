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
    /**
     * @brief on_btn_watch_clicked 查看按钮
     */
    void on_btn_watch_clicked();

    /**
     * @brief on_btn_save_clicked 保存按钮
     */
    void on_btn_save_clicked();

private:
    Ui::PreviewDlg *ui;
    /**
     * @brief selected_file 当前选中的文件路径
     */
    QString selected_file;
};

#endif // PREVIEWDLG_H
