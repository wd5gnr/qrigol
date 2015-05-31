#ifndef MLOGGER_H
#define MLOGGER_H

#include <QObject>
#include <QFile>
#include <QThread>
#include <QSemaphore>
#include <QMutex>

class MLogger : public QThread
{
    Q_OBJECT
public:
    explicit MLogger(QThread *parent = 0);
    QSemaphore sample;
    QMutex inuse;
    volatile float data[2][20];

 private:
    QFile *file;
    bool closeFlag;
    bool header;
    int repeat;


signals:
    void finished();

public slots:

protected:

    void run();

public:
    void prep(QFile *file, bool header, bool closeflag, int repeat);
};

#endif // MLOGGER_H
