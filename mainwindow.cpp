#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSettings>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <QTemporaryFile>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QProcess>
#include "unistd.h"
#include "plotdialog.h"

// TODO: Read scales
// TODO: The plot example UI probably doesn't work well with a keyboard


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    nocommands=false;  // used to inhibit commands while updating UI
// Force current tab
    ui->tabWidget->setCurrentIndex(0);
    ui->statusBar->showMessage("Disconnected");
// recall device name and other settings
    QString dev;
    QSettings set;
    dev=set.value("Options/Device","/dev/usbtmc0").toString();
    ui->unlockBtn->setChecked(set.value("Options/Unlock",false).toBool());
    ui->hoffincr->setValue(set.value("Options/hincr",100.0).toFloat());
    ui->deviceName->setText(dev);
    ui->hoffsetspin->setSingleStep(ui->hoffincr->value());
    ui->tboxpulse->setVisible(false);
    ui->tboxslope->setVisible(false);

// set up uTimer (for measure acq) and uiTimer (for updating RS button and handling unlock)
    uTimer=new QTimer(this);
    uiTimer=new QTimer(this);
    connect(uTimer,SIGNAL(timeout()),this,SLOT(on_measUpdate_clicked()));
    connect(uiTimer,SIGNAL(timeout()),this,SLOT(on_uiUpdate()));
    uiTimer->start(1000);
}

MainWindow::~MainWindow()
{
    if (com.connected()) com.close();
    delete ui;
}

// Helpers that take QString
int MainWindow::command(const QString &cmd)
{
  return com.command(cmd.toLatin1());
}

float MainWindow::cmdFloat(const QString &cmd)
{
  return com.cmdFloat(cmd.toLatin1());
}

// More helpers
void MainWindow::waitForStop(void)
{
    QTime timeout;
    timeout.start();
    do
    {
        com.command(":TRIG:STAT?");
    } while (*com.buffer!='S' && timeout.elapsed()<2000);  // Wait for stop or 2 seconds

}

void MainWindow::on_uiUpdate()  // periodic update of scope status -- this makes it impossible to unlock so we need a supression
{
    if (!com.connected()) return;
    com.command(":TRIG:STAT?");
    ui->actionRun_Stop->setChecked(*com.buffer!='S');
    ui->rsButton->setStyleSheet(*com.buffer!='S'?"background: green;":"background: red;");
    if (ui->unlockBtn->isChecked()) com.unlock();
    if (mlogworker.isFinished()) ui->measLogEnable->setChecked(false);
    ui->actionConnect->setChecked(com.connected());



}

void MainWindow::on_connectButton_clicked()
{

    if (!com.connected())
    {
        ui->connectButton->setChecked(false);
        com.open(ui->deviceName->text().toLatin1());
        if (!com.connected())
        {
            QMessageBox bx;
            bx.setText("Can't open device");
            ui->statusBar->showMessage("Can't open device");
            bx.exec();
            return;
        }
        if (com.command("*IDN?") < 0)
        {
            ui->idstring->setText("No response");
            ui->statusBar->showMessage("No response");
        }
        else
        {
            QSettings set;
            ui->idstring->setText(com.buffer);
            set.setValue("Options/Device",ui->deviceName->text());
            ui->statusBar->showMessage(com.buffer);
        }
        ui->deviceName->setEnabled(false);
        ui->rsButton->setEnabled(true);
        ui->connectButton->setText("&Disconnect");
        on_uiUpdate();
        on_updAcq_clicked();
        ui->connectButton->setChecked(true);

    }
    else
    {
        ui->statusBar->showMessage("Disconnected");
        ui->deviceName->setEnabled(true);
        ui->rsButton->setEnabled(false);
        com.close();
        ui->idstring->clear();
        ui->connectButton->setText("&Connect");



    }

}

void MainWindow::on_rsButton_clicked()
{
    int rs;
    if (!com.connected()) return;
    com.command(":TRIG:STAT?");
    rs=*com.buffer!='S';

    if (rs)
    {
        rs=0;
        com.send(":STOP");
        ui->rsButton->setStyleSheet("background: red;");
        ui->hoffsetspin->setMinimum(-500000000.0f);
        ui->hoffsetspin->setMaximum(500000000.0f);
    }
    else
    {
        //float hmax=1.0f;
        rs=1;
        com.send(":RUN");
        ui->rsButton->setStyleSheet("background: green;");
       // ui->hoffsetspin->setMinimum(0.0f);
// TODO: Need to read current scale
       // ui->hoffincr->setMaximum(hmax);

    }

}


