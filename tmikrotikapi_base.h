#ifndef TMIKROTIKAPI_BASE_H
#define TMIKROTIKAPI_BASE_H

#include <QByteArray>
#include "qmikrotikapi.h"
#include <QByteArrayList>


class TmikrotikAPI_base
{
public:
    TmikrotikAPI_base();
    QByteArray binfo(QMikrotikAPI *m);
    QByteArray bget(QByteArray byf, QByteArray key);
    bool bdone(QByteArray byf);
    QByteArrayList bdecompose(QByteArray byf);
};

#endif // TMIKROTIKAPI_BASE_H
