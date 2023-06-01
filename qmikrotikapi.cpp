#include "qmikrotikapi.h"

// constructor
//-----------------------------
// ip - IP of Mikrotik router
// uname - user name
// pwd - user password
// port - Mikrotik connection port number
QMikrotikAPI::QMikrotikAPI(QByteArray ip, QByteArray uname, QByteArray pwd, int port )
{
    tcpSocket = new QTcpSocket(this);
    loginSt = false;    // while no realy login

    // conn to routeros host
    tcpSocket->connectToHost(ip,port);  // port = 8728
    //qDebug() << tcpSocket->state();

    if (!tcpSocket->waitForConnected()) {
        emit error(tcpSocket->error(), tcpSocket->errorString());
        qCritical() << tcpSocket->error() <<  tcpSocket->errorString(); // DBG
        return;
    }
    // get login
    username = uname;
    password = pwd;
    sendLoginReq();
    connect(tcpSocket,SIGNAL(readyRead()),this,SIGNAL(rawDataReady()));

    return;
}

/* destructor */
QMikrotikAPI::~QMikrotikAPI(){
    quit();
    close();
    delete tcpSocket;
}

//send data to RouterOS host
// data_BA  -byte array of data
// end - if true - it is final sending data with '\0'
void
QMikrotikAPI::send(QByteArray data_BA, bool end)
{
    // no send() if no login
    if( !loginSt){
        return;
    }

    QByteArray data_size_BA = EncodeLength(data_BA.length());
    tcpSocket->write(data_size_BA,data_size_BA.length());
    tcpSocket->write(data_BA,data_BA.length());

    if(end){
        //write null word
        tcpSocket->write( "\0",1 );
    }
    if(!tcpSocket->waitForBytesWritten()){
        emit error(tcpSocket->error(), tcpSocket->errorString());
        return;
    }
    retBA.clear();  // UNUSED
}

// read all data from RouterOS host. If tcp socket error - return 'false' !
QByteArray
QMikrotikAPI::readAllData(){
    QByteArray ba;
    if(loginSt){
        if (!tcpSocket->waitForReadyRead()) {
            emit error(tcpSocket->error(), tcpSocket->errorString());
            return NULL;
        }
        while (tcpSocket->bytesAvailable() > 0 ) {
            ba.append(tcpSocket->readAll());
            if(ba.contains("!done")){
                break;
            }
            if (!tcpSocket->waitForReadyRead(1000)) {
                break;
            }
        }
    }
    return ba;  // if error - ba.isEmpty()
}

// quit connection with RouterOS host.
bool
QMikrotikAPI::quit(){
    if( !loginSt){
        return false;
    }
    QByteArrayList bal;
    bal << "/quit";
    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){
        loginSt = false;
        return true;
    }
    else {
        return false;
    }
}


// get info about ip address by ip address value and/or network ip or all interfaces.
// ipAddr param must be at "192.168.0.3/24" | NULL format, ipNetw param is "192.168.0.3" | NULL
// if address and/or network not found - return empty QByteArray(NULL).
//
// ip - IP
// netw - network
// interface - interface of RouterOS
QByteArray
QMikrotikAPI::get_ip_address_info(QByteArray ip, QByteArray netw, QByteArray interface){
    QByteArrayList bal;
    bal << "/ip/address/print";     //req;
    if( !ip.isEmpty()){
        bal << "?=address=" + ip;
    }
    if( !netw.isEmpty()){
        bal << "?=network=" + netw;
    }
    if( !interface.isEmpty()){
        bal << "?=interface=" + interface;
    }

    QByteArray ba = request(bal);

    if( !ip.isEmpty() && !netw.isEmpty() && !interface.isEmpty()){
        QByteArray ipkey = "=address=" + ip;
        QByteArray netwkey = "=network=" + netw;
        QByteArray interfaceKey = "=interface=" + interface;
        if( ba.contains(ipkey) &&  ba.contains(netwkey) && ba.contains(interfaceKey) && !ba.contains("!trap") ){
            return ba;
        }else{
            return NULL;
        }
    }else if(!ip.isEmpty()){
        QByteArray ipkey = "=address=" + ip;
        if( ba.contains(ipkey) && !ba.contains("!trap") ){
            return ba;
        }else{
            return NULL;
        }
    }else if(!netw.isEmpty()){
        QByteArray netwkey = "=network=" + netw;
        if( ba.contains(netwkey) && !ba.contains("!trap") ){
            return ba;
        }else{
            return NULL;
        }
    }else if(!interface.isEmpty()){
        QByteArray interfaceKey = "=interface=" + interface;
        if( ba.contains(interfaceKey) && !ba.contains("!trap") ){
            return ba;
        }else{
            return NULL;
        }
    }
    return ba;
}

