/*
 *
 *  This program is Copyright (c) 2015 by Al Williams al.williams@awce.com
 *  All rights reserved.
 *
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSettings>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include "helpdialog.h"
#include "unistd.h"

// TODO: Read scales
// I think this is done for chanXoffset
// Needs to also be done for :TIM:OFFS, CHANX:SCAL (if we didn't already) :TRIG:SLOP:LEVA/B :TRIG:MODE:LEV
// TODO: The plot example UI probably doesn't work well with a keyboard

void MainWindow::on_action_About_triggered()
{
#if 0
    QMessageBox::information(this,tr("About"),tr("qrigol V0.2 Copyright (c) 2015 by Al Williams http://www.awce.com.\n"
                                                 "This program comes with ABSOLUTELY NO WARRANTY. "
                                                 "This is free software, and you are welcome to redistribute it under certain conditions.\n"
                                                 "See the file COPYING for more information."
                                                 ));
#else
    if (!helpdlg) helpdlg=new HelpDialog(this);
    helpdlg->showRequest("about");
#endif
}
void MainWindow::restoreSavedSettings(void)
{
    QSettings set;
    ui->deviceName->setText(set.value("Options/Device","/dev/usbtmc0").toString());
    ui->unlockBtn->setChecked(set.value("Options/Unlock",false).toBool());
    ui->hoffincr->setValue(set.value("Options/hincr",100.0).toFloat());
    ui->exportFmt->setCurrentIndex(set.value("Options/exportselect",0).toInt());
    ui->wavec1->setChecked(set.value("Options/exportc1",true).toBool());
    ui->wavec2->setChecked(set.value("Options/exportc2",true).toBool());
    ui->wavehead->setChecked(set.value("Options/exporthead",true).toBool());
    ui->wavesavecfg->setChecked(set.value("Options/exporthead",true).toBool());
    ui->waveraw->setChecked(set.value("Options/exportraw",false).toBool());
    ui->logicThresh->setValue(set.value("Options/exportthresh",2.5f).toFloat());
    this->setGeometry((QRect)set.value("Options/mainwinpos",QRect(0,0,640,480)).toRect());

}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scope(this)
{
    helpdlg=NULL;
    ui->setupUi(this);
    nocommands=false;  // used to inhibit commands while updating UI
// Force current tab
    ui->tabWidget->setCurrentIndex(0);
    ui->statusBar->showMessage("Disconnected");
// recall device name and other settings
    restoreSavedSettings();
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
    QSettings set;
    set.setValue("Options/mainwinpos",this->geometry());
    if (scope.connected()) scope.close();
    delete ui;
}



void MainWindow::chanDisp(int chan, bool disp)
{
    if (chan==1)
    {
        ui->cdisp1->setChecked(disp);
        on_cdisp1_clicked();
    }
    else if (chan==2)
    {
        ui->cdisp2->setChecked(disp);
        on_cdisp2_clicked();
    }
    usleep(250000);
}


void MainWindow::on_uiUpdate()  // periodic update of scope status -- this makes it impossible to unlock so we need a supression
{
    if (!scope.connected()) return;
    int status=scope.trigStatus();

    ui->actionRun_Stop->setChecked(status=='S');
    ui->rsButton->setStyleSheet(status!='S'?"background: green;":"background: red;");
    if (ui->unlockBtn->isChecked()) scope.unlock();
    if (mlogworker.isFinished()) ui->measLogEnable->setChecked(false);
    ui->actionConnect->setChecked(scope.connected());



}

void MainWindow::on_connectButton_clicked()
{

    if (!scope.connected())
    {
        ui->connectButton->setChecked(false);
        scope.open(ui->deviceName->text());
        if (!scope.connected())
        {
            QMessageBox bx;
            bx.setText("Can't open device");
            ui->statusBar->showMessage("Can't open device");
            bx.exec();
            return;
        }
        QString id=scope.id();
        if (id.isEmpty())
        {
            ui->idstring->setText("No response");
            ui->statusBar->showMessage("No response");
        }
        else
        {
            QSettings set;
            ui->idstring->setText(id);
            set.setValue("Options/Device",ui->deviceName->text());
            ui->statusBar->showMessage(id);
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
        scope.close();
        ui->idstring->clear();
        ui->connectButton->setText("&Connect");



    }

}

void MainWindow::on_rsButton_clicked()
{
    bool rs;
    if (!scope.connected()) return;
    rs=scope.trigStatus()!='S';

    if (rs)
    {
        rs=0;
        scope.stop();
        ui->rsButton->setStyleSheet("background: red;");
        ui->hoffsetspin->setMinimum(-500000000.0f);
        ui->hoffsetspin->setMaximum(500000000.0f);
    }
    else
    {
        //float hmax=1.0f;
        rs=1;
        scope.run();
        ui->rsButton->setStyleSheet("background: green;");
       // ui->hoffsetspin->setMinimum(0.0f);
// TODO: Need to read current scale
       // ui->hoffincr->setMaximum(hmax);

    }

}


void MainWindow::on_measUpdate_clicked()
{
    if (!scope.connected()) return;
    ui->vpp1->setText(QString::number(lastMeasure[0][0]=scope.cmdFloat(":MEAS:VPP? CHAN1")));
    ui->vmax1->setText(QString::number(lastMeasure[0][1]=scope.cmdFloat(":MEAS:VMAX? CHAN1")));
    ui->vmin1->setText(QString::number(lastMeasure[0][2]=scope.cmdFloat(":MEAS:VMIN? CHAN1")));
    ui->vavg1->setText(QString::number(lastMeasure[0][3]=scope.cmdFloat(":MEAS:VAV? CHAN1")));
    ui->vamp1->setText(QString::number(lastMeasure[0][4]=scope.cmdFloat(":MEAS:VAMP? CHAN1")));
    ui->vrms1->setText(QString::number(lastMeasure[0][5]=scope.cmdFloat(":MEAS:VRMS? CHAN1")));
    ui->vtop1->setText(QString::number(lastMeasure[0][6]=scope.cmdFloat(":MEAS:VTOP? CHAN1")));
    ui->vbas1->setText(QString::number(lastMeasure[0][7]=scope.cmdFloat(":MEAS:VBAS? CHAN1")));
    ui->pwid1->setText(QString::number(lastMeasure[0][8]=scope.cmdFloat(":MEAS:PWID? CHAN1")*1000.0));
    ui->nwid1->setText(QString::number(lastMeasure[0][9]=scope.cmdFloat(":MEAS:NWID? CHAN1")*1000.0));
    ui->freq1->setText(QString::number(lastMeasure[0][10]=scope.cmdFloat(":MEAS:FREQ? CHAN1")));
    ui->per1->setText(QString::number(lastMeasure[0][11]=scope.cmdFloat(":MEAS:PER? CHAN1")*1000.0));
    ui->over1->setText(QString::number(lastMeasure[0][12]=scope.cmdFloat(":MEAS:OVER? CHAN1")*100.0));
    ui->pres1->setText(QString::number(lastMeasure[0][13]=scope.cmdFloat(":MEAS:PRES? CHAN1")*100.0));
    ui->rise1->setText("<" + QString::number(lastMeasure[0][14]=scope.cmdFloatlt(":MEAS:RIS? CHAN1")*1E6));
    ui->fall1->setText("<" + QString::number(lastMeasure[0][15]=scope.cmdFloatlt(":MEAS:FALL? CHAN1")*1E6));
    ui->pdut1->setText(QString::number(lastMeasure[0][16]=scope.cmdFloat(":MEAS:PDUT? CHAN1")*100.0));
    ui->ndut1->setText(QString::number(lastMeasure[0][17]=scope.cmdFloat(":MEAS:NDUT? CHAN1")*100.0));
    ui->pdel1->setText(QString::number(lastMeasure[0][18]=scope.cmdFloat(":MEAS:PDEL? CHAN1")*1000.0));
    ui->ndel1->setText(QString::number(lastMeasure[0][19]=scope.cmdFloat(":MEAS:NDEL? CHAN1")*1000.0));


    ui->vpp2->setText(QString::number(lastMeasure[1][0]=scope.cmdFloat(":MEAS:VPP? CHAN2")));
    ui->vmax2->setText(QString::number(lastMeasure[1][1]=scope.cmdFloat(":MEAS:VMAX? CHAN2")));
    ui->vmin2->setText(QString::number(lastMeasure[1][2]=scope.cmdFloat(":MEAS:VMIN? CHAN2")));
    ui->vavg2->setText(QString::number(lastMeasure[1][3]=scope.cmdFloat(":MEAS:VAV? CHAN2")));
    ui->vamp2->setText(QString::number(lastMeasure[1][4]=scope.cmdFloat(":MEAS:VAMP? CHAN2")));
    ui->vrms2->setText(QString::number(lastMeasure[1][5]=scope.cmdFloat(":MEAS:VRMS? CHAN2")));
    ui->vtop2->setText(QString::number(lastMeasure[1][6]=scope.cmdFloat(":MEAS:VTOP? CHAN2")));
    ui->vbas2->setText(QString::number(lastMeasure[1][7]=scope.cmdFloat(":MEAS:VBAS? CHAN2")));
    ui->pwid2->setText(QString::number(lastMeasure[1][8]=scope.cmdFloat(":MEAS:PWID? CHAN2")*1000.0));
    ui->nwid2->setText(QString::number(lastMeasure[1][9]=scope.cmdFloat(":MEAS:NWID? CHAN2")*1000.0));
    ui->freq2->setText(QString::number(lastMeasure[1][10]=scope.cmdFloat(":MEAS:FREQ? CHAN2")));
    ui->per2->setText(QString::number(lastMeasure[1][11]=scope.cmdFloat(":MEAS:PER? CHAN2")*1000.0));
    ui->over2->setText(QString::number(lastMeasure[1][12]=scope.cmdFloat(":MEAS:OVER? CHAN2")*100.0));
    ui->pres2->setText(QString::number(lastMeasure[1][13]=scope.cmdFloat(":MEAS:PRES? CHAN2")*100.0));
    ui->rise2->setText("<" + QString::number(lastMeasure[1][14]=scope.cmdFloatlt(":MEAS:RIS? CHAN2")*1E6));
    ui->fall2->setText("<" + QString::number(lastMeasure[1][15]=scope.cmdFloatlt(":MEAS:FALL? CHAN2")*1E6));
    ui->pdut2->setText(QString::number(lastMeasure[1][16]=scope.cmdFloat(":MEAS:PDUT? CHAN2")*100.0));
    ui->ndut2->setText(QString::number(lastMeasure[1][17]=scope.cmdFloat(":MEAS:NDUT? CHAN2")*100.0));
    ui->pdel2->setText(QString::number(lastMeasure[1][18]=scope.cmdFloat(":MEAS:PDEL? CHAN2")*1000.0));
    ui->ndel2->setText(QString::number(lastMeasure[1][19]=scope.cmdFloat(":MEAS:NDEL? CHAN2")*1000.0));

    if (ui->measLogEnable->isChecked())
    {
        mlogworker.inuse.lock();
        std::copy(&lastMeasure[0][0],&lastMeasure[0][0]+2*20,&mlogworker.data[0][0]);
// If std::copy didn't work, this is what used to be here
//        for (int i=0;i<2;i++)  // should use std::copy (std::copy(&array[0][0],&array[0][0]+rows*columns,&dest[0][0]);
//            for (int j=0;j<20;j++) mlogworker.data[i][j]=lastMeasure[i][j];
        mlogworker.sample.release();
        mlogworker.inuse.unlock();
        capcount++;
        ui->captureCount->setText(QString::number(capcount)+" captures logged");
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
    if (!scope.connected()) return;
    scope.command(":HARD");
}



void MainWindow::setupChannel(int ch,QComboBox *probebox,QComboBox *scalebox,QDoubleSpinBox *coffset)
{
    QString cmdbase=":CHAN";
    QString cmd;
    cmdbase+=QString::number(ch);
    cmd=cmdbase+":PROB?";
//    printf("DEBUG: cmd=%s\n",cmd.toLatin1().data());
    float probe=scope.cmdFloat(cmd);
    probebox->setCurrentIndex(probebox->findText(QString::number((int)probe)+"X"));
//    printf("DEBUG: %f %d\n",probe, (int)probe);
//    QString dbg=QString::number((int)probe)+"X";
//    printf("DEBUG: %s\n",(char *)dbg.toLatin1().data());
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
    probe=scope.cmdFloat(cmd);
    // here probe equals the current scale which we should use to
    // set the CHANnOffset range too
    if (probe<0.250)
    {
        coffset->setMinimum(-2.0);
        coffset->setMaximum(2.0);
    }
    else
    {
        coffset->setMinimum(-40.0);
        coffset->setMaximum(40.0);
    }
/*
    if (ch==?? trig source ??)
    ui->tlevel->setMaximum(6*probe);
    ui->tlevel->setMinimum(-6*probe);
    ui->tslopea->
    ui->tslopeb->

*/
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
    if (!scope.connected()) return;
    scope.setConfig();
    nocommands=true;   // stop ui changes from writing to scope
    ui->acqType->setCurrentIndex(tmp=scope.cmdCharIndex(":ACQ:TYPE?","NAP"));
    ui->acqAvg->setEnabled(tmp==1);
    // detect real or equal time
    ui->acqMode->setCurrentIndex(scope.acqMode()=='R'?0:1);
    int samp=scope.average();
    int idx=0;
    switch (samp)
    {
    case 255: idx=7;
              break;
    case 128: idx=6;
              break;
    case 64:  idx=5;
              break;
    case 32:  idx=4;
              break;
    case 16:  idx=3;
              break;
    case 8:   idx=2;
              break;
    case 4:   idx=1;
              break;
    case 2:   idx=0;
              break;
    }

    ui->acqAvg->setCurrentIndex(idx);
    ui->acqMem->setChecked(scope.isLongMemory());
    ui->srate->setText(QString::number(scope.sampleRate(),'f',0));


    on_tmode_currentIndexChanged(ui->tmode->currentIndex());

    // kind of ugly
    int decade;
    int sign;
    int scale=scope.scale(&decade,&sign);
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
        if (sign==1) sscale.append("0s"); else sscale.append("00ms");
        break;
    case 0:
        sscale.append("s");
    }
   ui->hscale->setCurrentIndex(ui->hscale->findText(sscale));
   ui->cdisp1->setChecked(scope.isChannelDisplayed(1));  // programming guide is wrong!
   ui->cdisp2->setChecked(scope.isChannelDisplayed(2));  // programming guide is wrong!
   ui->c1bw->setChecked(scope.bandwidthLimit(1));
   ui->c2bw->setChecked(scope.bandwidthLimit(2));
   ui->c1inv->setChecked(scope.inverted(1));
   ui->c2inv->setChecked(scope.inverted(2));
   ui->c1filt->setChecked(scope.filtered(1));
   ui->c2filt->setChecked(scope.filtered(2));
   QString mode=scope.trigMode();
   int  moden=ui->tmode->findText(mode,static_cast<Qt::MatchFlags>(Qt::MatchFixedString));
   if (moden!=-1)
   {
       ui->tmode->setCurrentIndex(moden);
   }
   else
   {
       ui->statusBar->showMessage(QString("Unsupported trigger mode ")+mode);
   }
   // since nocommands is set, this will just update ui
   on_tmode_currentIndexChanged(ui->tmode->currentIndex());
   QString source=scope.triggerSource(ui->tmode->currentText());
   ui->tsource->setCurrentIndex(ui->tsource->findText(source,Qt::MatchFlags(Qt::MatchFixedString)));
   // TODO use trigger source channel and type to set level min/max
   // based on scopedata.config.vscale[chan]


   setupChannel(1,ui->c1probe,ui->c1vscale,ui->c1offspin);
   setupChannel(2,ui->c2probe,ui->c2vscale,ui->c2offspin);
   ui->c1coup->setCurrentIndex(ui->c1coup->findText(scope.coupling(1)));
   ui->c2coup->setCurrentIndex(ui->c2coup->findText(scope.coupling(2)));
   // convert to uS
   ui->hoffsetspin->setValue(scope.config.hoffset*1000000.0f);
   ui->c1offspin->setValue(scope.config.voffset[0]);
   ui->c2offspin->setValue(scope.config.voffset[1]);



   QString cmd;

   source=scope.sweep(ui->tmode->currentText()); // scope doesn't seem to care what trigger type is active
   ui->tsweep->setCurrentIndex(ui->tsweep->findText(source,Qt::MatchFlags(Qt::MatchFixedString)));

   ui->tholdoff->setValue(scope.triggerHoldUs());

   // TODO: Need to read current scale to set range. Complex because range depends on trigger source scale

   if (ui->tmode->currentText()!="Slope")
   {
    ui->tlevel->setValue(scope.cmdFloat(":TRIG:"+ui->tmode->currentText()+":LEV?"));
   }
    ui->tcouple->setCurrentIndex(ui->tcouple->findText(scope.trigCoupling(ui->tmode->currentText()),Qt::MatchFlags(Qt::MatchFixedString)));

    ui->tposneg->setCurrentIndex(scope.isEdgeSlopePos()?0:1);
    ui->tedgesense->setValue(scope.cmdFloat(":TRIG:EDGE:SENS?"));
    ui->tpulsesense->setValue(scope.cmdFloat(":TRIG:PULS:SENS?"));
    ui->tpulswid->setValue(scope.cmdFloat(":TRIG:PULS:WIDT?")*1000000.0f);
    cmd=scope.trigPulseMode();
    for (tmp=0;tmp<ui->tpulsemode->count();tmp++)
    {
        if (ui->tpulsemode->itemText(tmp).remove(' ')==cmd)
        {
            ui->tpulsemode->setCurrentIndex(tmp);
            break;
        }
    }

    ui->tslopetime->setValue(scope.cmdFloat(":TRIG:SLOP:TIME?")*1000000.0f);
    ui->tslopesense->setValue(scope.cmdFloat(":TRIG:SLOP:SENS?"));
    // oddly TRIG SLOP MODE this returns spaces but can't have them in the command
    ui->tslopemode->setCurrentIndex(ui->tslopemode->findText(scope.trigSlopeMode(),Qt::MatchFlags(Qt::MatchFixedString)));
    ui->tslopewin->setCurrentIndex(ui->tslopewin->findText(scope.trigSlopeWin(),Qt::MatchFlags(Qt::MatchFixedString)));
    ui->tslopea->setValue(scope.cmdFloat(":TRIG:SLOP:LEVA?"));
    ui->tslopeb->setValue(scope.cmdFloat(":TRIG:SLOP:LEVB?"));



    ui->mathdisp->setChecked(scope.mathDisplay());
    ui->mathsel->setCurrentIndex(ui->mathsel->findText(scope.mathOp(),Qt::MatchFlags(Qt::MatchFixedString)));


   nocommands=savestate;

}

