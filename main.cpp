#include <QCoreApplication>
#include "qmikrotikapi.h"
#include <QDebug>
#include <QByteArrayList>
#include <QByteArray>
#include "tmikrotikapi_pppoe.h"
#include "tclient.h"
#include <QThread>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TmikrotikAPI_PPPoE m;
    m.open("172.16.0.83","admin","123");
    Tclient c;
    //qDebug()<<m.add(c);
    QMap<QByteArray,QByteArray> dd = m.info();
    qDebug()<<dd;
    return a.exec();
}
