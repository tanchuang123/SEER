#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "receivefileserver.h"
#include <QMainWindow>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void InitForm();
private slots:
    //void on_btnListen_clicked();
    void updateReceivePBar(qint64 size);
    void updateReceiveStatus(QString msg);
    void setReceivePBar(qint64 size);
    void receiveFinsh();
    void receiveFileName(QString name);
private:
    Ui::MainWindow *ui;
    ReceiveFileServer *receive;
    int port;
    int receiveBytes;
    qint64 receiveBlockNumber;
    qint64 receiveMaxBytes;
   // bool ok;
};

#endif // MAINWINDOW_H
