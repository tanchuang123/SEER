#include "SCStatusTcp.h"
#include "SCHeadData.h"
#include <QFile>
#include "SCTcpToolWidget.h"
SCStatusTcp::SCStatusTcp(QObject *parent) : QObject(parent),
    _tcpSocket(Q_NULLPTR)
{

    connect(_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(displaySocketError(QAbstractSocket::SocketError)));
}
SCStatusTcp::~SCStatusTcp()
{
    releaseTcpSocket();
    if(_tcpSocket){
        delete _tcpSocket;
    }
}
/** 释放tcpSocket
 * @brief SCStatusTcp::releaseTcpSocket
 */
void SCStatusTcp::releaseTcpSocket()
{
    if(_tcpSocket){

        if(_tcpSocket->isOpen()){
            _tcpSocket->close();
        }
        _tcpSocket->abort();
    }
}
/** 连接
 * @brief SCStatusTcp::connectHost
 * @param ip
 * @param port
 * @return
 */
int SCStatusTcp::connectHost(const QString&ip,quint16 port)
{
    int ret = 0;
    if(!_tcpSocket){
        _tcpSocket = new QTcpSocket(this);
        connect(_tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveTcpReadyRead()));
        connect(_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                this->parent(), SLOT(stateChanged(QAbstractSocket::SocketState)));
        connect(_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this->parent(),
                SLOT(receiveTcpError(QAbstractSocket::SocketError)));
    }
    if(_tcpSocket->isOpen()
            &&(_tcpSocket->state()==QAbstractSocket::ConnectedState
               ||_tcpSocket->state()==QAbstractSocket::ConnectingState)){
        _tcpSocket->close();
        _tcpSocket->abort();
        qDebug()<<"----close _tcpSocket----\n";
        ret = 1;
    }else{
        _tcpSocket->connectToHost(ip,port,QTcpSocket::ReadWrite,QTcpSocket::IPv4Protocol);
        _ip = ip;
        _port = port;
        ret = 0;
    }
    return ret;
}
/** TCP请求
 * @brief SCStatusTcp::writeTcpData
 * @param sendCommand 报文类型
 * @param sendData 数据区数据
 * @param number 序号
 * @return
 */
