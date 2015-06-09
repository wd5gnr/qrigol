#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "rigolcomm.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QThread>
#include "mlogger.h"
#include "scopedata.h"
#include "helpdialog.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    float lastMeasure[2][20];

    void chanDisp(int chan,bool disp);
    bool isChannelDisplayed(int chan);



private slots:
    void on_connectButton_clicked();

    void on_rsButton_clicked();

    void on_measUpdate_clicked();

    void on_autoUpdate_clicked();

    void on_uiUpdate();

    void on_hardcopyBTN_clicked();

    void on_updAcq_clicked();

    void on_acqType_currentIndexChanged(int index);


    void on_acqMode_currentIndexChanged(int index);

    void on_acqAvg_currentIndexChanged(int index);

    void on_acqMem_clicked();


    void on_hscale_currentIndexChanged(int index);

    void on_cdisp1_clicked();

    void on_updAcq_2_clicked();

    void on_cdisp2_clicked();

    void on_c1bw_clicked();

    void on_c2bw_clicked();

    void on_c1inv_clicked();

    void on_c2inv_clicked();

    void on_c1filt_clicked();

    void on_c2filt_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_c1probe_currentIndexChanged(int index);

    void on_c2probe_currentIndexChanged(int index);

    void on_c1coup_currentIndexChanged(int index);

    void on_c2coup_currentIndexChanged(int index);



    void on_hoffsetspin_valueChanged(double arg1);

    void on_hoffincr_valueChanged(double arg1);

    void on_c1offspin_valueChanged(double arg1);

    void on_c2offspin_valueChanged(double arg1);

    void on_c1vscale_currentIndexChanged(int index);

    void on_c2vscale_currentIndexChanged(int index);

    void on_unlockBtn_clicked();

    void on_hoffclear_clicked();

    void on_action_About_triggered();



    void on_updAcq_3_clicked();


    void on_tmode_currentIndexChanged(int index);

    void on_tsweep_currentIndexChanged(const QString &arg1);

    void on_tsource_currentIndexChanged(int index);

    void on_tholdoff_valueChanged(double arg1);

    void on_tfifty_clicked();

    void on_tforce_clicked();

    void on_tlevel_valueChanged(double arg1);

    void on_tcouple_currentIndexChanged(const QString &arg1);

    void on_tposneg_currentIndexChanged(int index);

    void on_tedgesense_valueChanged(double arg1);

    void on_tpulsesense_valueChanged(double arg1);

    void on_tpulswid_valueChanged(double arg1);


    void on_tpulsemode_currentIndexChanged(const QString &arg1);

    void on_mlogfileselect_clicked();

    void on_measLogEnable_clicked();

      void on_mathdisp_clicked();

    void on_mathsel_currentIndexChanged(const QString &arg1);

    void on_tslopemode_currentIndexChanged(const QString &arg1);

    void on_tslopewin_currentIndexChanged(const QString &arg1);

    void on_tslopetime_valueChanged(double arg1);

    void on_tslopea_valueChanged(double arg1);

    void on_tslopesense_valueChanged(double arg1);

    void on_tslopeb_valueChanged(double arg1);

    void on_action_Diagnostic_triggered();



    void on_actionRun_Stop_triggered();



    void on_actionConnect_triggered();



    void on_exportButton_clicked();

    void on_exportFmt_currentIndexChanged(int index);

    void on_action_Help_triggered();

private:
    Ui::MainWindow *ui;
     QTimer *uTimer;
     QTimer *uiTimer;

    bool nocommands;
    void setupChannel(int ch, QComboBox *probebox, QComboBox *scalebox, QDoubleSpinBox *coffset);
    void restoreSavedSettings(void);
    QFile *mlog;
    MLogger mlogworker;
    ScopeData scope;
    unsigned capcount;
    HelpDialog *helpdlg;
};

#endif // MAINWINDOW_H
