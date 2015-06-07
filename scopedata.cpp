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
#include "scopedata.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QProcess>
#include <QTime>
#include <QDebug>
#include "mainwindow.h"
#include "plotdialog.h"
#include <unistd.h>

// TODO: Need to detect bad QProcess starts
// TODO: Need to check return value from sigrok

ScopeData::ScopeData(QObject *parent) :
    QObject(parent)
{
    win=(MainWindow *)parent;
}
QString ScopeData::id(void)
{
       *com.buffer='\0';
       com.command("*IDN?");
       return QString(com.buffer); }

bool ScopeData::logiccmd(QString cmd,int chan,int offset,int ch)
{
    QString rcmd;
    if (chan!=-1) rcmd.sprintf(cmd.toLatin1(),chan); else rcmd=cmd;
    command(rcmd);
    return com.buffer[offset]==ch;
}

bool ScopeData::isChannelDisplayed(int chan)
{
    // Note the manual says this returns ON/OFF, but we see it returns 0 and 1 so..
    QString cmd=":CHAN";
    cmd+=QString::number(chan)+":DISP?";
    command(cmd);
    return com.buffer[1]=='N' || com.buffer[0]=='1';   // just in case programming guide is right for old fw
}


int ScopeData::cmdCharIndex(const QString &cmd,const QString &search,int bpos)
{
    QChar c;
    command(cmd);
    c=com.buffer[bpos];
    return search.indexOf(c,0,Qt::CaseInsensitive);
}


int ScopeData::scale(int *decade,int *sign)
{
    char *p;
    command(":TIM:SCAL?");
    if (decade) *decade=QString(com.buffer).right(1).toInt();
    if (sign) *sign=QString(com.buffer).right(3).left(1)[0]=='+'?1:-1;
    p=strchr(com.buffer,'.');
    if (p) *p='\0';
    return QString(com.buffer).toInt();
}

QString ScopeData::triggerSource(const QString &mode)
{
    QString cmd=":TRIG:";
    QString rv;
    cmd+=mode+":SOUR?";
    command(cmd);
    if (com.buffer[2]=='1') rv="CHAN1";
    else if (com.buffer[2]=='2') rv="CHAN2";
    else rv=com.buffer;
    return rv;
}


QString ScopeData::sweep(const QString &mode)
{
    QString cmd=":TRIG:";
    QString rv;
    cmd+=mode+":SWE?";
    command(cmd);
    rv=com.buffer;
    return rv;
}

void ScopeData::setConfig(void)
{
    QString cmdbase,cmd;
    config.hscale=cmdFloat(":TIM:SCAL?");
    config.hoffset=cmdFloat(":TIM:OFFS?");

    config.vscale[0]=cmdFloat(":CHAN1:SCAL?");
    config.voffset[0]=cmdFloat(":CHAN1:OFFS?");
    config.vscale[1]=cmdFloat(":CHAN2:SCAL?");
    config.voffset[1]=cmdFloat(":CHAN2:OFFS?");
    config.srate=cmdFloat(":ACQ:SAMP? CHAN1");
    config.deltat=1.0f/config.srate;
    config.set=true;
}


int ScopeData::convertbuf(int chan, const QString &cmd, bool raw)
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

void ScopeData::waitForStop(void)
{
    QTime timeout;
    timeout.start();
    do
    {
        command(":TRIG:STAT?");
    } while (*(com.buffer)!='S' && timeout.elapsed()<2000);  // Wait for stop or 2 seconds

}


