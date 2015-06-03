#include "plotdialog.h"
#include "ui_plotdialog.h"
#include <QSettings>

const char *defcmds[] =
{
    "gnuplot {SCRIPT}",
    "gnuplot {SCRIPT}",
    "gnuplot {SCRIPT}",
    "qtiplot {FILE}",
    "bash {SCRIPT}",
    "libreoffice --calc {FILE}"

};

const char *defscripts[]=
{
    "xsize=800\n"
    "ysize=600\n"
    "set terminal qt 1 size xsize,ysize\n"
    "set datafile separator \",\"\n"
    "set title \"Channel 1\"\n"
    "plot \"{FILE}\" using 1:2 with lines  title columnhead(2)  lc \"orange\",  \"{FILE}\" using 1:3 with lines title columnhead(3) lc \"blue\"\n"
    "pause mouse close\n",
    "xsize=800\n"
    "ysize=600\n"
    "set terminal x11 1 size xsize,ysize\n"
    "set datafile separator \",\"\n"
    "set title \"Channel 1\"\n"
    "plot \"{FILE}\" using 1:2 with lines  title columnhead(2)  lc \"orange\",  \"{FILE}\" using 1:3 with lines title columnhead(3) lc \"blue\"\n"
    "pause mouse close\n",
    "set terminal pdf size 8.00in,11.00in\nset output '/tmp/scope.pdf'\nset datafile separator \",\"\nplot \"{FILE}\" using 1:2 with lines\npause -1\n",
    "",
    "gnuplot -persist -e \"\n"
    "set terminal qt size 800,600;\n"
    "set datafile separator \\\",\\\";\n"
    "set title \\\"Channel 1\\\";\n"
    "plot \\\"{FILE}\\\" using 1:2 with lines  title columnhead(2)  lc \\\"orange\\\";\n"
    "pause mouse button3 ;\n"
    "\"   &\n"
    "gnuplot -persist -e \"\n"
    "set terminal qt size 800,600;\n"
    "set datafile separator \\\",\\\";\n"
    "set title \\\"Channel 2\\\";\n"
    "plot \\\"{FILE}\\\" using 1:3 with lines  title columnhead(3)  lc \\\"blue\\\";\n"
    "pause mouse button3 ;\n"
    "\"   &\n",
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
    ui->comboBox->setCurrentIndex(0);   // this probably blows if using keyboard

}