void MainWindow::on_measUpdate_clicked()
{
    if (!com.connected()) return;
    ui->vpp1->setText(QString::number(lastMeasure[0][0]=com.cmdFloat(":MEAS:VPP? CHAN1")));
    ui->vmax1->setText(QString::number(lastMeasure[0][1]=com.cmdFloat(":MEAS:VMAX? CHAN1")));
    ui->vmin1->setText(QString::number(lastMeasure[0][2]=com.cmdFloat(":MEAS:VMIN? CHAN1")));
    ui->vavg1->setText(QString::number(lastMeasure[0][3]=com.cmdFloat(":MEAS:VAV? CHAN1")));
    ui->vamp1->setText(QString::number(lastMeasure[0][4]=com.cmdFloat(":MEAS:VAMP? CHAN1")));
    ui->vrms1->setText(QString::number(lastMeasure[0][5]=com.cmdFloat(":MEAS:VRMS? CHAN1")));
    ui->vtop1->setText(QString::number(lastMeasure[0][6]=com.cmdFloat(":MEAS:VTOP? CHAN1")));
    ui->vbas1->setText(QString::number(lastMeasure[0][7]=com.cmdFloat(":MEAS:VBAS? CHAN1")));
    ui->pwid1->setText(QString::number(lastMeasure[0][8]=com.cmdFloat(":MEAS:PWID? CHAN1")*1000.0));
    ui->nwid1->setText(QString::number(lastMeasure[0][9]=com.cmdFloat(":MEAS:NWID? CHAN1")*1000.0));
    ui->freq1->setText(QString::number(lastMeasure[0][10]=com.cmdFloat(":MEAS:FREQ? CHAN1")));
    ui->per1->setText(QString::number(lastMeasure[0][11]=com.cmdFloat(":MEAS:PER? CHAN1")*1000.0));
    ui->over1->setText(QString::number(lastMeasure[0][12]=com.cmdFloat(":MEAS:OVER? CHAN1")*100.0));
    ui->pres1->setText(QString::number(lastMeasure[0][13]=com.cmdFloat(":MEAS:PRES? CHAN1")*100.0));
    ui->rise1->setText("<" + QString::number(lastMeasure[0][14]=com.cmdFloatlt(":MEAS:RIS? CHAN1")*1E6));
    ui->fall1->setText("<" + QString::number(lastMeasure[0][15]=com.cmdFloatlt(":MEAS:FALL? CHAN1")*1E6));
    ui->pdut1->setText(QString::number(lastMeasure[0][16]=com.cmdFloat(":MEAS:PDUT? CHAN1")*100.0));
    ui->ndut1->setText(QString::number(lastMeasure[0][17]=com.cmdFloat(":MEAS:NDUT? CHAN1")*100.0));
    ui->pdel1->setText(QString::number(lastMeasure[0][18]=com.cmdFloat(":MEAS:PDEL? CHAN1")*1000.0));
    ui->ndel1->setText(QString::number(lastMeasure[0][19]=com.cmdFloat(":MEAS:NDEL? CHAN1")*1000.0));


    ui->vpp2->setText(QString::number(lastMeasure[1][0]=com.cmdFloat(":MEAS:VPP? CHAN2")));
    ui->vmax2->setText(QString::number(lastMeasure[1][1]=com.cmdFloat(":MEAS:VMAX? CHAN2")));
    ui->vmin2->setText(QString::number(lastMeasure[1][2]=com.cmdFloat(":MEAS:VMIN? CHAN2")));
    ui->vavg2->setText(QString::number(lastMeasure[1][3]=com.cmdFloat(":MEAS:VAV? CHAN2")));
    ui->vamp2->setText(QString::number(lastMeasure[1][4]=com.cmdFloat(":MEAS:VAMP? CHAN2")));
    ui->vrms2->setText(QString::number(lastMeasure[1][5]=com.cmdFloat(":MEAS:VRMS? CHAN2")));
    ui->vtop2->setText(QString::number(lastMeasure[1][6]=com.cmdFloat(":MEAS:VTOP? CHAN2")));
    ui->vbas2->setText(QString::number(lastMeasure[1][7]=com.cmdFloat(":MEAS:VBAS? CHAN2")));
    ui->pwid2->setText(QString::number(lastMeasure[1][8]=com.cmdFloat(":MEAS:PWID? CHAN2")*1000.0));
    ui->nwid2->setText(QString::number(lastMeasure[1][9]=com.cmdFloat(":MEAS:NWID? CHAN2")*1000.0));
    ui->freq2->setText(QString::number(lastMeasure[1][10]=com.cmdFloat(":MEAS:FREQ? CHAN2")));
    ui->per2->setText(QString::number(lastMeasure[1][11]=com.cmdFloat(":MEAS:PER? CHAN2")*1000.0));
    ui->over2->setText(QString::number(lastMeasure[1][12]=com.cmdFloat(":MEAS:OVER? CHAN2")*100.0));
    ui->pres2->setText(QString::number(lastMeasure[1][13]=com.cmdFloat(":MEAS:PRES? CHAN2")*100.0));
    ui->rise2->setText("<" + QString::number(lastMeasure[1][14]=com.cmdFloatlt(":MEAS:RIS? CHAN2")*1E6));
    ui->fall2->setText("<" + QString::number(lastMeasure[1][15]=com.cmdFloatlt(":MEAS:FALL? CHAN2")*1E6));
    ui->pdut2->setText(QString::number(lastMeasure[1][16]=com.cmdFloat(":MEAS:PDUT? CHAN2")*100.0));
    ui->ndut2->setText(QString::number(lastMeasure[1][17]=com.cmdFloat(":MEAS:NDUT? CHAN2")*100.0));
    ui->pdel2->setText(QString::number(lastMeasure[1][18]=com.cmdFloat(":MEAS:PDEL? CHAN2")*1000.0));
    ui->ndel2->setText(QString::number(lastMeasure[1][19]=com.cmdFloat(":MEAS:NDEL? CHAN2")*1000.0));

    if (ui->measLogEnable->isChecked())
    {
        mlogworker.inuse.lock();
        for (int i=0;i<2;i++)  // should use std::copy (std::copy(&array[0][0],&array[0][0]+rows*columns,&dest[0][0]);
            for (int j=0;j<20;j++) mlogworker.data[i][j]=lastMeasure[i][j];
        mlogworker.sample.release();
        mlogworker.inuse.unlock();
    }

}

void MainWindow::on_autoUpdate_clicked()  // if auto update measurements
{
    if (ui->autoUpdate->checkState())
    {
        unsigned timeout;
        ui->upInterval->setEnabled(false);
        ui->updTimeScale->setEnabled(false);
        ui->measUpdate->setEnabled(false);
        timeout=ui->upInterval->value();
        switch (ui->updTimeScale->currentIndex())  // scale for ms, seconds, etc.
        {
        case 0:
            break;
        case 1:
            timeout*=1000;
            break;
        case 2:
            timeout*=1000*60;
            break;
        case 3:
            timeout*=1000*60*60;
        }

        uTimer->start(timeout);
        on_measUpdate_clicked();

    }
    else
    {
        ui->measUpdate->setEnabled(true);
        ui->upInterval->setEnabled(true);
        ui->updTimeScale->setEnabled(true);
        uTimer->stop();
    }
}

