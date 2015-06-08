#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>

namespace Ui {
class HelpDialog;
}

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = 0);
    ~HelpDialog();
    void showRequest(const QString &topic=QString());

private slots:

    void on_indexButton_clicked();

    void on_search_clicked();

    void on_webView_loadFinished(bool arg1);

private:
    Ui::HelpDialog *ui;
    QString topicload;
};

#endif // HELPDIALOG_H