int ScopeData::prepExport(bool c1, bool c2)
{
    bool running=false;

    // make sure connected
    if (!com.connected()) return -1;
    // we know we will be stopped so we are going to get
    // either 8K/16K (based on display) or 512K/1M based on long memory mode
    //
    command(":TRIG:STAT?");
    running=com.buffer[0]!='S';
    if (running)
    {
         win->chanDisp(1,c1);
         win->chanDisp(2,c2);
    }

    if ((c1 && !isChannelDisplayed(1)) || (c2 && !isChannelDisplayed(2)))
    {
       QMessageBox bx;
       bx.setText("Channel not enabled and instrument in stop mode");
       bx.exec();
       return -1;
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

int ScopeData::fillExportBuffer(bool c1, bool c2,bool raw)
{
    int asize,wsize=0;
    // Now we can really compute the size
    command(":WAV:POIN:MODE MAX");
    command(":ACQ:MEMD?");
    asize=com.buffer[0]=='L'?524288:8192;   // now it will be right
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
    command(":STOP");   // only way to be sure we sync chan1 and chan2 that I can tell
    // however, it takes a bit for it to "settle" If you get a short sample, just run it again
    // since the scope is stopped it will be ok
    waitForStop();
    usleep(500000);  // if you don't delay you get short buffer first time even with wait for stop ???
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
   return wsize;
}

// This function handles exporting for both CSV and plotting
int ScopeData::exportEngine(bool dotime, bool c1, bool c2, bool wheader, bool wconfig, bool raw, QFile *file)
{
    int wsize;

    if ((wsize=fillExportBuffer(c1,c2,raw))<0) return -1;

    // Get file name
    if (file==NULL)
    {
        QString fn=QFileDialog::getSaveFileName(win,"Select log file to create or append to",QString(),"Comma Separate Value Files (*.csv);;All files (*.*)");
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

void ScopeData::do_wave_plot(bool c1, bool c2)
{
    PlotDialog *dlg=new PlotDialog(win);
    QString cmd,script;

    if (prepExport(c1,c2)) return;
    if (!dlg->exec()) return;
    QTemporaryFile *file=new QTemporaryFile(QDir::tempPath()+"/qrigoldata_XXXXXX.csv");
    QTemporaryFile *scriptfile=new QTemporaryFile(QDir::tempPath()+"/qrigolscript_XXXXXX");
    file->setAutoRemove(false);
    scriptfile->setAutoRemove(false);
    scriptfile->open();
    file->open();
    script=dlg->script;
    script=script.replace("{FILE}",file->fileName());  // does not work unless file is open!
    scriptfile->write(script.toLatin1());
    cmd=dlg->command;
    cmd=cmd.replace("{SCRIPT}",scriptfile->fileName());
    cmd=cmd.replace("{FILE}",file->fileName());
// get names before close
    scriptfile->close();
    // generate temporary file
    exportEngine(true,c1,c2,true,false,false,file);
    // execute command
    QProcess proc;
    if (!proc.startDetached(cmd))
    {
        QMessageBox done;
        done.setText("Unable to start plot program specified.");
        done.exec();
    }
    delete file;
    delete scriptfile; // really should have made these on the stack
    // Note autodelete is off so we pollute the /tmp directory with files
    // The problem is we don't know when the other program is done with them
}

void ScopeData::do_export_csv(bool c1, bool c2, bool dotime, bool wheader, bool wconfig, bool raw)
{
    if (prepExport(c1,c2)) return;

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

void ScopeData::do_export_ols(bool c1, bool c2,float thresh)
{
    if (prepExport(c1,c2)) return;

    int i,lastdat;
    QString line;

    QString fn;
    if (fillExportBuffer(c1,c2,false)<0) return;
    fn=QFileDialog::getSaveFileName(win,"OLS Export File Name",QString(),"OLS Files (*.ols);; All Files (*.*)");
    if (fn.isEmpty()) return;
    QFile file(fn);
    file.open(QIODevice::WriteOnly);
    line=";Rate: ";
    line+=QString().sprintf("%d\n",(int)config.srate);
    file.write(line.toLatin1());
    line=";Channels: ";
    i=0;
    if (c1) i++;
    if (c2) i++;
    line+=QString::number(i)+"\n";
    file.write(line.toLatin1());
    i=0;
    if (c1) i|=1;
    if (c2) i|=2;
    line="EnabledChannels: ";
    line+=QString::number(i)+"\n";
    file.write(line.toLatin1());
    lastdat=-1;
    for (i=0;i<chansize;i++)
    {
        int dat=0;
        if (c1 && chandata[0][i]>thresh) dat|=1;
        if (c2 && chandata[1][i]>thresh) dat|=2;
        if (dat!=lastdat)
        {
            line.sprintf("%X@%d\n",dat,i);
            file.write(line.toLatin1());
            lastdat=dat;
        }
    }
    file.close();
    QMessageBox done;
    QString msg;
    msg="Wrote ";
    msg+=QString::number(chansize)+" records";
    done.setText(msg);
    done.exec();

}

void ScopeData::do_export_sigrok(bool c1, bool c2, float thresh)
{
    if (prepExport(c1,c2)) return;
    int i;
    QString line;

    QString fn;
    if (fillExportBuffer(c1,c2,false)<0) return;
    fn=QFileDialog::getSaveFileName(win,"OLS Export File Name",QString(),"Sigrok Files (*.sr);; All Files (*.*)");
    if (fn.isEmpty()) return;
    QTemporaryFile file(QDir::tempPath()+"/qrigoldata_XXXXXX.csv");
//    file.setAutoRemove(false);  -- should be ok to remove this one when done
    file.open();
    for (i=0;i<chansize;i++)
    {
        int dat=0;
        if (c2 && chandata[1][i]>thresh) dat=1;
        if (c1) dat<<=1;
        if (c1 && chandata[0][i]>thresh) dat|=1;
        line.sprintf("%d,%d\n",dat&1,(dat&2)?1:0);
        file.write(line.toLatin1());
    }


    QProcess proc;
    QStringList args;
    QString srs;
    srs=srs.sprintf("csv:samplerate=%d:numchannels=",(int)config.srate);
    if (c1&&c2) srs+="2"; else srs+="1";
    args<<"-I"<<srs<<"-i"<<file.fileName()<<"-o"<<fn;
    file.close();
    qDebug()<<args;
    proc.start("sigrok-cli",args);
    proc.waitForFinished();
    QMessageBox done;
    QString msg;
    msg="Wrote ";
    msg+=QString::number(chansize)+" records";
    done.setText(msg);
    done.exec();

}



