#include "plotdialog.h"
#include "ui_plotdialog.h"
#include <QSettings>

const char *defcmds[] =
{
    "gnuplot {SCRIPT}",
    "gnuplot {SCRIPT}",
    "qtiplot {FILE}"

};

const char *defscripts[]=
{
    "# Terminal x11 is basic\n"
    "# Terminal qt has more features\n"
    "# but only the most recent one works at one time\n"
    "# You can change the size\n"
    "xsize=800\n"
    "ysize=600\n"
    "set terminal qt 1 size xsize,ysize\n"
    "set datafile separator \",\"\n"
    "set title \"Channel 1\"\n"
    "plot \"{FILE}\" using 1:2 with lines  title columnhead(2)  lc \"orange\"\n"
    "# You can play with the window until you click the mouse\n"
    "pause mouse\n"
    "# Go on to channel 2\n"
    "set title \"Channel 2\"\n"
    "plot \"{FILE}\" using 1:3 with lines title columnhead(3) lc \"blue\"\n"
    "pause mouse close # for some reason, now you'll have to close with the X on the window\n",
    "set terminal postscript portrait enhanced mono\nset output '/tmp/scope.ps'\nset datafile separator \",\"\nplot \"{FILE}\" using 1:2 with lines\npause -1\n",
    ""
};

PlotDialog::PlotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotDialog)
{
    QSettings set;
    ui->setupUi(this);
    command=set.value("plot/command",defcmds[0]).toString();
    script=set.value("plot/script",defscripts[0]).toString();
    ui->plotscript->setPlainText(script);
    ui->plotcmd->setText(command);
}

PlotDialog::~PlotDialog()
{
    delete ui;
}


void PlotDialog::on_plotscript_textChanged()
{
    QSettings set;
     script=ui->plotscript->toPlainText();
     set.setValue("plot/script",script);
}


void PlotDialog::on_plotcmd_textChanged(const QString &arg1)
{
    QSettings set;
    command=arg1;
    set.setValue("plot/command",command);
}





void PlotDialog::on_comboBox_currentIndexChanged(int index)
{
    if (index==0) return;
    index--;
    QSettings set;
    command=defcmds[index];
    script=defscripts[index];
    ui->plotcmd->setText(command);
    ui->plotscript->setPlainText(script);
    set.setValue("plot/command",command);
    set.setValue("plot/script",script);
    ui->comboBox->setCurrentIndex(0);

}
