#include "helpdialog.h"
#include "ui_helpdialog.h"
#include <QSettings>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QWebFrame>  // debugging

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    QSettings set;
    topicload.clear();
    ui->setupUi(this);
    this->setGeometry((QRect)set.value("Options/helpsize",QRect(0,0,640,480)).toRect());
    on_indexButton_clicked();
}

HelpDialog::~HelpDialog()
{
    QSettings set;
    set.setValue("Options/helpsize",this->geometry());
    delete ui;
}


void HelpDialog::on_indexButton_clicked()
{
    ui->webView->page()->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls,true);
    ui->webView->page()->settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,true);
    QString url="qrc:/qrigol/help.html";
    ui->webView->load(QUrl(url));
}

void HelpDialog::on_search_clicked()
{
    if (!ui->webView->findText(ui->searchText->text()))
    {
        QMessageBox dlg;
        dlg.setText("Not found");
        dlg.exec();  // TODO Modeless!
    }
}

void HelpDialog::showRequest(const QString &topic)
{
    show();
    on_indexButton_clicked();
    if (topic.isEmpty()) return;
    topicload="qrc:/qrigol/" + topic + ".html";

}

void HelpDialog::on_webView_loadFinished(bool arg1)
{
    if (!topicload.isEmpty())
    {
        ui->webView->page()->mainFrame()->childFrames().at(1)->load(QUrl(topicload));
        topicload.clear();
    }
}
