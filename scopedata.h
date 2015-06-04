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
    RigolComm *com;
    MainWindow *win;

public:


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
    int command(const QString &cmd);
    float cmdFloat(const QString &cmd);
    void setComm(RigolComm *c) { com=c; }
    void do_wave_plot(bool c1, bool c2);
    void do_export_csv(bool c1, bool c2, bool dotime, bool wheader, bool wconfig, bool raw);
    void do_export_ols(bool c1, bool c2, float thresh);
    void do_export_sigrok(bool c1, bool c2, float thresh);


signals:

public slots:



};

#endif // SCOPEDATA_H
