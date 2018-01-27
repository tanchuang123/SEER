#include "formstatus.h"
#include "ui_formstatus.h"
#include <QtDebug>
#include <QFile>
Formstatus::Formstatus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Formstatus)
{
    ui->setupUi(this);
}

Formstatus::~Formstatus()
{
    delete ui;
}

void Formstatus::updateSendPBar_(qint64 size,qint64 sendBlockNumber_,qint64 sendMaxBytes_)
{
    //sendBlockNumber_ ;
    sendBytes_ += size;
    ui->pbarSendData->setValue(sendBytes_);
    QString msg_ = QString("已发数据包:%1个 当前数据包大小:%2字节 已发字节:%3 总共字节:%4").arg(sendBlockNumber_).arg(size).arg(sendBytes_).arg(sendMaxBytes_);
    // qDebug()<<","<<sendBlockNumber_<<","<<size<<","<<sendBytes_<<","<<sendMaxBytes_<<",";
    ui->txtSendStatus->setText(msg_);
    qApp->processEvents();//强制刷新
}

void Formstatus::updateSendStatus_(QString msg_)
{
    qDebug() << "发送文件客户端:" << msg_;
}

void Formstatus::setSendPBar_(qint64 size)
{
    //QFile file(fileName);

   // sendMaxBytes_ = size;

    ui->pbarSendData->setRange(0, size-1 );
    ui->pbarSendData->setValue(0);
}

void Formstatus::sendFinsh_()
{
    ui->pbarSendData->setRange(0, 100);
    ui->pbarSendData->setValue(100);
    ui->txtSendStatus->setText("发送文件成功");
}
QString Formstatus::getfileName(QString fileName)
{
    ui->txtSendFile->setText(fileName);

    return fileName;
}