void MainWindow::on_hardcopyBTN_clicked()  // scope's "hardcopy" function
{
    if (!com.connected()) return;
    com.command(":HARD");
}

int MainWindow::cmdCharIndex(const QString &cmd,const QString &search,int bpos)
{
    QChar c;
    command(cmd);
    c=com.buffer[bpos];
    return search.indexOf(c,0,Qt::CaseInsensitive);
}

void MainWindow::setupChannel(int ch,QComboBox *probebox,QComboBox *scalebox)
{
    QString cmdbase=":CHAN";
    QString cmd;
    cmdbase+=QString::number(ch);
    cmd=cmdbase+":PROB?";
    float probe=cmdFloat(cmd);
    probebox->setCurrentIndex(probebox->findText(QString::number(probe)+"X"));
    scalebox->clear();
    if (probe==1.0)
    {
        scalebox->addItem("0.002");
        scalebox->addItem("0.005");
    }
    if (probe<=5.0)
    {
        scalebox->addItem("0.01");
    }
    if (probe<=10.0)
    {
        scalebox->addItem("0.02");
        scalebox->addItem("0.05");
    }
    if (probe<=50.0)
    {
        scalebox->addItem("0.1");
    }
    if (probe<=100.0)
    {
        scalebox->addItem("0.2");
        scalebox->addItem("0.5");
    }
    if (probe<=500.0)
    {
        scalebox->addItem("1.0");
    }
    scalebox->addItem("2.0");
    scalebox->addItem("5.0");
    scalebox->addItem("10.0");
    if (probe>1.0)
    {
        scalebox->addItem("20.0");
        scalebox->addItem("50.0");
    }
    if (probe>5.0)
      {
         scalebox->addItem("100.0");
      }
    if (probe>10.0)
      {
         scalebox->addItem("200.0");
         scalebox->addItem("500.0");
      }
    if (probe>50.0)
    {
        scalebox->addItem("1000.0");
    }
    if (probe>100.0)
      {
        scalebox->addItem("2000.0");
        scalebox->addItem("5000.0");
      }
    if (probe>500.0)
    {
        scalebox->addItem("10000.0");
    }
    cmd=cmdbase+":SCAL?";
    probe=cmdFloat(cmd);
    int n=0;
    while (scalebox->itemText(n).toFloat()!=probe)
        if (++n>=scalebox->count()) break;
    scalebox->setCurrentIndex(n);

}