// get ip address id by ip or/and network or/and interface
//
// ip - IP
// netw - network
// interface - interface of RouterOS
QByteArray
QMikrotikAPI::get_ip_address_id(QByteArray ip, QByteArray netw, QByteArray interface){
    QByteArray ipAddrInfo = get_ip_address_info(ip,netw,interface);
    if(!ipAddrInfo.isEmpty()){
        QByteArrayList infobalist = this->clearData(ipAddrInfo);
        foreach (QByteArray b, infobalist) {
            if(b.contains("=.id=")){
                b.replace("=.id=","");
                return b;
                //break;
            }
        }
    }
    return NULL;
}

// delete ip by id
// "/ip/address/remove"  +
// =.id=*236 (USED REAL ID)
// !!! NO "?=....=" ONLY "=....=" for actions, "?" - for requests !!!
// ip as "10.10.0.1/24" , netw as "10.10.0.0" interface as "ether0"
//
// ip - IP
// netw - network
// interface - interface of RouterOS
bool
QMikrotikAPI::ip_address_remove(QByteArray ip, QByteArray netw, QByteArray interface){
    QByteArrayList bal;
    bal << "/ip/address/remove";
    bal <<"=.id=" + get_ip_address_id(ip,netw,interface);
    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){  //
        return true;
    }
    else {
        return false;
    }
}

// add ip addres in format "192.168.0.33/32" & network addr for interface
//
// ip - IP
// network - network
// interface - interface of RouterOS
bool
QMikrotikAPI::ip_address_add(QByteArray ip, QByteArray network, QByteArray interface){
    QByteArrayList bal;
    bal << "/ip/address/add";
    bal <<"=address=" + ip;
    bal <<"=interface=" + interface;
    bal <<"=network=" + network;
    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){
        return true;
    }
    else {
        return false;
    }
}

//    Add rule allowing access to the internal server from external networks:
//    /ip firewall nat add chain=dstnat dst-address=10.5.8.200 action=dst-nat to-addresses=192.168.0.109
bool
QMikrotikAPI::ip_firewall_nat_add_dstnat(QByteArray dst_addr, QByteArray to_addr)
{
    QByteArrayList bal;
    bal << "/ip/firewall/add";
    bal <<"=chain=dstnat" ;
    bal <<"=dst-address=" + dst_addr;   // public IP
    bal <<"=action=dst-nat";
    bal <<"=to-addresses=" + to_addr;   // local IP
    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){
        return true;
    }
    else {
        return false;
    }
}

//    Add rule allowing the internal server to initate connections to the outer networks having its source address translated to 10.5.8.200:
//    /ip firewall nat add chain=srcnat src-address=192.168.0.109 action=src-nat to-addresses=10.5.8.200
bool
QMikrotikAPI::ip_firewall_nat_add_srcnat(QByteArray src_addr, QByteArray to_addr)
{
    QByteArrayList bal;
    bal << "/ip/firewall/add";
    bal <<"=chain=srcnat";
    bal <<"=src-address=" + src_addr;   // local IP
    bal <<"=action=src-nat";
    bal <<"=to-addresses=" + to_addr;   // public IP
    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){
        return true;
    }
    else {
        return false;
    }
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~ /queue/simple ~~~~~~~~~~~~~~~~~~~~~~~~~~//

