#include "tmikrotikapi_pppoe.h"

TmikrotikAPI_PPPoE::TmikrotikAPI_PPPoE()
{
    mk=0;
}

bool TmikrotikAPI_PPPoE::open(QByteArray ip, QByteArray login, QByteArray passwd)
{
    mk=new QMikrotikAPI(ip,login,passwd);
    return connected();
}

void TmikrotikAPI_PPPoE::close()
{
    if(mk->isLogined()) mk->quit();
    delete mk;
}

bool TmikrotikAPI_PPPoE::connected()
{
    return mk->isLogined();
}

bool TmikrotikAPI_PPPoE::add(Tclient &c)
{
    QByteArrayList temp;
    bool r=true;
    temp.clear();
    temp.push_back("/ppp/secret/add");
    temp.push_back("=name="+c.login());
    temp.push_back("=password="+c.passwd());
    temp.push_back("=local-address="+c.localip());
    temp.push_back("=remote-address="+c.ip());
    temp.push_back("=comment="+c.login());
    byf=mk->request(temp);
    r&=bdone(byf);

    temp.clear();
    temp.push_back("/ip/firewall/address-list/add");
    temp.push_back("=address="+c.ip());
    temp.push_back("=list="+(QByteArray)LISTNAME);
    temp.push_back("=comment="+c.login());
    byf=mk->request(temp);
    r&=bdone(byf);

    temp.clear();
    temp.push_back("/queue/simple/add");
    temp.push_back("=name="+c.login());
    temp.push_back("=target="+c.ip());
    temp.push_back("=max-limit="+c.up()+"/"+c.down());
    temp.push_back("=comment="+c.login());
    byf=mk->request(temp);
    r&=bdone(byf);
    return r;
}

bool TmikrotikAPI_PPPoE::del(Tclient &c)
{
    QByteArrayList temp;
    bool r=true;
    temp.clear();
    temp.push_back("/ppp/secret/print");
    temp.push_back("?comment="+c.login());
    byf=mk->request(temp);
    QByteArray str=bget(byf,".id");
    if(str!=""){
        temp.clear();
        temp.push_back("/ppp/secret/remove");
        temp.push_back("=numbers="+str);
        byf=mk->request(temp);
        r&=bdone(byf);
    }else r=false;

    temp.clear();
    temp.push_back("/ip/firewall/address-list/print");
    temp.push_back("?comment="+c.login());
    byf=mk->request(temp);
    str=bget(byf,".id");
    if(str!=""){
        temp.clear();
        temp.push_back("/ip/firewall/address-list/remove");
        temp.push_back("=numbers="+str);
        byf=mk->request(temp);
        r&=bdone(byf);
    }else r=false;

    temp.clear();
    temp.push_back("/queue/simple/print");
    temp.push_back("?comment="+c.login());
    byf=mk->request(temp);
    str=bget(byf,".id");
    if(str!=""){
        temp.clear();
        temp.push_back("/queue/simple/remove");
        temp.push_back("=numbers="+str);
        byf=mk->request(temp);
        r&=bdone(byf);
    }else r=false;

    return r;
}

bool TmikrotikAPI_PPPoE::enable(Tclient &c)
{
    QByteArrayList temp;
    temp.clear();
    temp.push_back("/ip/firewall/address-list/print");
    temp.push_back("?comment="+c.login());
    byf=mk->request(temp);
    QByteArray str=bget(byf,".id");
    if(str!=""){
        temp.clear();
        temp.push_back("/ip/firewall/address-list/enable");
        temp.push_back("=numbers="+str);
        byf=mk->request(temp);
        return bdone(byf);
    }
    return false;
}

bool TmikrotikAPI_PPPoE::disable(Tclient &c)
{
    QByteArrayList temp;
    temp.clear();
    temp.push_back("/ip/firewall/address-list/print");
    temp.push_back("?comment="+c.login());
    byf=mk->request(temp);
    QByteArray str=bget(byf,".id");
    if(str!=""){
        temp.clear();
        temp.push_back("/ip/firewall/address-list/disable");
        temp.push_back("=numbers="+str);
        byf=mk->request(temp);
        return bdone(byf);
    }
    return false;
}

QByteArray TmikrotikAPI_PPPoE::info(Tclient &c)
{
    QByteArrayList temp;
    temp.clear();
    temp.push_back("/ppp/active/print");
    temp.push_back("?comment="+c.login());
    byf=mk->request(temp);
    QByteArray str=bget(byf,"=uptime=");
    return str;
}

QMap<QByteArray, QByteArray> TmikrotikAPI_PPPoE::info()
{
    QByteArrayList temp;
    QMap<QByteArray, QByteArray> r;
    temp.clear();
    temp.push_back("/ppp/active/print");
    byf=mk->request(temp);
    temp.clear();
    temp=bdecompose(byf);
    for(int i=0; i<temp.size(); i++){
        r.insert(bget(temp[i],"=name="),bget(temp[i],"=uptime="));
    }
    return r;
}






QByteArray TmikrotikAPI_PPPoE::routerinfo()
{
    byf.clear();
    byf=binfo(mk);
    QByteArray temp;
    temp.push_back(bget(byf,"board-name"));
    temp.push_back(" ");
    temp.push_back(bget(byf,"model"));
    temp.push_back(" ");
    temp.push_back(bget(byf,"current-firmware"));
    return temp;
}



TmikrotikAPI_PPPoE::~TmikrotikAPI_PPPoE()
{
    if(mk!=0) close();
}
