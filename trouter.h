#ifndef TROUTER_H
#define TROUTER_H

#include <QByteArray>
#include <QByteArrayList>
#include "tclient.h"
#include <QMap>



class Trouter
{
    QByteArray _ip, _login, _passwd;
public:
    Trouter();
    bool open(QByteArray ip, QByteArray login, QByteArray passwd);
    void close();
    bool connected();
    bool add(Tclient &c);
    bool del(Tclient &c);
    bool enable(Tclient &c);
    bool disable(Tclient &c);
    QByteArray info(Tclient &c);
    QMap<QByteArray,QByteArray> info();
    QByteArray routerinfo();
};

#endif // TROUTER_H