// /queue simple add
//
// example from Mikrotik API
// /queue simple add target-addresses=192.168.11.1/32 limit-at=32000/32000 max-limit=64000/42000 time=00:00:00-00:00:00,mon,tue,wed,thu,fri
// for call this need fill QByteArrayList with parameters:
//
// QByteArrayList params;
//params <<"=target=" + "...";
//params <<"=limit-at=" + "...";
//params <<"=max-limit=" + "...";
//params <<"=name="+"queue_name";   etc

bool
QMikrotikAPI::queue_simple_add(QByteArrayList params){
    if(params.isEmpty()){
        return false;
    }
    QByteArrayList bal;
    bal << "/queue/simple/add";
    bal << params;
    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){  //
        return true;
    }
    else {
        return false;
    }
}

// /queue simple add
//
// FOR EXAMPLE:
// target-addresses=192.168.11.1/32 , limit-at=32000/32000, queue=default/default(from /queue type) ,time=00:00:00-00:00:00,mon,tue,wed,thu,fri
// total_queue = "wireless-default" (from /queue type)
bool
QMikrotikAPI::queue_simple_add(QByteArray target, QByteArray limit_at, QByteArray max_limit, QByteArray name, QByteArray queue, QByteArray total_queue, QByteArray priority, QByteArray time){

    QByteArrayList bal;

    bal << "/queue/simple/add";
    bal << "=target=" + target;
    bal << "=limit-at=" + limit_at;
    bal << "=max-limit=" + max_limit;
    if( !name.isEmpty()){
        bal << "=name=" + name;
    }
    if( !queue.isEmpty()){
        bal << "=queue=" + queue;
    }
    if( !total_queue.isEmpty()){
        bal << "=total-queue=" + total_queue;
    }
    if( !priority.isEmpty()){

        bal << "=priority=" + priority;
    }
    if( !time.isEmpty()){

        bal << "=time=" + time;
    }

    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){  //
        return true;
    }
    else {
        return false;
    }
}

// get queue simple info by queue name
QByteArray
QMikrotikAPI::get_queue_simple_info_by_name(QByteArray queue_s_Name){
    return get_interface_info(queue_s_Name,"/queue/simple/print");
}


// get queue simple id by queue name
QByteArray
QMikrotikAPI::get_queue_simple_id_by_name(QByteArray queue_s_Name){
    QByteArray qsInfo = get_queue_simple_info_by_name(queue_s_Name);
    //qDebug() << "get_vlan_id_by_name() " << vlanInfo;
    if(!queue_s_Name.isEmpty()){
        QByteArrayList infobalist = this->clearData(qsInfo);
        foreach (QByteArray b, infobalist) {
            if(b.contains("=.id=")){
                b.replace("=.id=","");
                return b;
            }
        }
    }
    return NULL;
}


// get parameter 'par' for queue simple with name 'qsName'
// !!! parameter must be in format 'target' but not '=target=' !!!
QByteArray
QMikrotikAPI::queue_simple_get_parameter(QByteArray qsName,QByteArray par){
    QByteArray qsInfo = get_queue_simple_info_by_name(qsName);
    if(!qsInfo.isEmpty()){
        QByteArrayList infobalist = this->clearData(qsInfo);
        foreach (QByteArray b, infobalist) {
            if(b.contains("=" + par + "=")){
                b.replace("=" + par + "=","");
                return b;
            }
        }
    }
    return NULL;
}


// set parameter 'par' into value 'val' for queue simple with name 'qsName'
// !!! parameter must be in format 'target' but not '=target=' !!!
bool
QMikrotikAPI::queue_simple_set(QByteArray qsName, QByteArray par,QByteArray val)
{
    QByteArray id = get_queue_simple_id_by_name(qsName);
    if( !id.isEmpty() ){
        QByteArrayList bal;
        bal << "/queue/simple/set";
        bal << "=.id=" + id;
        bal << "=" + par + "=" + val;
        QByteArray ba = request(bal);
        if( !ba.contains("!trap") ){
            return true;
        }
    }
    return false;
}