void MainWindow::on_acqType_currentIndexChanged(int index)
{
    if (!scope.connected()||nocommands) return;
    switch (index)
    {
    case 0:
        scope.setAcqTNormal();
        break;
    case 1:
        scope.setAcqTAverage();
        break;
    case 2:
        scope.setAcqTPeak();
        break;
    }
    ui->acqAvg->setEnabled(index==1);
}

void MainWindow::on_acqMode_currentIndexChanged(int index)
{
    if (!scope.connected()||nocommands) return;
    switch (index)
    {
    case 0:
        scope.setAcqModeRtim();
        break;
    case 1:
        scope.setAcqModeEtim();
        break;
    }
}

void MainWindow::on_acqAvg_currentIndexChanged(int index)
{
    if (!scope.connected()||nocommands) return;
    scope.setAcqAverage(1<<(index+1));
}


void MainWindow::on_acqMem_clicked()
{
    if (!scope.connected()) return;
    if (ui->acqMem->isChecked()) scope.setAcqMemLong(); else scope.setAcqMemNorm();
}



void MainWindow::on_hscale_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!scope.connected()||nocommands) return;
    scope.setTimeScale(ui->hscale->currentText());
}

void MainWindow::on_cdisp1_clicked()
{
    if (!scope.connected()) return;
    scope.setChanDisp(1,ui->cdisp1->checkState());
}