void MainWindow::on_updAcq_clicked()  // this function updates the ui from the scope
// called from several ui buttons and also on start up or switching of tabs
{
    bool savestate=nocommands;
    int tmp;
    if (!com.connected()) return;
    setConfig();
    nocommands=true;   // stop ui changes from writing to scope
    ui->acqType->setCurrentIndex(tmp=cmdCharIndex(":ACQ:TYPE?","NAP"));
    ui->acqAvg->setEnabled(tmp==1);
    // detect real or equal time
    com.command(":ACQ:MODE?");
    ui->acqMode->setCurrentIndex(*com.buffer=='R'?0:1);
    com.command(":ACQ:AVER?");
    int idx=0;
    if (com.buffer[1]=='5') idx=7;
    else if (com.buffer[0]=='1')
    {
        if (com.buffer[1]=='2') idx=6; else idx=3;
    }
    else if (com.buffer[1]=='4') idx=5;
    else if (com.buffer[1]=='2') idx=4;
    else if (com.buffer[0]=='8') idx=2;
    else if (com.buffer[0]=='4') idx=1;
    ui->acqAvg->setCurrentIndex(idx);
    com.command(":ACQ:MEMD?");
    ui->acqMem->setChecked(*com.buffer=='L');
    com.command(":ACQ:SAMP?");
    ui->srate->setText(com.buffer);

    on_tmode_currentIndexChanged(ui->tmode->currentIndex());

    // kind of ugly
    com.command(":TIM:SCAL?");
    int decade=QString(com.buffer).right(1).toInt();
    char sign=QString(com.buffer).right(3).left(1)[0].toLatin1();
    char *p=strchr(com.buffer,'.');
    if (p) *p='\0';
    int scale=QString(com.buffer).toInt();
    QString sscale=QString::number(scale);
    switch (decade)
    {
    case 9:
        sscale.append("ns");
        break;
    case 8:
        sscale.append("0ns");
        break;
    case 7:
        sscale.append("00ns");
        break;
    case 6:
        sscale.append("us");
        break;
    case 5:
        sscale.append("0us");
        break;
    case 4:
        sscale.append("00us");
        break;
    case 3:
        sscale.append("ms");
        break;
    case 2:
        sscale.append("0ms");
        break;
    case 1:
        // need to tell + from - here
        if (sign=='+') sscale.append("0s"); else sscale.append("00ms");
        break;
    case 0:
        sscale.append("s");
    }
   ui->hscale->setCurrentIndex(ui->hscale->findText(sscale));
   com.command(":CHAN1:DISP?");
   if (com.buffer[1]=='N') com.buffer[0]='1';   // just in case programming guide is right for old fw
   ui->cdisp1->setChecked(com.buffer[0]=='1');  // programming guide is wrong!
   com.command(":CHAN2:DISP?");
   if (com.buffer[1]=='N') com.buffer[0]='1';   // just in case programming guide is right for old fw
   ui->cdisp2->setChecked(com.buffer[0]=='1');  // programming guide is wrong!
   com.command(":CHAN1:BWL?");
   ui->c1bw->setChecked(com.buffer[1]=='N');
   com.command(":CHAN2:BWL?");
   ui->c2bw->setChecked(com.buffer[1]=='N');
   com.command(":CHAN1:INV?");
   ui->c1inv->setChecked(com.buffer[1]=='N');
   com.command(":CHAN2:INV?");
   ui->c2inv->setChecked(com.buffer[1]=='N');
   com.command(":CHAN1:FILT?");
   ui->c1filt->setChecked(com.buffer[1]=='N');
   com.command(":CHAN2:FILT?");
   ui->c2filt->setChecked(com.buffer[1]=='N');
   setupChannel(1,ui->c1probe,ui->c1vscale);
   setupChannel(2,ui->c2probe,ui->c2vscale);
   com.command(":CHAN1:COUP?");
   ui->c1coup->setCurrentIndex(ui->c1coup->findText(com.buffer));
   com.command(":CHAN2:COUP?");
   ui->c2coup->setCurrentIndex(ui->c2coup->findText(com.buffer));
   float off=config.hoffset;
   // convert to uS
   off*=1000000.0f;
   ui->hoffsetspin->setValue(off);
   off=config.vscale[0];
   ui->c1offspin->setValue(off);
   off=config.vscale[1];
   ui->c2offspin->setValue(off);

   com.command(":TRIG:MODE?");
   int  moden=ui->tmode->findText(com.buffer,static_cast<Qt::MatchFlags>(Qt::MatchFixedString));
   if (moden!=-1)
   {
       ui->tmode->setCurrentIndex(moden);
   }
   else
   {
       qDebug()<<"Unsupported trigger mode "<<com.buffer;
       ui->statusBar->showMessage(QString("Unsupported trigger mode ")+com.buffer);
       ui->tmode->findText(com.buffer,static_cast<Qt::MatchFlags>(Qt::MatchFixedString));
   }
   // since nocommands is set, this will just update ui
   on_tmode_currentIndexChanged(ui->tmode->currentIndex());
   QString cmd,cmdbase=":TRIG:";
   cmdbase=cmdbase+ui->tmode->currentText();
   cmd=cmdbase+":SOUR?";
   command(cmd);
   if (com.buffer[2]=='1') strcpy(com.buffer,"CHAN1");
   if (com.buffer[2]=='2') strcpy(com.buffer,"CHAN2");
   ui->tsource->setCurrentIndex(ui->tsource->findText(com.buffer,Qt::MatchFlags(Qt::MatchFixedString)));
   cmd=cmdbase+":SWE?";
   command(cmd); // scope doesn't seem to care what trigger type is active
   ui->tsweep->setCurrentIndex(ui->tsweep->findText(com.buffer,Qt::MatchFlags(Qt::MatchFixedString)));

   float hold=com.cmdFloat(":TRIG:HOLD?");
   hold*=1000000.0f;  // convert to uS
   ui->tholdoff->setValue(hold);

   // TODO: Need to read current scale to set range. Complex because range depends on trigger source scale
   cmd=cmdbase+":LEV?";
   if (ui->tmode->currentText()!="Slope")
   {
    off=cmdFloat(cmd);
    ui->tlevel->setValue(off);
   }
    cmd=cmdbase+=":COUP?";
    command(cmd);
    ui->tcouple->setCurrentIndex(ui->tcouple->findText(com.buffer,Qt::MatchFlags(Qt::MatchFixedString)));

    com.command(":TRIG:EDGE:SLOP?");
    ui->tposneg->setCurrentIndex(*com.buffer=='P'?0:1);
    off=com.cmdFloat(":TRIG:EDGE:SENS?");
    ui->tedgesense->setValue(off);
    off=com.cmdFloat(":TRIG:PULS:SENS?");
    ui->tpulsesense->setValue(off);
    off=com.cmdFloat(":TRIG:PULS:WIDT?");
    off*=1000000.0f;
    ui->tpulswid->setValue(off);
    com.command(":TRIG:PULS:MODE?");
    cmd=com.buffer;
    for (tmp=0;tmp<ui->tpulsemode->count();tmp++)
    {
        if (ui->tpulsemode->itemText(tmp).remove(' ')==cmd)
        {
            ui->tpulsemode->setCurrentIndex(tmp);
            break;
        }
    }

    off=com.cmdFloat(":TRIG:SLOP:TIME?");
    off*=1000000.0f;
    ui->tslopetime->setValue(off);
    off=com.cmdFloat(":TRIG:SLOP:SENS?");
    ui->tslopesense->setValue(off);
    com.command(":TRIG:SLOP:MODE?");  // oddly this returns spaces but can't have them in the command
    ui->tslopemode->setCurrentIndex(ui->tslopemode->findText(com.buffer,Qt::MatchFlags(Qt::MatchFixedString)));
    com.command(":TRIG:SLOP:WIND?");
    ui->tslopewin->setCurrentIndex(ui->tslopewin->findText(com.buffer,Qt::MatchFlags(Qt::MatchFixedString)));
    off=com.cmdFloat(":TRIG:SLOP:LEVA?");
    ui->tslopea->setValue(off);
    off=com.cmdFloat(":TRIG:SLOP:LEVB?");
    ui->tslopeb->setValue(off);



    com.command(":MATH:DISP?");
    ui->mathdisp->setChecked(com.buffer[1]=='N');
    com.command(":MATH:OPER?");
    ui->mathsel->setCurrentIndex(ui->mathsel->findText(com.buffer,Qt::MatchFlags(Qt::MatchFixedString)));



   nocommands=savestate;

}

void MainWindow::on_acqType_currentIndexChanged(int index)
{
    QString cmd=":ACQ:TYPE ";
    if (!com.connected()||nocommands) return;
    switch (index)
    {
    case 0:
        cmd+="NORM";
        break;
    case 1:
        cmd+="AVER";
        break;
    case 2:
        cmd+="PEAK";
        break;
    }
    ui->acqAvg->setEnabled(index==1);
    command(cmd);
}