// /queue simple remove by queue name - queue_s_Name
bool
QMikrotikAPI::queue_simple_remove_by_name(QByteArray queue_s_Name){
    if(queue_s_Name.isEmpty()){
        return false;
    }

    QByteArrayList bal;
    bal << "/queue/simple/remove";
    bal <<"=.id=" + get_queue_simple_id_by_name(queue_s_Name);
    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){  //
        return true;
    }
    else {
        return false;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~ END /queue/simple END ~~~~~~~~~~~~~~~~~~~~~~//


//~~~~~~~~~~~~~~~~~~~~~~~~~ /interface/vlan ~~~~~~~~~~~~~~~~~~~~~~~~~//


// get vlan info by vlan name | all vlans if vlanName is NULL ,
// if not exists - NULL QByteArray returned
QByteArray
QMikrotikAPI::get_vlan_info_by_name(QByteArray vlanName){
    return get_interface_info(vlanName,"/interface/vlan/print");
}

// return vlan id (=.id=) by vlan name | NULL QBArray if not exists | other err
QByteArray
QMikrotikAPI::get_vlan_id_by_name(QByteArray vlanName){
    QByteArray vlanInfo = get_vlan_info_by_name(vlanName);

    if(!vlanInfo.isEmpty()){
        QByteArrayList infobalist = this->clearData(vlanInfo);
        foreach (QByteArray b, infobalist) {
            if(b.contains("=.id=")){
                b.replace("=.id=","");
                return b;
                //break;
            }
        }
    }
    return NULL;
}


// get parameter 'par' for vlan with name 'vlanName'
// !!! parameter must be in format 'target' but not '=target=' !!!
QByteArray
QMikrotikAPI::get_vlan_parameter(QByteArray vlanName, QByteArray par)
{
    QByteArray vlanInfo = get_vlan_info_by_name(vlanName);
    if(!vlanInfo.isEmpty()){
        QByteArrayList infobalist = this->clearData(vlanInfo);
        foreach (QByteArray b, infobalist) {
            if(b.contains("=" + par + "=")){
                b.replace("=" + par + "=","");
                return b;
            }
        }
    }
    return NULL;
}

// set vlan parameter
// 'par' must be is 'disabled' BUT NO '=disabled=' !!!
// for ex. 'vlan100','disabled','true' - set vlan100 to the disabled
//
// vlanName - vlan name
// par - parameter
// val - parameter value
bool
QMikrotikAPI::vlan_set(QByteArray vlanName, QByteArray par, QByteArray val)
{
    QByteArray id = get_vlan_id_by_name(vlanName);
    if( !id.isEmpty() ){
        QByteArrayList bal;
        bal << "/interface/vlan/set";
        bal << "=.id=" + id;
        bal << "=" + par + "=" + val;
        QByteArray ba = request(bal);
        if( !ba.contains("!trap") ){
            return true;
        }
    }
    return false;
}

// add vlan by name & vlanID
//
// vlanName - vlan name
// vlanID - vlan ID
// interface - interface
// proxy_arp = "proxy-arp" - proxy arp, default is 'proxy-arp'
// l2mtu = "65531" - l2mtu , default is '65531'
bool
QMikrotikAPI::vlan_add(QByteArray vlanName, QByteArray vlanID, QByteArray interface, QByteArray proxy_arp, QByteArray l2mtu){
    QByteArrayList bal;
    bal << "/interface/vlan/add";
    bal <<"=arp=" + proxy_arp;
    bal <<"=interface=" + interface;
    bal <<"=l2mtu=" + l2mtu;    //65531
    bal <<"=name="+vlanName;
    bal <<"=vlan-id="+vlanID;
    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){  //
        return true;
    }
    else {
        return false;
    }
}