void MainWindow::on_cdisp2_clicked()
{
    if (!scope.connected()) return;
    scope.setChanDisp(2,ui->cdisp2->checkState());
}

void MainWindow::on_updAcq_2_clicked()
{
    on_updAcq_clicked();
}


void MainWindow::on_c1bw_clicked()
{
    if (!scope.connected()) return;
    scope.setChanBWL(1,ui->c1bw->checkState());
}

void MainWindow::on_c2bw_clicked()
{
    if (!scope.connected()) return;
    scope.setChanBWL(2,ui->c2bw->checkState());
}

void MainWindow::on_c1inv_clicked()
{
    if (!scope.connected()) return;
    scope.setChanInvert(1,ui->c1inv->checkState());
}

void MainWindow::on_c2inv_clicked()
{
    if (!scope.connected()) return;
    scope.setChanInvert(2,ui->c2inv->checkState());
}

void MainWindow::on_c1filt_clicked()
{
    if (!scope.connected()) return;
    scope.setChanFilter(1,ui->c1filt->checkState());
}

void MainWindow::on_c2filt_clicked()
{
    if (!scope.connected()) return;
    scope.setChanFilter(2,ui->c2filt->checkState());
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index);
    if (!scope.connected()) return;
    on_updAcq_clicked();  // refresh from scope when switching tabs
}