void MainWindow::on_acqMode_currentIndexChanged(int index)
{
    QString cmd=":ACQ:MODE ";
    if (!com.connected()||nocommands) return;
    switch (index)
    {
    case 0:
        cmd+="RTIM";
        break;
    case 1:
        cmd+="ETIM";
        break;
    }
    command(cmd);

}

void MainWindow::on_acqAvg_currentIndexChanged(int index)
{
    QString cmd=":ACQ:AVER ";
    if (!com.connected()||nocommands) return;
    QString num=QString::number(1<<(index+1));
    cmd+=num;
    command(cmd);
}


void MainWindow::on_acqMem_clicked()
{
    QString cmd=":ACQ:MEMD ";
    if (!com.connected()) return;
    if (ui->acqMem->isChecked()) cmd+="LONG"; else cmd+="NORM";
    command(cmd);
}



void MainWindow::on_hscale_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!com.connected()||nocommands) return;
    com.command((":TIM:SCAL " + ui->hscale->currentText()).toLatin1());
}

void MainWindow::on_cdisp1_clicked()
{
    if (!com.connected()) return;
    QString cmd=":CHAN1:DISP ";
    if (ui->cdisp1->checkState()) cmd+="ON"; else cmd+="OFF";
    command(cmd);
}

void MainWindow::on_updAcq_2_clicked()
{
    on_updAcq_clicked();
}

void MainWindow::on_cdisp2_clicked()
{
    if (!com.connected()) return;
    QString cmd=":CHAN2:DISP ";
    if (ui->cdisp2->checkState()) cmd+="ON"; else cmd+="OFF";
    command(cmd);

}

void MainWindow::on_c1bw_clicked()
{
    if (!com.connected()) return;
    QString cmd=":CHAN1:BWL ";
    if (ui->c1bw->checkState()) cmd+="ON"; else cmd+="OFF";
    command(cmd);

}

void MainWindow::on_c2bw_clicked()
{
    if (!com.connected()) return;
    QString cmd=":CHAN2:BWL ";
    if (ui->c2bw->checkState()) cmd+="ON"; else cmd+="OFF";
    command(cmd);

}

void MainWindow::on_c1inv_clicked()
{
    if (!com.connected()) return;
    QString cmd=":CHAN1:INV ";
    if (ui->c1inv->checkState()) cmd+="ON"; else cmd+="OFF";
    command(cmd);

}

void MainWindow::on_c2inv_clicked()
{
    if (!com.connected()) return;
    QString cmd=":CHAN2:INV ";
    if (ui->c2inv->checkState()) cmd+="ON"; else cmd+="OFF";
    command(cmd);

}

void MainWindow::on_c1filt_clicked()
{
    if (!com.connected()) return;
    QString cmd=":CHAN1:FILT ";
    if (ui->c1filt->checkState()) cmd+="ON"; else cmd+="OFF";
    command(cmd);

}

void MainWindow::on_c2filt_clicked()
{
    if (!com.connected()) return;
    QString cmd=":CHAN2:FILT ";
    if (ui->c2filt->checkState()) cmd+="ON"; else cmd+="OFF";
    command(cmd);

}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index);
    if (!com.connected()) return;
    on_updAcq_clicked();  // refresh from scope when switching tabs
}

void MainWindow::on_c1probe_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!com.connected()||nocommands) return;
    QString cmd=":CHAN1:PROB ";
    cmd+=ui->c1probe->currentText().remove('X');
    command(cmd);

}

void MainWindow::on_c2probe_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!com.connected()||nocommands) return;
    QString cmd=":CHAN2:PROB ";
    cmd+=ui->c2probe->currentText().remove('X');
    command(cmd);


}

void MainWindow::on_c1coup_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!com.connected()||nocommands) return;
    QString cmd=":CHAN1:COUP ";
    cmd+=ui->c1coup->currentText();
    command(cmd);
}

void MainWindow::on_c2coup_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!com.connected()||nocommands) return;
    QString cmd=":CHAN2:COUP ";
    cmd+=ui->c2coup->currentText();
    command(cmd);

}


void MainWindow::on_hoffsetspin_valueChanged(double arg1)
{
    float arg=arg1/1000000.0f;  // convert to seconds
    if (!com.connected()||nocommands) return;
    QString cmd=QString(":TIM:OFFS ")+QString::number(arg);
    command(cmd);
}

void MainWindow::on_hoffincr_valueChanged(double arg1)
{
    ui->hoffsetspin->setSingleStep(arg1);
    QSettings set;
    set.setValue("Options/hincr",arg1);

}

void MainWindow::on_c1offspin_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    if (!com.connected()||nocommands) return;
    QString cmd=":CHAN1:OFFS ";
    cmd+=QString::number(ui->c1offspin->value());
    command(cmd);
}

void MainWindow::on_c2offspin_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    if (!com.connected()||nocommands) return;
    QString cmd=":CHAN2:OFFS ";
    cmd+=QString::number(ui->c2offspin->value());
    command(cmd);

}

void MainWindow::on_c1vscale_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!com.connected()||nocommands) return;
    QString cmd=":CHAN1:SCAL "+ui->c1vscale->currentText();
    command(cmd);
}

void MainWindow::on_c2vscale_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!com.connected()||nocommands) return;
    QString cmd=":CHAN2:SCAL "+ui->c2vscale->currentText();
    command(cmd);

}

void MainWindow::on_unlockBtn_clicked()
{
    QSettings set;
    set.setValue("Options/Unlock",ui->unlockBtn->checkState()==Qt::Checked);
}

