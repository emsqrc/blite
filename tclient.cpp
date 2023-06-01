#include "tclient.h"

Tclient::Tclient()
{

}

QByteArray Tclient::ip()
{
    return "10.1.0.2";
}

QByteArray Tclient::login()
{
    return "10002";
}

QByteArray Tclient::passwd()
{
    return "123";
}

QByteArray Tclient::localip()
{
    return "10.1.0.1";
}

QByteArray Tclient::up()
{
    return "20";
}

QByteArray Tclient::down()
{
    return "50";
}