void MainWindow::on_c1probe_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!scope.connected()||nocommands) return;
    scope.setChanProbe(1,ui->c1probe->currentText().remove('X').toInt());
}

void MainWindow::on_c2probe_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!scope.connected()||nocommands) return;
    scope.setChanProbe(2,ui->c2probe->currentText().remove('X').toInt());
}

void MainWindow::on_c1coup_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!scope.connected()||nocommands) return;
    scope.setChanCouple(1,ui->c1coup->currentText());
}

void MainWindow::on_c2coup_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!scope.connected()||nocommands) return;
    scope.setChanCouple(2,ui->c2coup->currentText());
}


void MainWindow::on_hoffsetspin_valueChanged(double arg1)
{
    if (!scope.connected()||nocommands) return;
    scope.setTimeOffsetUs(arg1);
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
    if (!scope.connected()||nocommands) return;
    scope.setChanOffset(1,ui->c1offspin->value());
}

void MainWindow::on_c2offspin_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    if (!scope.connected()||nocommands) return;
    scope.setChanOffset(2,ui->c2offspin->value());
}

void MainWindow::on_c1vscale_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!scope.connected()||nocommands) return;
    scope.setChanScale(1,ui->c1vscale->currentText());
}

void MainWindow::on_c2vscale_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if (!scope.connected()||nocommands) return;
    scope.setChanScale(2,ui->c2vscale->currentText());
}

