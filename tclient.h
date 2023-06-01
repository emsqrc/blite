#ifndef TCLIENT_H
#define TCLIENT_H

#include <QByteArray>

class Tclient
{
public:
    Tclient();
    QByteArray ip();
    QByteArray login();
    QByteArray passwd();
    QByteArray localip();
    QByteArray up();
    QByteArray down();
};

#endif // TCLIENT_H
