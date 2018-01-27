#ifndef SCTCPTOOLWIDGET_H
#define SCTCPTOOLWIDGET_H
#include <QFileDialog>
#include <QWidget>
#include "SCStatusTcp.h"
#include "ProtoBufTool/ProtobufWidget.h"
#include <QCoreApplication>
#include "formStatus/formstatus.h"
namespace Ui {
class SCTcpToolWidget;
}

class SCTcpToolWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SCTcpToolWidget(QWidget *parent = 0);
    ~SCTcpToolWidget();
    static QString GetFileNameWithExtension(QString strFilePath)//获取文件名
    {
        QFileInfo fileInfo(strFilePath);
        return fileInfo.fileName();
    }

    static QString GetFileName(QString filter)//打开文件
    {
        return QFileDialog::getOpenFileName(0, "选择文件", QCoreApplication::applicationDirPath(), filter);
        //QAppllication::appllicationDirPath()的路径始终都是exe文件所在的绝对路径
    }
public slots:
    void stateChanged(QAbstractSocket::SocketState state);
    void receiveTcpError(QAbstractSocket::SocketError error);
    void slotPrintInfo(QString info);

    void slotChangedText(bool isOk, int revCommand, QByteArray revData, QByteArray revHex, int number, int msTime);
    void slotAutomaticallyScroll();
    void updateSendPBar(qint64 size);
    void updateSendStatus(QString msg);
    void setSendPBar(qint64 size);
    void sendFinsh();
private slots:
    void on_pushButton_connect_clicked(bool checked);
    void on_pushButton_send_clicked();

    void on_pushButton_clearInfo_clicked();

    void on_pushButton_zipFile_clicked();

    void on_checkBox_timeOut_clicked(bool checked);


    void on_btnSend_clicked();

    void on_btnSelect_clicked();

private:
    Ui::SCTcpToolWidget *ui;
    SCStatusTcp *pSCStatusTcp;
    ProtobufWidget *pProtobufWidget;

    int sendBytes;
    qint64 sendBlockNumber;
    qint64 sendMaxBytes;
    QString SendFile;

    Formstatus *status;
};

#endif // SCTCPTOOLWIDGET_H
