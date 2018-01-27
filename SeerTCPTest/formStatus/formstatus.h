#ifndef FORMSTATUS_H
#define FORMSTATUS_H

#include <QWidget>

namespace Ui {
class Formstatus;
}

class Formstatus : public QWidget
{
    Q_OBJECT

public:
    explicit Formstatus(QWidget *parent = 0);
    ~Formstatus();
public slots:
    void updateSendPBar_(qint64 size,qint64 sendBlockNumber_,qint64 sendMaxBytes_);
    void updateSendStatus_(QString );
    void setSendPBar_(qint64 );
    void sendFinsh_();
    QString getfileName(QString fileName);
private:
    Ui::Formstatus *ui;

    int sendBytes_;
    //qint64 sendBlockNumber_;
    //qint64 sendMaxBytes_;

};

#endif // FORMSTATUS_H
