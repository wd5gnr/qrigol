#include "plotdialog.h"
#include "ui_plotdialog.h"
#include <QSettings>


const char defcmd[]="gnuplot {SCRIPT}";
const char defscript[]="set terminal X11\nset datafile separator \",\"\nplot \"{FILE}\" using 1:2 with lines\npause -1";

PlotDialog::PlotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotDialog)
{
    QSettings set;
    ui->setupUi(this);
    command=set.value("plot/command",defcmd).toString();
    script=set.value("plot/script",defscript).toString();
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


const char *defcmds[] =
{
    "gnuplot {SCRIPT}",
    "gnuplot {SCRIPT}",
    "qtiplot {FILE}"

};

const char *defscripts[]=
{
    "set terminal X11\nset datafile separator \",\"\nplot \"{FILE}\" using 1:2 with lines\npause -1\n",
    "set terminal postscript portraint enhanced mono\nset output '/tmp/scope.ps'\n,set datafile separator \",\"\nplot \"{FILE}\" using 1:2 with lines\npause -1\n",
    ""
};


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