void MainWindow::on_unlockBtn_clicked()
{
    QSettings set;
    set.setValue("Options/Unlock",ui->unlockBtn->checkState()==Qt::Checked);
}

void MainWindow::on_hoffclear_clicked()
{
    if (!scope.connected()) return;
    ui->hoffsetspin->setValue(0.0);
}







void MainWindow::on_updAcq_3_clicked()
{
    on_updAcq_clicked();
}

void MainWindow::on_tmode_currentIndexChanged(int index)
{
    ui->tboxedge->setVisible(index==0);
    ui->tboxpulse->setVisible(index==1);
    ui->tboxslope->setVisible(index==2);
    ui->tsource->clear();
    ui->tsource->addItem("CHAN1");
    ui->tsource->addItem("CHAN2");
    ui->tsource->addItem("EXT");
    if (index==0) ui->tsource->addItem("ACLINE");
    ui->tcouple->setEnabled(index<3);
    if (!scope.connected()||nocommands) return;
    scope.setTrigMode(ui->tmode->currentText());
    QString res;
    res=scope.triggerSource(ui->tmode->currentText());
    ui->tsource->setCurrentIndex(ui->tsource->findText(res,Qt::MatchFlags(Qt::MatchFixedString)));
    res=scope.sweep(ui->tmode->currentText());
    ui->tsweep->setCurrentIndex(ui->tsweep->findText(res,Qt::MatchFlags(Qt::MatchFixedString)));
// This seems like a good idea, but the scope doesn't shift gears fast enough
// For example, going to slope mode where the trigger channel is invalid and back to edge does not
// update the channel source correctly
//    on_updAcq_clicked();

}

