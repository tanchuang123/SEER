#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InitForm();

    receive->listen(QHostAddress::Any, ui->comboBox_Port->currentText().toInt());
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::InitForm()
{

    receiveBytes = 0;
    receiveBlockNumber = 0;
    receiveMaxBytes = 0;

    ui->pbarReceiveData->setRange(0, 100);
    ui->pbarReceiveData->setValue(0);

    receive = new ReceiveFileServer(this);
    connect(receive, SIGNAL(finished()), this, SLOT(receiveFinsh()));
    connect(receive, SIGNAL(message(QString)), this, SLOT(updateReceiveStatus(QString)));
    connect(receive, SIGNAL(receiveFileName(QString)), this, SLOT(receiveFileName(QString)));
    connect(receive, SIGNAL(receiveFileSize(qint64)), this, SLOT(setReceivePBar(qint64)));
    connect(receive, SIGNAL(receiveData(qint64)), this, SLOT(updateReceivePBar(qint64)));
}

//void MainWindow::on_btnListen_clicked()
//{
//    port=ui->comboBox_Port->currentText().toInt();
//    if (port)
//    {
//        if (ui->btnListen->text() == "开始监听")
//        {
//            bool ok = receive->startServer(port);
//            if (ok)
//            {
//                ui->txtReceiveStatus->setText("监听成功");
//                ui->btnListen->setText("停止监听");
//            }
//            else
//            {
//                ui->txtReceiveStatus->setText("监听失败");
//            }
//        }
//        else
//        {
//            receive->stopServer();
//            ui->txtReceiveStatus->setText("停止监听成功");
//            ui->btnListen->setText("开始监听");
//        }
//    }
//    ui->pbarReceiveData->setRange(0, 100);
//    ui->pbarReceiveData->setValue(0);
//}

void MainWindow::updateReceivePBar(qint64 size)
{
    receiveBlockNumber ++;
    receiveBytes += size;
    ui->pbarReceiveData->setValue(receiveBytes);
    QString msg = QString("已接收数据包:%1个 当前数据包大小:%2字节 已接收字节:%3 总共字节:%4")
                  .arg(receiveBlockNumber).arg(size).arg(receiveBytes).arg(receiveMaxBytes);
    ui->txtReceiveStatus->setText(msg);
    qApp->processEvents();
}
void MainWindow::updateReceiveStatus(QString msg)
{
   // qDebug() << "接收文件服务端:" << msg<<;
}

void MainWindow::setReceivePBar(qint64 size)
{
    receiveBytes = 0;
    receiveBlockNumber = 0;
    receiveMaxBytes = size;
    ui->pbarReceiveData->setRange(0, size - 1);
    ui->pbarReceiveData->setValue(0);
}

void MainWindow::receiveFinsh()
{
    ui->pbarReceiveData->setRange(0, 100);
    ui->pbarReceiveData->setValue(100);
    ui->txtReceiveStatus->setText("接收文件成功");
}

void MainWindow::receiveFileName(QString name)
{
    ui->pbarReceiveData->setRange(0, 100);
    ui->pbarReceiveData->setValue(100);
    ui->txtReceiveFile->setText(name);
}