// delete vlan by name
// "/interface/vlan/remove"  +
// =.id=*236(!!! REAL ID !!!)
// !!! NO "?=....=" ONLY "=....=" for actions, "?" - for requests !!!
// remove vlan by name & vlanID ???
//
// vlanName - vlan name
// vlanID - vlan ID
bool
QMikrotikAPI::vlan_remove(QByteArray vlanName, QByteArray vlanID){
    Q_UNUSED(vlanID);
    QByteArrayList bal;
    bal << "/interface/vlan/remove";
    bal <<"=.id=" + get_vlan_id_by_name(vlanName);
    QByteArray ba = request(bal);
    if( !ba.contains("!trap") ){
        return true;
    }
    else {
        return false;
    }
}


//~~~~~~~~~~~~~~~~~~~~~ END /interface/vlan END ~~~~~~~~~~~~~~~~~~~~~//


//~~~~~~~~~~~~~~~~~~~ get mikrotik RouterOS info ~~~~~~~~~~~~~~~~~~~~~//


// get interface by name if exists or all interfaces if exists
QByteArray QMikrotikAPI::get_interface_info(QByteArray ifName, QByteArray req){
    QByteArrayList bal;
    bal << req;
    if( !ifName.isEmpty()){
        bal << "?=name=" + ifName;
    }

    QByteArray ba = request(bal);
    //qDebug() << ba.size();

    if( !ifName.isEmpty()){
        QByteArray key = "=name=" + ifName;
        if( ba.contains(key) && !ba.contains("!trap")){
            //                qDebug() << "Key:" << key << "Exists !";
            return ba;
        }else{
            //                qDebug() << "Key:" << key << "NOT Exists !";
            return NULL;
        }
    }
    return ba;
}


//~~~~~~~~~~~~~~~ END get mikrotik RouterOS info END ~~~~~~~~~~~~~~~~~//


// send request sentence to routeros mikrotik
QByteArray
QMikrotikAPI::request(QByteArrayList &req_bal){
    foreach (QByteArray b, req_bal) {
        this->send(b);
    }
    this->send("",true);
    return this->readAllData();
}

//private
// process all login
bool
QMikrotikAPI::sendLoginReq(){
    loginSt = true; // tmp set true for login only for call send()
    send("/login", true);
    loginSt = false;

    //connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(slot_readyReadLoginRet() ));
    QByteArray hash;
    if (!tcpSocket->waitForReadyRead()) {
        emit error(tcpSocket->error(), tcpSocket->errorString());
        return false;
    }

    while (tcpSocket->bytesAvailable() > 0 ) {
        hash.append(tcpSocket->readAll());
        //hash.append(tcpSocket->read(tcpSocket->bytesAvailable()));
    }

    // remove '=ret=' from returned hash & get only hash string with 32 bytes length
    hash.remove(0,hash.indexOf("=ret=")+5);

    loginSt = true; // tmp set true for login only for call send()
    send("/login");
    send("=name=" + username);
    //send("=response=00" + EncodePassword(password, hash), true);
    send("=password="+password,true);
    loginSt = false;

    if (!tcpSocket->waitForReadyRead()) {
        emit error(tcpSocket->error(), tcpSocket->errorString());
        return false;
    }

    // read final login response
    hash.clear();
    while (tcpSocket->bytesAvailable() > 0 ) {
        hash.append(tcpSocket->readAll());
    }
    QByteArrayList bal = hash.split('\0');

    if (bal[0].indexOf("!done") != -1 )
    {
        loginSt = true;
    }
    else
    {
        loginSt = false;
    }
    return loginSt;

}

//private
QByteArray
QMikrotikAPI::EncodeLength(int intval)
{
    if (intval < 0x80)
    {
        //qDebug() << "EncodeLength BA length:" << Int2BigEndianBA(intval).remove(0,3).length();
        return (Int2BigEndianBA(intval).remove(0,3));
    }
    if (intval < 0x4000)
    {
        return (Int2BigEndianBA(intval | 0x8000).remove(0,2));
    }
    if (intval < 0x200000)
    {
        return (Int2BigEndianBA(intval | 0xC00000).remove(0,1));
    }
    if (intval < 0x10000000)
    {
        return Int2BigEndianBA(intval | 0xE0000000);
    }
    else
    {
        QByteArray tmp = Int2BigEndianBA(intval);
        const char c[] = { static_cast<char>(0xF0) };
        QByteArray ba(c);
        ba.append(tmp);
        return tmp;
    }
}