void MainWindow::on_tsweep_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    if (!scope.connected()||nocommands) return;
    scope.setSweep(ui->tmode->currentText(),ui->tsweep->currentText());
}

void MainWindow::on_tsource_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    QString cmd=":TRIG:";
    if (!scope.connected()||nocommands) return;
    scope.setTrigSource(ui->tmode->currentText(),ui->tsource->currentText());
}

void MainWindow::on_tholdoff_valueChanged(double arg1)
{
    QString cmd;
    if (!scope.connected()||nocommands) return;
    scope.setTrigHoldUs(arg1);
}

void MainWindow::on_tfifty_clicked()
{
    if (!scope.connected()) return;
    scope.trig50();
}

void MainWindow::on_tforce_clicked()
{
    if (!scope.connected()) return;
    scope.force();
}

void MainWindow::on_tlevel_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    if (!scope.connected()||nocommands) return;
    scope.setTrigLevel(ui->tmode->currentText(),ui->tlevel->value());
}

void MainWindow::on_tcouple_currentIndexChanged(const QString &arg1)
{
    if (!scope.connected()||nocommands) return;
    scope.setTrigCouple(ui->tmode->currentText(),arg1);
}

void MainWindow::on_tposneg_currentIndexChanged(int index)
{
    if (!scope.connected()||nocommands) return;
    scope.setTrigEdgeSlope(index==0);
}