bool SCStatusTcp::writeTcpData(uint16_t sendCommand,
                               const QByteArray &sendData,
                               uint16_t &number)
{
    //已发送
    _oldSendCommand = sendCommand;
    _oldNumber = number;
    //数据区长度
    int size = 0;
    //报文头部数据
    uint8_t* headBuf = Q_NULLPTR;
    int headSize = 0;
    //发送的全部数据
    SeerData* seerData = Q_NULLPTR;
    //开始计时
    _time.start();

    //根据数据区数据进行数据转换
    if(sendData.isEmpty()){
        headSize = sizeof(SeerHeader);
        headBuf = new uint8_t[headSize];
        seerData = (SeerData*)headBuf;
        size = seerData->setData(sendCommand,Q_NULLPTR,0,number);
    }else{
        std::string json_str = sendData.toStdString();
        headSize = sizeof(SeerHeader) + json_str.length();
        headBuf = new uint8_t[headSize];
        seerData = (SeerData*)headBuf;
        size = seerData->setData(sendCommand,
                                 (uint8_t*)json_str.data(),
                                 json_str.length(),
                                 number);
    }
    //---------------------------------------
    //发送的所有数据
    QByteArray tempA = QByteArray::fromRawData((char*)seerData,size);
    qDebug()<<"send:"<<QString(tempA)<<"  Hex:"<<tempA.toHex()<<"seerData:size:"<<size;
    QString dataHex = "";
    if(size<=2048){
        dataHex = hexToQString(sendData.toHex());
    }else{
        dataHex = tr("数据大于2048字节，不打印信息.");
    }
    //打印信息
    QString info = QString("\n%1--------- 请求 ---------\n"
                           "报文类型:%2 (0x%3) \n"
                           "端口: %4\n"
                           "序号: %5 (0x%6)\n"
                           "头部十六进制: %7 \n"
                           "数据区[size:%8 (0x%9)]: %10 \n"
                           "数据区十六进制: %11 ")
            .arg(getCurrentDateTime())
            .arg(sendCommand)
            .arg(QString::number(sendCommand,16))
            .arg(_port)
            .arg(number)
            .arg(QString::number(number,16))
            .arg(hexToQString(tempA.left(16).toHex()))
            .arg(sendData.size())
            .arg(QString::number(sendData.size(),16))
            .arg(QString(sendData))
            .arg(dataHex);

    emit sigPrintInfo(info);
    //---------------------------------------
    _tcpSocket->write((char*)seerData, size);
    delete[] headBuf;

    //-------------
    qDebug()<<"TCP:_timeOut:"<<_timeOut;
    //如果_timeOut = 0表示不监听超时
    if(0 == _timeOut){
        return true;
    }

    //等待写入
    if(!_tcpSocket->waitForBytesWritten(_timeOut)){
        _lastError = tr("waitForBytesWritten: time out!");
        return false;
    }
    //等待读取
    if(!_tcpSocket->waitForReadyRead(_timeOut)){
        _lastError = tr("waitForReadyRead: time out!");
        return false;
    }
    return true;
}
void SCStatusTcp::receiveTcpReadyRead()
{
    //读取所有数据
    //返回的数据大小不定,需要使用_lastMessage成员变量存放多次触发槽读取的数据。
    QByteArray message = _tcpSocket->readAll();
    message = _lastMessage + message;
    int size = message.size();

    while(size > 0){
        char a0 = message.at(0);
        if (uint8_t(a0) == 0x5A){//检测第一位是否为0x5A
            if (size >= 16) {//返回的数据最小长度为16位,如果大小小于16则数据不完整等待再次读取
                SeerHeader* header = new SeerHeader;
                memcpy(header, message.data(), 16);

                uint32_t data_size;//返回所有数据总长值
                uint16_t revCommand;//返回报文数据类型
                uint16_t number;//返回序号
                qToBigEndian(header->m_length,(uint8_t*)&(data_size));
                qToBigEndian(header->m_type, (uint8_t*)&(revCommand));
                qToBigEndian(header->m_number, (uint8_t*)&(number));
                delete header;

                int remaining_size = size - 16;//所有数据总长度 - 头部总长度16 = 数据区长度
                //如果返回数据长度值 大于 已读取数据长度 表示数据还未读取完整，跳出循环等待再次读取.
                if (data_size > remaining_size) {
                    _lastMessage = message;

                    break;
                }else{//返回数据长度值 大于等于 已读取数据，开始解析
                    QByteArray tempMessage;
                    if(_lastMessage.isEmpty()){
                        tempMessage = message;
                    }else{
                        tempMessage = _lastMessage;
                    }
                    QByteArray headB = message.left(16);
                    //截取报头16位后面的数据区数据
                    QByteArray json_data = message.mid(16,data_size);
                    qDebug()<<"rev:"<<QString(json_data)<<"  Hex:"<<json_data.toHex();
                    //--------------------------------------
                    QString dataHex = "";
                    if(size<=2048){
                        dataHex = hexToQString(json_data.toHex());
                    }else{
                        dataHex = tr("数据大于2048字节，不打印信息.");
                    }
                    //输出打印信息
                    QString info = QString("%1--------- 响应 ---------\n"
                                           "报文类型:%2 (%3) \n"
                                           "序号: %4 (0x%5)\n"
                                           "头部十六进制: %6\n"
                                           "数据区[size:%7 (0x%8)]: %9 \n"
                                           "数据区十六进制: %10 \n")
                            .arg(getCurrentDateTime())
                            .arg(revCommand)
                            .arg(QString::number(revCommand,16))
                            .arg(number)
                            .arg(QString::number(number,16))
                            .arg(hexToQString(headB.toHex()))
                            .arg(json_data.size())
                            .arg(QString::number(json_data.size(),16))
                            .arg(QString(json_data))
                            .arg(dataHex);

                    emit sigPrintInfo(info);
                    int msTime = _time.elapsed();
                    //----------------------------------------
                    //输出返回信息
                    emit sigChangedText(true,revCommand,
                                        json_data,json_data.toHex(),
                                        number,msTime);
                    //截断message,清空_lastMessage
                    message = message.right(remaining_size - data_size);
                    size = message.size();
                    _lastMessage.clear();
                }

            } else{
                _lastMessage = message;
                break;
            }
        }else{
            //报头数据错误
            setLastError("Seer Header Error !!!");
            message = message.right(size - 1);
            size = message.size();
            int msTime = _time.elapsed();
            emit sigChangedText(false,_oldSendCommand,
                                _lastMessage,_lastMessage.toHex(),
                                0,msTime);
        }
    }
}

int SCStatusTcp::getTimeOut() const
{
    return _timeOut;
}

void SCStatusTcp::setTimeOut(int timeOut)
{
    _timeOut = timeOut;
}


QTcpSocket *SCStatusTcp::tcpSocket() const
{
    return _tcpSocket;
}

