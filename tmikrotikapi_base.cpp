#include "tmikrotikapi_base.h"
#include <QByteArrayList>


TmikrotikAPI_base::TmikrotikAPI_base()
{

}

QByteArray TmikrotikAPI_base::binfo(QMikrotikAPI *m)
{
    QByteArrayList r;
    r<<"/system/routerboard/print";
    return m->request(r);
}

QByteArray TmikrotikAPI_base::bget(QByteArray byf, QByteArray key)
{
    int i=0, n=0;
    QByteArray temp;
    while(i<byf.size()){
        n=byf.at(i);
        temp=byf.mid(i+1,n);
        i=i+n+1;
        if(temp.contains(key)){
            int k=temp.indexOf("=");
            temp.remove(0,k+1);
            k=temp.indexOf("=");
            temp.remove(0,k+1);
            return temp;
        }
    }
    return "";
}

bool TmikrotikAPI_base::bdone(QByteArray byf)
{
    int i=0, n=0;
    QByteArray temp;
    while(i<byf.size()){
        n=byf.at(i);
        temp=byf.mid(i+1,n);
        i=i+n+1;
        if(temp.contains("!trap")) return false;
    }
    i=0;
    while(i<byf.size()){
        n=byf.at(i);
        temp=byf.mid(i+1,n);
        i=i+n+1;
        if(temp.contains("!done")) return true;
    }
    return false;
}

QByteArrayList TmikrotikAPI_base::bdecompose(QByteArray byf)
{
    QByteArray temp = byf;
    QByteArrayList r;
    int n=temp.indexOf("\x03!re",strlen("\x03!re"));
    while(n>0){
        r.push_back(temp.left(n));
        temp.remove(0,n);
        n=temp.indexOf("\x03!re",strlen("\x03!re"));
    }
    r.push_back(temp);
    return r;
}