void MainWindow::on_hoffclear_clicked()
{
    if (!com.connected()) return;
    ui->hoffsetspin->setValue(0.0);
}



void MainWindow::on_action_About_triggered()
{
    QMessageBox::information(this,tr("About"),tr("qrigol V0.1 Copyright (c) 2015 by Al Williams http://www.awce.com.\n"
                                                 "This program comes with ABSOLUTELY NO WARRANTY. "
                                                 "This is free software, and you are welcome to redistribute it under certain conditions.\n"
                                                 "See the file COPYING for more information."
                                                 ));
}



void MainWindow::on_updAcq_3_clicked()
{
    on_updAcq_clicked();
}

void MainWindow::on_tmode_currentIndexChanged(int index)
{
    QString cmd=":TRIG:MODE ";
    ui->tboxedge->setVisible(index==0);
    ui->tboxpulse->setVisible(index==1);
    ui->tboxslope->setVisible(index==2);
    ui->tsource->clear();
    ui->tsource->addItem("CHAN1");
    ui->tsource->addItem("CHAN2");
    ui->tsource->addItem("EXT");
    if (index==0) ui->tsource->addItem("ACLINE");
    ui->tcouple->setEnabled(index<3);
    if (!com.connected()||nocommands) return;
    cmd+=ui->tmode->currentText();
    command(cmd);
    QString cmdbase=":TRIG:";
    cmdbase=cmdbase+ui->tmode->currentText();
    cmd=cmdbase+":SOUR?";
    command(cmd);
    ui->tsource->setCurrentIndex(ui->tsource->findText(com.buffer,Qt::MatchFlags(Qt::MatchFixedString)));
    cmd=cmdbase+":SWE?";
    command(cmd);
    ui->tsweep->setCurrentIndex(ui->tsweep->findText(com.buffer,Qt::MatchFlags(Qt::MatchFixedString)));
// This seems like a good idea, but the scope doesn't shift gears fast enough
// For example, going to slope mode where the trigger channel is invalid and back to edge does not
// update the channel source correctly
//    on_updAcq_clicked();

}

void MainWindow::on_tsweep_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    QString cmd=":TRIG:";
    if (!com.connected()||nocommands) return;
    cmd+=ui->tmode->currentText()+":SWE ";
    cmd+=ui->tsweep->currentText();
    command(cmd);
}

void MainWindow::on_tsource_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    QString cmd=":TRIG:";
    if (!com.connected()||nocommands) return;
    cmd+=ui->tmode->currentText()+":SOUR "+ui->tsource->currentText();
    command(cmd);
}

void MainWindow::on_tholdoff_valueChanged(double arg1)
{
    QString cmd;
    if (!com.connected()||nocommands) return;
    arg1/=1000000.0f; // convert to seconds
    cmd=":TRIG:HOLD ";
    cmd+=QString::number(arg1,'f');
    command(cmd);
}

void MainWindow::on_tfifty_clicked()
{
    if (!com.connected()) return;
    com.command(":TRIG%50");
}

void MainWindow::on_tforce_clicked()
{
    if (!com.connected()) return;
    com.command(":FORC");
}

void MainWindow::on_tlevel_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    QString cmd=":TRIG:";
    if (!com.connected()||nocommands) return;
    cmd+=ui->tmode->currentText()+":LEV " + QString::number(ui->tlevel->value());
    command(cmd);
}

void MainWindow::on_tcouple_currentIndexChanged(const QString &arg1)
{
    QString cmd=":TRIG:";
    if (!com.connected()||nocommands) return;
    cmd+=ui->tmode->currentText()+":COUP " +arg1;
    command(cmd);
}

void MainWindow::on_tposneg_currentIndexChanged(int index)
{
    QString cmd;
    if (!com.connected()||nocommands) return;
    cmd=":TRIG:EDGE:SLOP ";
    if (index) cmd+="NEG"; else cmd+="POS";
    command(cmd);
}

void MainWindow::on_tedgesense_valueChanged(double arg1)
{
    QString cmd;
    if (!com.connected()||nocommands) return;
    cmd=":TRIG:EDGE:SENS ";
    cmd+=QString::number(arg1);
    command(cmd);
}

void MainWindow::on_tpulsesense_valueChanged(double arg1)
{
    QString cmd;
    if (!com.connected()||nocommands) return;
    cmd=":TRIG:PULS:SENS ";
    cmd+=QString::number(arg1);
    command(cmd);
}

void MainWindow::on_tpulswid_valueChanged(double arg1)
{
    QString cmd;
    if (!com.connected()||nocommands) return;
    arg1/=1000000.0f; // convert to seconds
    cmd=":TRIG:PULS:WIDT ";
    cmd+=QString::number(arg1,'f');
    command(cmd);
}



void MainWindow::on_tpulsemode_currentIndexChanged(const QString &arg1)
{
    QString cmd=":TRIG:PULS:MODE ", arg=arg1;
    arg=arg.remove(' ');
    cmd+=arg;
    if (!com.connected()||nocommands) return;
    command(cmd);
}



void MainWindow::setConfig(void)
{
    QString cmdbase,cmd;
    config.hscale=com.cmdFloat(":TIM:SCAL?");
    config.hoffset=com.cmdFloat(":TIM:OFFS?");

    config.vscale[0]=com.cmdFloat(":CHAN1:SCAL?");
    config.voffset[0]=com.cmdFloat(":CHAN1:OFFS?");
    config.vscale[1]=com.cmdFloat(":CHAN2:SCAL?");
    config.voffset[1]=com.cmdFloat(":CHAN2:OFFS?");
    config.srate=com.cmdFloat(":ACQ:SAMP? CHAN1");
    config.deltat=1.0f/config.srate;
    config.set=true;
}

