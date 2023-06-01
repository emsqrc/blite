#ifndef TMIKROTIKAPI_PPPOE_H
#define TMIKROTIKAPI_PPPOE_H

#define LISTNAME "WhiteList"

#include "qmikrotikapi.h"
#include "trouter.h"
#include "tmikrotikapi_base.h"
#include <QMap>


class TmikrotikAPI_PPPoE : public Trouter, TmikrotikAPI_base
{
    QMikrotikAPI *mk;
    QByteArray byf;
public:
    TmikrotikAPI_PPPoE();
    bool open(QByteArray ip, QByteArray login, QByteArray passwd);
    void close();
    bool connected();
    bool add(Tclient &c);
    bool del(Tclient &c);
    bool enable(Tclient &c);
    bool disable(Tclient &c);
    QByteArray info(Tclient &c);
    QMap<QByteArray, QByteArray> info();
    QByteArray routerinfo();
    ~TmikrotikAPI_PPPoE();
};

#endif // TMIKROTIKAPI_PPPOE_H