// get login state
bool
QMikrotikAPI::isLogined(){
    if( loginSt && (socketState() == tcpSocket->ConnectedState /*QAbstractSocket::SocketState::ConnectedState*/) ){
        return true;
    }
    return false;
}

// close connection
void
QMikrotikAPI::close()
{
    tcpSocket->close();
}

// clear raw data getting from mikrotik & return QByteArrayList with =key=value strings
// | NULL (empty) QBAList if error | if ba.isEmpty().
QByteArrayList
QMikrotikAPI::clearData(QByteArray &ba){
    QByteArrayList bal;
    if(ba.isEmpty()){
        return bal;
    }

    QByteArray size(4,0);
    int ind = 0;
    int count = 0;

    while(ind < ba.size()){
        size.fill(0);
        size[0] = ba.at(ind);

        if((int)size[0] == 0){
            ind++;
        }else if ( (int)size[0] < 0x80){

            // !!! IT WORK !!!
            count = size[0];
            bal << ba.mid(ind + 1,count);   // append next lex
            ind += count + 1;
            //qDebug() << "1 byte count: " << count << " size[0] = " << (int)size[0];

        }else if( (int)size[0] < 0xC0){
            size[1] = size[0];
            size[0] = ba.at(ind + 1);
            count = BA2Int_LittleEndian(size);
            count = count ^ 0x8000;
            bal << ba.mid(ind + 2,count);
            ind += count + 2;
            //qDebug() << "2 byte count: " << count;
        }
        else if ( (int)size[0] < 0xE0){
            size[2] = size[0];
            size[1] = ba.at(ind + 1);
            size[0] = ba.at(ind + 2);
            count = BA2Int_LittleEndian(size);
            count = count ^ 0xC00000;
            bal << ba.mid(ind + 3,count);
            ind += count + 3;
            //qDebug() << "3 byte count: " << count;
            //()
        }else if ( (int)size[0] < 0xF0){
            size[3] = size[0];
            size[2] = ba.at(ind + 1);
            size[1] = ba.at(ind + 2);
            size[0] = ba.at(ind + 3);
            count = BA2Int_LittleEndian(size);
            bal << ba.mid(ind + 4,count);
            ind += count + 4;
        }else if ( (int)size[0] == 0xF0){
            size[3] = ba.at(ind + 1);
            size[2] = ba.at(ind + 2);
            size[1] = ba.at(ind + 3);
            size[0] = ba.at(ind + 4);
            count = BA2Int_LittleEndian(size);
        }else{
            // packet reception error, unknown length
            bal.clear();    // return NULL QByteArrayList
            break;
        }
    }
    return bal;
}

//private
QByteArray
QMikrotikAPI::EncodePassword(QString Password, QString hash)
{
    QByteArray hash_BA = QByteArray::fromHex(hash.toLatin1());
    QByteArray ret_BA;
    ret_BA.append(static_cast<const char>(0));
    ret_BA.append(QByteArray(Password.toLatin1()));
    ret_BA.append(hash_BA);
    return QCryptographicHash::hash(ret_BA, QCryptographicHash::Md5).toHex();
}

//private
QByteArray
QMikrotikAPI::Int2LittleEndianBA(int val){
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << val;
    return byteArray;
}

//private
QByteArray
QMikrotikAPI::Int2BigEndianBA(int val){
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << val;
    return byteArray;
}

//private
int
QMikrotikAPI::BA2Int_LittleEndian(QByteArray ba){
    QDataStream stream(&ba, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    for(int i = ba.size(); i < sizeof(int) ;++i){
        ba.append(static_cast<char>(0));
    }
    int i;
    stream >> i;
    return i;
}

