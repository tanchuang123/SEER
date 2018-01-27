#include "receivefilethread.h"
////#include "myhelper.h"
#include <QDataStream>
#include <QHostAddress>
#include <QCoreApplication>
#include <QThread>
ReceiveFileThread::ReceiveFileThread(int socketDescriptor, QObject *parent) :
    QThread(parent)
{
    blockSize = 0;
    blockNumber  = 0;
    server = new QTcpSocket(this);
    server->setSocketDescriptor(socketDescriptor);//设置描述符
    connect(server, SIGNAL(readyRead()), this, SLOT(ReceiveData()));
    connect(server, SIGNAL(disconnected()), this, SLOT(DisConnect()));
    connect(server, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(displaySocketError(QAbstractSocket::SocketError)));
}

ReceiveFileThread::~ReceiveFileThread()
{

}

void ReceiveFileThread::proccessData(QByteArray &array)
{
    QDataStream in(&array, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_7);

    int key;
    QByteArray data;
    in >> key >> data;
    blockNumber ++;//已接收数据包

    emit message(QString("已接收数据包:%1个").arg( blockNumber));
    emit message(QString("收到标识符:%1 当前数据包大小:%2字节").arg(key).arg(data.size()));

    switch(key) {
    case 0x01:
        fileName = fileName.fromUtf8(data.data(), data.size());
       // file.setFileName(QCoreApplication::applicationDirPath() + "/" + fileName);//QCoreApplication::applicationDirPath()获取exe文件所在路径
        file.setFileName("F:/work/WORK/123/"+fileName);
        emit receiveFileName(file.fileName());
        if(file.exists())
        {
            file.remove();
        }
        if(!file.open(QIODevice::WriteOnly)) {
            emit message("不能打开文件进行写入");
            break;
        }
        break;
    case 0x02: {
        QString size = QString::fromUtf8(data.data(), data.size());
        emit receiveFileSize(size.toUInt());
        break;
    }
    case 0x03:
        file.write(data.data(), data.size());
        file.flush();
        break;
    case 0x04:
        file.close();
        server->disconnectFromHost();
        break;
    }
}

void ReceiveFileThread::run()
{
    emit message(QString("收到新连接请求 IP:%1[%2]").arg(server->peerAddress().toString()).arg(server->peerPort()));
    exec();
}

void ReceiveFileThread::DisConnect()
{
    exit(0);
}

void ReceiveFileThread::ReceiveData()
{
    while(server->bytesAvailable() >= sizeof(quint64)) {
        if(blockSize == 0) {
            if(server->bytesAvailable() < sizeof(qint64)) {
                return;
            }
            server->read((char *)&blockSize, sizeof(qint64));
        }
        if(server->bytesAvailable() < blockSize) {
            return;
        }
        emit receiveData(blockSize + sizeof(qint64));
        QByteArray data = server->read(blockSize);
        proccessData(data);
        blockSize = 0;
    }
}

void ReceiveFileThread::displaySocketError(QAbstractSocket::SocketError)
{
    emit message(QString("接收文件遇到错误:%1[%2]").arg(server->errorString()));
    emit message(QString("正在移除文件:%1").arg(file.fileName()));
    if(file.isOpen()) {
        file.close();
    } else {
        return;
    }
    if(!file.fileName().isEmpty()) {
        file.remove(fileName);
    } else {
        return;
    }
}