QTime SCStatusTcp::time() const
{
    return _time;
}

void SCStatusTcp::setLastError(const QString &lastError)
{
    _lastError = lastError;
}

QString SCStatusTcp::lastError() const
{
    return _lastError;
}
/** 获取当前时间
 * @brief SCStatusTcp::getCurrentDateTime
 * @return
 */
QString SCStatusTcp::getCurrentDateTime()const
{
    return QDateTime::currentDateTime().toString("[yyyyMMdd|hh:mm:ss:zzz]:");
}
/** 16进制全部显示大写
 * @brief SCStatusTcp::hexToQString
 * @param b
 * @return
 */
QString SCStatusTcp::hexToQString(const QByteArray &b)
{
    QString str;
    for(int i=0;i<b.size();++i){
        ////        //每2位加入 空格0x
        ////        if((!(i%2)&&i/2)||0==i){
        ////            str+= QString(" 0x");
        ////        }
        str +=QString("%1").arg(b.at(i));
    }
    str = str.toUpper();
    return str;
}

/**************************************
 * 文件传输
 * *******************************/


void SCStatusTcp::SendFile(QString fileName)//发送文件名到服务端
{
    this->fileName = fileName;
   // connectToHost(serverIP, serverPort);
    //connectToHost(QHostAddress("127.0.0.1"), 19211);

}

void SCStatusTcp::SendData()
{
    emit message("与服务器建立连接成功");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"fileName"<<fileName;
        emit message("文件不能打开进行读取");
       _tcpSocket->disconnectFromHost();
        return;
    }
    else
    {
        emit fileSize(file.size());
    }

    qint64 size;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    QString name =SCTcpToolWidget::GetFileNameWithExtension(fileName);

    //写入开始符及文件名称
    emit message("发送开始符及文件名称");
    block.clear();
    out.device()->seek(0);
    out << 0x01 << name.toUtf8();
    size = block.size();
    _tcpSocket->write((char *)&size, sizeof(qint64));
    _tcpSocket->write(block.data(), size);

    if(!_tcpSocket->waitForBytesWritten(-1)) {
        emit message(QString("发送开始符数据发生错误:%1").arg(_tcpSocket->errorString()));
        _tcpSocket->disconnectFromHost();
        return;
    }

    //写入文件大小
    emit message("发送文件大小");
    block.clear();
    out.device()->seek(0);
    out << 0x02 << QString::number(file.size()).toUtf8();
    size = block.size();
    _tcpSocket->write((char *)&size, sizeof(qint64));
    _tcpSocket->write(block.data(), size);

    if(!_tcpSocket->waitForBytesWritten(-1)) {
        emit message(QString("发送文件大小数据发生错误:%1").arg(_tcpSocket->errorString()));
        _tcpSocket->disconnectFromHost();
        return;
    }

    //循环写入文件数据
    do {
        block.clear();
        out.device()->seek(0);
        //每次最多读取0xFFFF 即65535个字节发送,对于大文件如果一次性读取完内存不一定吃得消
        //每次发送的文件数据都带上一个0x02标识符
        out << 0x03 << file.read(0xFFFF);
        size = block.size();
        emit message(QString("当前发送数据大小:%1字节").arg(size));
        _tcpSocket->write((char *)&size, sizeof(qint64));
       _tcpSocket->write(block.data(), size);
        if(!_tcpSocket->waitForBytesWritten(-1)) {
            emit message(QString("发送文件数据发生错误:%1").arg(_tcpSocket->errorString()));
            _tcpSocket->disconnectFromHost();
            return;
        }
    } while(!file.atEnd());

    //写入结束符及文件名称
    emit message("发送结束符及文件名称");
    block.clear();
    out.device()->seek(0);
    out << 0x04 << name.toUtf8() ;
    size = block.size();
    _tcpSocket->write((char *)&size, sizeof(qint64));
    _tcpSocket->write(block.data(), size);

    if(!_tcpSocket->waitForBytesWritten(-1)) {
        emit message(QString("发送结束符数据发生错误:%1").arg(_tcpSocket->errorString()));
       _tcpSocket->disconnectFromHost();
        return;
    }

    emit message("发送文件完毕,等待服务器断开连接");
    _tcpSocket->waitForDisconnected();
    emit message("客户端主动断开连接");
    _tcpSocket->disconnectFromHost();
}

void SCStatusTcp::displaySocketError(QAbstractSocket::SocketError )
{
    emit message(QString("发生错误:%1").arg(_tcpSocket->errorString()));
}

