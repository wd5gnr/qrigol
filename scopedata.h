#ifndef SCOPEDATA_H
#define SCOPEDATA_H
#include "rigolcomm.h"

#include <QObject>
#include <QFile>

class MainWindow;

class ScopeData : public QObject
{
    Q_OBJECT
public:
    explicit ScopeData(QObject *parent = 0);

protected:
    MainWindow *win;


public:


   RigolComm com;   // ought to be protected but for now...

    struct rigolwfmcfg
    {
        bool set;
        float hscale;
        float hoffset;
        float vscale[2];
        float voffset[2];
        float srate;
        float deltat;
    } config;

    void setConfig(void);
    int convertbuf(int chan, const QString &cmd, bool raw);


    double *chandata[2];
    int chansize;
    int prepExport(bool c1, bool c2);
    int fillExportBuffer(bool c1, bool c2,bool raw);
    int exportEngine(bool dotime=true, bool c1=true, bool c2=true, bool wheader=true, bool wconfig=true, bool raw=false, QFile *file=NULL);
    int command(const QString &cmd) { return com.command(cmd.toLatin1()); }
    float cmdFloat(const QString &cmd) { return com.cmdFloat(cmd.toLatin1()); }
    float cmdFloatlt(const char *cmd) { return com.cmdFloat(cmd); }
    bool isChannelDisplayed(int chan);
    void do_wave_plot(bool c1, bool c2);
    void do_export_csv(bool c1, bool c2, bool dotime, bool wheader, bool wconfig, bool raw);
    void do_export_ols(bool c1, bool c2, float thresh);
    void do_export_sigrok(bool c1, bool c2, float thresh);
    void waitForStop(void);
    bool connected(void) { return com.connected(); }
    int close(void) { return com.close(); }
    int trigStatus(void) { com.command(":TRIG:STAT?"); return *com.buffer; }
    int unlock(void) { return com.unlock(); }
    int open(QString dev) { return com.open(dev.toLatin1()); }
    QString id(void);
    int stop(void) { return com.command(":STOP"); }
    int run(void) { return com.command(":RUN"); }
    int cmdCharIndex(const QString &cmd,const QString &search,int bpos=0);
    int acqMode(void) { command(":ACQ:MODE?"); return *com.buffer; }
    int average(void) { return (int)cmdFloat(":ACQ:AVER?"); }





signals:

public slots:



};

#endif // SCOPEDATA_H