void MainWindow::on_tedgesense_valueChanged(double arg1)
{
    if (!scope.connected()||nocommands) return;
    scope.setTrigEdgeSense(arg1);
}

void MainWindow::on_tpulsesense_valueChanged(double arg1)
{
    if (!scope.connected()||nocommands) return;
    scope.setTrigPulseSense(arg1);
}

void MainWindow::on_tpulswid_valueChanged(double arg1)
{
    if (!scope.connected()||nocommands) return;
    scope.setTrigPulseWidthUs(arg1);
}



void MainWindow::on_tpulsemode_currentIndexChanged(const QString &arg1)
{
    QString arg=arg1;
    if (!scope.connected()||nocommands) return;
    arg=arg.remove(' ');
    scope.setTrigPulseMode(arg);
}







void MainWindow::on_mlogfileselect_clicked()
{
    QString fn=QFileDialog::getSaveFileName(this,"Select log file to create or append to",QString(),"Comma Separated Value Files (*.csv);;All files (*.*)",0,QFileDialog::DontConfirmOverwrite);
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
        capcount=0;
        ui->captureCount->setText("0 captures logged");
        ui->mlogfileselect->setEnabled(false);
        mlog=new QFile(ui->mlogfilename->text());
        mlogworker.prep(mlog,ui->measlogheader->isChecked(),ui->updTimeScale->currentIndex()>0,ui->mlogstop->isChecked()?ui->mlogrepeat->value():-1);
        mlogworker.start();
    }
    else
    {
        ui->mlogfileselect->setEnabled(true);
        mlogworker.terminate();
        mlog->close();
    }
}


void MainWindow::on_mathdisp_clicked()
{
    if (!scope.connected()) return;
    scope.setMathDisp(ui->mathdisp->isChecked());
}

void MainWindow::on_mathsel_currentIndexChanged(const QString &arg1)
{
    if (!scope.connected()||nocommands) return;
   scope.setMathOper(arg1);
}

/* Still need to do the slope commands */
void MainWindow::on_tslopemode_currentIndexChanged(const QString &arg1)
{
    // Needs to have no spaces
    QString arg=arg1;
    if (!scope.connected()||nocommands) return;
    arg=arg.remove(' ');
    scope.setTrigSlopeMode(arg);
}

void MainWindow::on_tslopewin_currentIndexChanged(const QString &arg1)
{
    // need to convert P_WIN_A to PA, P_WIN_AB to PAB etc.
    if (!scope.connected()||nocommands) return;
    QString cmd,arg=arg1;
    arg=arg.replace("_WIN_","");
    scope.setTrigSlopeWindow(arg);
}

void MainWindow::on_tslopetime_valueChanged(double arg1)
{
    if (!scope.connected()||nocommands) return;
    scope.setTrigSlopeTimeUs(arg1);
}