int MainWindow::convertbuf(int chan, const QString &cmd, bool raw)
{
    int i,size;
    if (!config.set) setConfig();
    chan--;
    size=command(cmd);
    if (size==-1) return -1;
    for (i=0;i<size;i++)
    {
        unsigned int rawdata=(unsigned char)(com.buffer[i]);
        chandata[chan][i]=raw?((double)rawdata):((double)(125.0-(double)rawdata)/25.0f);
        if (!raw)
        {
            chandata[chan][i]*=config.vscale[chan];
            chandata[chan][i]-=config.voffset[chan];
        }
    }
  chansize=size;
  return size;

}



void MainWindow::on_mlogfileselect_clicked()
{
    QString fn=QFileDialog::getSaveFileName(this,"Select log file to create or append to",QString(),"Comma Separate Value Files (*.csv);;All files (*.*)",0,QFileDialog::DontConfirmOverwrite);
    if (fn.isEmpty()) return;
    ui->mlogfilename->setText(fn);

}

void MainWindow::on_measLogEnable_clicked()
{
    if (ui->measLogEnable->isChecked())
    {
        if (ui->mlogfilename->text().isEmpty())
        {
            ui->measLogEnable->setChecked(false);
            QMessageBox bx;
            bx.setText("Select a file name before enabling logging.");
            bx.exec();
            return;
        }
        mlog=new QFile(ui->mlogfilename->text());
        mlogworker.prep(mlog,ui->measlogheader->isChecked(),ui->updTimeScale->currentIndex()>0,ui->mlogstop->isChecked()?ui->mlogrepeat->value():-1);
        mlogworker.start();
    }
    else
    {
        mlogworker.terminate();
        mlog->close();
    }
}


void MainWindow::on_mathdisp_clicked()
{
    if (!com.connected()) return;
    QString cmd=":MATH:DISP ";
    cmd+=ui->mathdisp->isChecked()?"ON":"OFF";
    command(cmd);

}

void MainWindow::on_mathsel_currentIndexChanged(const QString &arg1)
{
    QString cmd=":MATH:OPER ";
    cmd+=arg1;
    if (!com.connected()||nocommands) return;
    command(cmd);

}

/* Still need to do the slope commands */
void MainWindow::on_tslopemode_currentIndexChanged(const QString &arg1)
{
    // Needs to have no spaces
    if (!com.connected()||nocommands) return;
    QString cmd,arg=arg1;
    arg=arg.remove(' ');
    cmd=":TRIG:SLOP:MODE "+arg;
    command(cmd);

}

void MainWindow::on_tslopewin_currentIndexChanged(const QString &arg1)
{
    // need to convert P_WIN_A to PA, P_WIN_AB to PAB etc.
    if (!com.connected()||nocommands) return;
    QString cmd,arg=arg1;
    arg=arg.replace("_WIN_","");
    cmd=":TRIG:SLOP:WIND "+arg;
    command(cmd);

}

void MainWindow::on_tslopetime_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    // convert uS
    if (!com.connected()||nocommands) return;
    QString cmd=":TRIG:SLOP:TIME "+QString::number(ui->tslopetime->value()/1000000.0f);
    command(cmd);

}

void MainWindow::on_tslopea_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    if (!com.connected()||nocommands) return;
    QString cmd=":TRIG:SLOP:LEVA "+QString::number(ui->tslopea->value());
    command(cmd);

}

void MainWindow::on_tslopesense_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    if (!com.connected()||nocommands) return;
    QString cmd=":TRIG:SLOP:SENS "+QString::number(ui->tslopesense->value());
    command(cmd);
}

void MainWindow::on_tslopeb_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    if (!com.connected()||nocommands) return;
    QString cmd=":TRIG:SLOP:LEVB "+QString::number(ui->tslopeb->value());
    command(cmd);

}

int MainWindow::prepExport(bool c1, bool c2)
{
    bool running=false;

    // make sure connected
    if (!com.connected()) return -1;
    // we know we will be stopped so we are going to get
    // either 8K/16K (based on display) or 512K/1M based on long memory mode
    //
    com.command(":TRIG:STAT?");
    running=com.buffer[0]!='S';
    if (running)
    {
         ui->cdisp1->setChecked(c1);
         on_cdisp1_clicked();
         ui->cdisp2->setChecked(c2);
         on_cdisp2_clicked();
    }
    if ((c1 && !ui->cdisp1->isChecked()) || (c2 && !ui->cdisp2->isChecked()))
    {
       QMessageBox bx;
       bx.setText("Channel not enabled and instrument in stop mode");
       bx.exec();
       return -1;
    }

    if (c1 && !ui->cdisp1->isChecked())
    {
        // if we are running we should enable chan1
        // if we are already stopped, that won't help any
        if (running)
        {
            com.command("CHAN1:DISP ON");
            ui->cdisp1->setChecked(true);
        } else
        {
            QMessageBox bx;
            bx.setText("Channel 1 not enabled.");
            bx.exec();
            return -1;
        }

    }

    // make sure one source selected
    if ((!c1)&&(!c2))
    {
        QMessageBox bx;
        bx.setText("You must select at least one source.");
        bx.exec();
        return -1;
    }
   return 0;
}

