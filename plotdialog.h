#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <QDialog>

namespace Ui {
class PlotDialog;
}

class PlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlotDialog(QWidget *parent = 0);
    QString command;
    QString script;
    ~PlotDialog();

private slots:

    void on_plotscript_textChanged();

    void on_plotcmd_textChanged(const QString &arg1);


    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::PlotDialog *ui;
};

#endif // PLOTDIALOG_H