void MainWindow::on_tslopea_valueChanged(double arg1)
{
    if (!scope.connected()||nocommands) return;
    scope.setTrigSlopeLevA(arg1);
}

void MainWindow::on_tslopeb_valueChanged(double arg1)
{
    if (!scope.connected()||nocommands) return;
    scope.setTrigSlopeLevB(arg1);
}


void MainWindow::on_tslopesense_valueChanged(double arg1)
{
    if (!scope.connected()||nocommands) return;
    scope.setTrigSlopeSense(arg1);
}

// This is about the only place we should be direct accessing command and com.buffer
void MainWindow::on_action_Diagnostic_triggered()
{
    bool ok;
    if (!scope.connected()) return;
    QString cmd=QInputDialog::getText(this,"Enter Diagnostic Command","Command:",QLineEdit::Normal,"",&ok);
    QMessageBox bx;
    if (!ok || cmd.isEmpty()) return;
 // Could check to see that command starts with : or *
    if (cmd.left(1)==":" || cmd.left(1)=="*")
    {
       scope.command(cmd);
       if (   cmd.right(1)[0]!='?') return;
       bx.setText(scope.com.buffer);
    }
    else
        bx.setText("Commands should start with : or * to be valid. No command sent.");
    bx.exec();
}


void MainWindow::on_actionRun_Stop_triggered()
{
    if (scope.connected())
       on_rsButton_clicked();
    else ui->actionRun_Stop->setChecked(false);
}


void MainWindow::on_actionConnect_triggered()
{
    ui->connectButton->setChecked(!scope.connected());
    on_connectButton_clicked();
}



void MainWindow::on_exportButton_clicked()
{
    QSettings set;
    set.setValue("Options/exportc1",ui->wavec1->isChecked());
    set.setValue("Options/exportc2",ui->wavec2->isChecked());
    set.setValue("Options/exporthead",ui->wavehead->isChecked());
    set.setValue("Options/exportcfg",ui->wavesavecfg->isChecked());
    set.setValue("Options/exportraw",ui->waveraw->isChecked());
    set.setValue("Options/exportthresh",ui->logicThresh->value());
    switch (ui->exportFmt->currentIndex())
    {
        case 0:
            scope.do_export_csv(ui->wavec1->isChecked(),ui->wavec2->isChecked(),ui->wavetimeopt->isChecked(),ui->wavehead->isChecked(),ui->wavesavecfg->isChecked(),ui->waveraw->isChecked());
            break;
        case 1:
            scope.do_export_ols(ui->wavec1->isChecked(),ui->wavec2->isChecked(),ui->logicThresh->value());
            break;
        case 2:
            scope.do_export_sigrok(ui->wavec1->isChecked(),ui->wavec2->isChecked(),ui->logicThresh->value());
            break;
        case 3:
            scope.do_wave_plot(ui->wavec1->isChecked(),ui->wavec2->isChecked());
            break;

    }
}

void MainWindow::on_exportFmt_currentIndexChanged(int index)
{
    // 0 = CSV, 1=OLS, 2=SIGROK, 3=PLOT/CUSTOM
    switch (index)
    {
    case 0:
        ui->wavehead->setEnabled(true);
        ui->waveraw->setEnabled(true);
        ui->wavetimeopt->setEnabled(true);
        ui->wavesavecfg->setEnabled(true);
        ui->logicThresh->setEnabled(false);
        break;
    case 1:
    case 2:
        ui->wavehead->setEnabled(false);
        ui->waveraw->setEnabled(false);
        ui->wavetimeopt->setEnabled(false);
        ui->wavesavecfg->setEnabled(false);
        ui->logicThresh->setEnabled(true);
        break;
    case 3:
        ui->wavehead->setEnabled(false);
        ui->waveraw->setEnabled(false);
        ui->wavetimeopt->setEnabled(false);
        ui->wavesavecfg->setEnabled(false);
        ui->logicThresh->setEnabled(false);

        break;

    }
    QSettings set;
    set.setValue("Options/exportselect",index);
}

void MainWindow::on_action_Help_triggered()
{
    // Not 100% sure about the lifetime management of this
    if (!helpdlg) helpdlg=new HelpDialog(this);
    helpdlg->showRequest();
}