// This function handles exporting for both CSV and plotting
int MainWindow::exportEngine(bool dotime, bool c1, bool c2, bool wheader, bool wconfig, bool raw, QFile *file)
{
    int asize,wsize;


    // Now we can really compute the size
    com.command(":WAV:POIN:MODE MAX");
    asize=ui->acqMem->isChecked()?524288:8192;
    if ((c1&&!c2) || (c2&&!c1)) asize*=2;   // one channel is double

    // allocate buffers
    if (c1) chandata[0]=new double[asize];
    if (c2) chandata[1]=new double[asize];
    if (((c1&&!chandata[0]) || (c2&&!chandata[1])))
    {
        QMessageBox bx;
        bx.setText("Sorry, insufficient memory.");
        bx.exec();
        return -1;

    }
    // Stop instrument
    com.command(":STOP");   // only way to be sure we sync chan1 and chan2 that I can tell
    // however, it takes a bit for it to "settle" If you get a short sample, just run it again
    // since the scope is stopped it will be ok
    waitForStop();
    setConfig();
    // Acquire each channel (as requested)
    if (c1)
    {
        wsize=convertbuf(1,":WAV:DATA? CHAN1",raw);
        if (wsize<0)
        {
            QMessageBox bx;
            bx.setText("Channel 1 data acquisition failure\n");
            bx.exec();
        }
    }
    if (c2)
    {
        wsize=convertbuf(2,":WAV:DATA? CHAN2",raw);
        if (wsize<0)
        {
            QMessageBox bx;
            bx.setText("Channel 2 data acquisition failure\n");
            bx.exec();
        }
    }

    // Get file name
    if (file==NULL)
    {
        QString fn=QFileDialog::getSaveFileName(this,"Select log file to create or append to",QString(),"Comma Separate Value Files (*.csv);;All files (*.*)");
        if (fn.isEmpty()) return -1;
        file=new QFile(fn);
        if (!file->open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox bx;
            bx.setText("Can't open file for writing!");
            return -1;
            return -1;
        }

    }
    // if requested, write out config
    if (wconfig)
    {
        QString cline;
        cline=",,,HSCALE,HOFFSET,VSCALE 1,VSCALE 2,VOFFSET 1, VOFFSET 2, SAMP RATE, DELTA T,SAMPLE COUNT\n";
        file->write(cline.toLatin1());
        cline=",,,";
        cline+=QString::number(config.hscale)+",";
        cline+=QString::number(config.hoffset)+",";
        cline+=QString::number(config.vscale[0])+",";
        cline+=QString::number(config.vscale[1])+",";
        cline+=QString::number(config.voffset[0])+",";
        cline+=QString::number(config.voffset[1])+",";
        cline+=QString::number(config.srate)+",";
        cline+=QString::number(config.deltat)+",";
        cline+=QString::number(wsize)+"\n";
        file->write(cline.toLatin1());
        file->write("\n");
    }
    // Write data out
    if (wheader)
    {
        // write header
        if (dotime) file->write("T,");
        if (c1) file->write("CHAN1");
        if (c1 && c2) file->write(",");
        if (c2) file->write("CHAN2");
        file->write("\n");
    }
    float t=0.0;
    for (int idx=0;idx<wsize;idx++)
    {
        QString item;
        if (dotime)
        {
            item=QString::number(t);
            t+=config.deltat;
            item+=",";
        }
        if (c1)
        {
            item+=QString::number(chandata[0][idx]);
            if (c2) item+=",";
        }
        if (c2)
        {
            item+=QString::number(chandata[1][idx]);
        }
     item+="\n";
     file->write(item.toLatin1());
    }
    // Close files
    file->close();
    // Free buffers
    if (c1) delete [] chandata[0];
    if (c2) delete [] chandata[1];
    return 0;
}



void MainWindow::on_action_Diagnostic_triggered()
{
    bool ok;
    if (!com.connected()) return;
    QString cmd=QInputDialog::getText(this,"Enter Diagnostic Command","Command:",QLineEdit::Normal,"",&ok);
    if (!ok || cmd.isEmpty()) return;
    command(cmd);
    if (cmd.right(1)[0]!='?') return;
    QMessageBox bx;
    bx.setText(com.buffer);
    bx.exec();
}

void MainWindow::on_wavplot_clicked()
{
    PlotDialog *dlg=new PlotDialog(this);
    QString cmd,script;

    if (prepExport(ui->wavec1->isChecked(),ui->wavec2->isChecked())) return;

    bool c1=ui->wavec1->isChecked();
    bool c2=ui->wavec2->isChecked();
    if (!dlg->exec()) return;
    QTemporaryFile *file=new QTemporaryFile; // don't know the lifetime of this so...
    QTemporaryFile *scriptfile=new QTemporaryFile;
    file->setAutoRemove(false);
    scriptfile->setAutoRemove(false);
    scriptfile->open();
    file->open();
    script=dlg->script;
    script=script.replace("{FILE}",file->fileName());  // does not work unless file is open!
    scriptfile->write(script.toLatin1());
    scriptfile->close();
    // generate temporary file
    exportEngine(true,c1,c2,true,false,false,file);
    // execute command
    cmd=dlg->command;
    cmd=cmd.replace("{SCRIPT}",scriptfile->fileName());
    cmd=cmd.replace("{FILE}",file->fileName());
    QProcess proc;
    proc.startDetached(cmd);
}

void MainWindow::on_wavecsv_clicked()
{
    if (prepExport(ui->wavec1->isChecked(),ui->wavec2->isChecked())) return;
    bool dotime=ui->wavetimeopt->isChecked();
    bool c1=ui->wavec1->isChecked();
    bool c2=ui->wavec2->isChecked();
    bool wheader=ui->wavehead->isChecked();
    bool wconfig=ui->wavesavecfg->isChecked();
    bool raw=ui->waveraw->isChecked();

    if (exportEngine(dotime,c1,c2,wheader,wconfig,raw)==0)
    {
        QMessageBox done;
        QString msg;
        msg="Wrote ";
        msg+=QString::number(chansize)+" records";
        done.setText(msg);
        done.exec();
    }
}

void MainWindow::on_actionRun_Stop_triggered()
{
    if (com.connected())
       on_rsButton_clicked();
    else ui->actionRun_Stop->setChecked(false);
}


void MainWindow::on_actionConnect_triggered()
{
    ui->connectButton->setChecked(!com.connected());
    on_connectButton_clicked();

}
