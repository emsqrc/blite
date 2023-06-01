#ifndef QMIKROTIKAPI_H
#define QMIKROTIKAPI_H

#include <QObject>
#include <QDebug>
#include <QtGlobal>
#include <QTcpSocket>
#include <QString>
#include <QAbstractSocket>
#include <QByteArray>
#include <QCryptographicHash>
#include <QStringList>
#include <QByteArrayList>
#include <QDataStream>
#include <QThread>

// for qt < 5.4
// typedef QList<QByteArray> QByteArrayList;

//!\version QMikrotikAPI v. 2.0.0


/*!
 * \brief The QMikrotikAPI class is base class of QMikrotikAPI
 * \example qmikrotikapi_examples.h
 */
class QMikrotikAPI:public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief tcpSocket TCP Socket for connect to the Mikrotik RouterOS host
     */
    QTcpSocket      *tcpSocket;     // socket

private:
    bool            loginSt;        // login state flag
    QByteArray      username;
    QByteArray      password;
    QByteArray      retBA;          // for ret raw data

    QByteArray EncodeLength(int intval);
    QByteArray EncodePassword(QString Password, QString hash);
    QByteArray Int2LittleEndianBA(int val);
    QByteArray Int2BigEndianBA(int val);
    int BA2Int_LittleEndian(QByteArray ba);
    bool sendLoginReq();    // process all login

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

public:

    /*!
     * \brief QMikrotikAPI constructor
     * \param ip    Mikrotik RouterOS host IP address
     * \param uname user name
     * \param pwd   user password
     * \param port  Mikrotik RouterOS host port
     */
    explicit QMikrotikAPI(QByteArray ip, QByteArray uname, QByteArray pwd, int port = 8728);

    ~QMikrotikAPI();

    /*!
     * \brief quit quit connection with RouterOS host
     * \return If no error - return 'true' , otherwise return 'false'
     */
    bool quit();

    /*!
     * \brief send function send data to RouterOS host
     * \param data_BA   QByteArray data
     * \param end       bool - if 'true' it ned for sent end '\0' char
     */
    void send(QByteArray data_BA,bool end = false);

    /*!
     * \brief request function send request sentence to Mikrotik RouterOS
     * \param req_bal QByteArrayList with list of requests
     * \return QByteArray answer from Mikrotik RouterOS host
     */
    QByteArray request(QByteArrayList &req_bal);

    /*!
     * \brief readAllData function read all data from Mikrotik RouterOS
     * \return QByteArray of data, if tcp socket error - return 'NULL' QByteArray
     */
    QByteArray readAllData();

    /*!
     * \brief clearData function clear raw data getting from Mikrotik RouterOS
     * \param ba QByteArray data for clearing
     * \return  QByteArrayList with =key=value strings or return NULL(empty)QBAList if error or if ba.isEmpty()
     */
    QByteArrayList clearData(QByteArray &ba);

    /*!
     * \brief socketState function get tcp socket state
     * \return QAbstractSocket::SocketState socket state
     */
    QAbstractSocket::SocketState socketState(){
        return tcpSocket->state();
    }

    /*!
     * \brief isLogined function get login state
     * \return  bool true if logined of false if no logined
     */
    bool isLogined();

    /*!
     * \brief close function close connection
     */
    void close();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

public:

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~ /ip/address ~~~~~~~~~~~~~~~~~~~~~~~~~~~//

    /*!
     * \brief get_ip_address_info function get info about ip address by ip address value and/or network ip or all interfaces.
     * \param ip QByteArray ipAddr param must be at "192.168.0.3/24" or NULL format
     * \param netw QByteArray ipNetw param is "192.168.0.3" or NULL
     * \param interface QByteArray name of interface or NULL
     * \return  QByteArray with ip address info or if address and/or network not found - return empty QByteArray(NULL).
     */
    QByteArray get_ip_address_info(QByteArray ip = NULL, QByteArray netw = NULL, QByteArray interface = NULL);

    /*!
     * \brief get_ip_address_id get ip address id by ip or/and network or/and interface
     * \param ip QByteArray IP address
     * \param netw  QByteArray network
     * \param interface QByteArray interface
     * \return QByteArray with data or NULL QByteArray if error
     */
    QByteArray get_ip_address_id(QByteArray ip = NULL, QByteArray netw = NULL, QByteArray interface = NULL);

    /*!
     * \brief ip_address_remove remove IP address
     * \param ip QByteArray IP address
     * \param netw QByteArray network
     * \param interface QByteArray interface
     * ip as "10.10.0.1/24" , netw as "10.10.0.0" interface as "ether0"
     * \return bool. If no error - return 'true' , otherwise return 'false'
     */
    bool ip_address_remove(QByteArray ip = NULL, QByteArray netw = NULL, QByteArray interface = NULL);

    /*!
     * \brief ip_address_add add ip addres in format "192.168.0.33/32" & network addr for interface
     * \param ip   QByteArray IP address
     * \param network QByteArray network
     * \param interface QByteArray interface
     * \return bool. If no error - return 'true' , otherwise return 'false'
     */
    bool ip_address_add(QByteArray ip,QByteArray network,QByteArray interface);

    //~~~~~~~~~~~~~~~~~~~~~~~ END /ip/address END ~~~~~~~~~~~~~~~~~~~~~~~//

public:

    //~~~~~~~~~~~~~~~~~~~~~~~~ /ip/firewall/nat ~~~~~~~~~~~~~~~~~~~~~~~~~//

    /*!
     * \brief ip_firewall_nat_add_dstnat Add rule allowing access to the internal server from external networks:
     * RouterOS host cmd: /ip firewall nat add chain=dstnat dst-address=10.5.8.200 action=dst-nat to-addresses=192.168.0.109
     * \param dst_addr QByteArray destination address
     * \param to_addr   QByteArray for local address
     * \return If no error - return 'true' , otherwise return 'false'
     */
    bool ip_firewall_nat_add_dstnat(QByteArray dst_addr, QByteArray to_addr);

    /*!
     * \brief ip_firewall_nat_add_srcnat Add rule allowing the internal server to initate connections to the outer networks having its source address translated to 10.5.8.200:
     * equiv cmd: /ip firewall nat add chain=srcnat src-address=192.168.0.109 action=src-nat to-addresses=10.5.8.200
     * \param src_addr QByteArray source address
     * \param to_addr QByteArray to address
     * \return If no error - return 'true' , otherwise return 'false'
     */
    bool ip_firewall_nat_add_srcnat(QByteArray src_addr, QByteArray to_addr);

    //~~~~~~~~~~~~~~~~~~~~ END /ip/firewall/nat END ~~~~~~~~~~~~~~~~~~~~~//

public:

    //~~~~~~~~~~~~~~~~~~~~~~~~~~ /queue/simple ~~~~~~~~~~~~~~~~~~~~~~~~~~//

    /*!
     * \brief queue_simple_add function add queue simple on the Mikrotik RouterOS host
     * \param params QByteArrayList list of parameters
     *
     * example from Mikrotik API
     * /queue simple add target-addresses=192.168.11.1/32 limit-at=32000/32000 max-limit=64000/42000 time=00:00:00-00:00:00,mon,tue,wed,thu,fri
     * for call this need fill QByteArrayList with parameters:
     * QByteArrayList params;
     * params << "=target=" + "...";
     * params << "=limit-at=" + "...";
     * params << "=max-limit=" + "...";
     * params << "=name="+"queue_name";
     * etc..
     *
     * \return bool. If success - return 'true' otherwise return 'false'
     */
    bool queue_simple_add(QByteArrayList params);

    /*!
     * \brief queue_simple_add add /queue/simple
     * \param target QByteArray target name
     * \param limit_at QByteArray limit
     * \param max_limit QByteArray maximum limit
     * \param name  QByteArray queue name
     * \param queue QByteArray queue
     * \param total_queue QByteArray total queue
     * \param priority  QByteArray priority
     * \param time QByteArray time
     * Example: target-addresses=192.168.11.1/32 , limit-at=32000/32000, queue=default/default(from /queue type) ,time=00:00:00-00:00:00,mon,tue,wed,thu,fri
     * total_queue = "wireless-default" (from /queue type)
     * \return If success - return 'true' otherwise return 'false'
     */
    bool queue_simple_add(QByteArray target, QByteArray limit_at,QByteArray max_limit,QByteArray name = NULL,
                          QByteArray queue = NULL, QByteArray total_queue = NULL,QByteArray priority = NULL, QByteArray time = NULL);

    /*!
     * \brief get_queue_simple_info_by_name  get /queue/simple info by queue name
     * \param queue_s_Name QByteArray /queue/simple name
     * \return QByteArray with data or NULL QByteArray
     */
    QByteArray get_queue_simple_info_by_name(QByteArray queue_s_Name);

    /*!
     * \brief get_queue_simple_id_by_name get /queue/simple DI by queue name
     * \param queue_s_Name QByteArray /queue/simple name
     * \return QByteArray with data or NULL QByteArray
     */
    QByteArray get_queue_simple_id_by_name(QByteArray queue_s_Name);

    /*!
     * \brief queue_simple_get_parameter get parameter 'par' for queue simple with name 'qsName'.
     * Parameter must be in format 'target' but not '=target=' !
     * \param qsName QByteArray /queue/simple name
     * \param par QByteArray /queue/simple parameter name
     * \return QByteArray with data or NULL QByteArray
     */
    QByteArray queue_simple_get_parameter(QByteArray qsName, QByteArray par = __null);

    /*!
     * \brief queue_simple_set set parameter 'par' into value 'val' for queue simple with name 'qsName'
     * Parameter must be in format 'target' but not '=target=' !
     * \param qsName QByteArray /queue/simple name
     * \param par QByteArray /queue/simple parameter name
     * \param val QByteArray /queue/simple parameter value to set
     * \return If success - return 'true' otherwise return 'false'
     */
    bool queue_simple_set(QByteArray qsName, QByteArray par, QByteArray val);

    /*!
     * \brief queue_simple_remove_by_name remove /queue/simple by name
     * \param queue_s_Name QByteArray /queue/simple name
     * \return If success - return 'true' otherwise return 'false'
     */
    bool queue_simple_remove_by_name(QByteArray queue_s_Name);

    //~~~~~~~~~~~~~~~~~~~~~~ END /queue/simple END ~~~~~~~~~~~~~~~~~~~~~~//

public:

    //~~~~~~~~~~~~~~~~~~~~~~~~~ /interface/vlan ~~~~~~~~~~~~~~~~~~~~~~~~~//

    /*!
     * \brief get_vlan_info_by_name get vlan info by vlan name or all vlans if vlanName is NULL ,
     * if not exists - NULL QByteArray returned
     * \param vlanName QByteArray vlan name
     * \return QByteArray with data or NULL QByteArray
     */
    QByteArray get_vlan_info_by_name(QByteArray vlanName);

    /*!
     * \brief get_vlan_id_by_name return vlan id (=.id=) by vlan name or NULL QBArray if not exists or other err
     * \param vlanName QByteArray vlan name
     * \return QByteArray with data or NULL QByteArray
     */
    QByteArray get_vlan_id_by_name(QByteArray vlanName);

    /*!
     * \brief get_vlan_parameter get parameter 'par' for vlan with name 'vlanName'
     * parameter must be in format 'target' but not '=target=' !
     * \param vlanName QByteArray vlan nanme
     * \param par QByteArray vlan parameter name
     * \return QByteArray with data or NULL QByteArray
     */
    QByteArray get_vlan_parameter(QByteArray vlanName, QByteArray par = __null);

    /*!
     * \brief vlan_set set vlan parameter to value
     * for example: 'vlan100','disabled','true' - set vlan100 to the disabled
     * \param vlanName QByteArray vlan name
     * \param par QByteArray vlan parameter name
     * \param val QByteArray vlan parameter value
     * \return If no error - return 'true' , otherwise return 'false'
     */
    bool vlan_set(QByteArray vlanName, QByteArray par, QByteArray val);

    /*!
     * \brief vlan_add add vlan by name and vlan ID
     * \param vlanName QByteArray vlan name
     * \param vlanID QByteArray vlan ID
     * \param interface QByteArray interface name
     * \param proxy_arp QByteArray 'proxy-arp' parameter
     * \param l2mtu QByteArray l2mtu parameter
     * \return If no error - return 'true' , otherwise return 'false'
     */
    bool vlan_add(QByteArray vlanName,QByteArray vlanID,QByteArray interface,
                  QByteArray proxy_arp = "proxy-arp", QByteArray l2mtu = "65531");

    /*!
     * \brief vlan_remove delete vlan by vlan name
     * EXAMPLE CMD: "/interface/vlan/remove + \n + =.id=*236"
     * *236 - is real vlan ID !
     * NO "?=....=" ONLY "=....=" for actions, "?" - for requests !
     * \param vlanName QByteArray vlan name
     * \param vlanID QByteArray vlan ID - now is UNUSED !!!
     * \return If no error - return 'true' , otherwise return 'false'
     */
    bool vlan_remove(QByteArray vlanName = NULL,QByteArray vlanID = NULL);

    //~~~~~~~~~~~~~~~~~~~~~ END /interface/vlan END ~~~~~~~~~~~~~~~~~~~~~//

public:

    //~~~~~~~~~~~~~~~~~ get mikrotik RouterOS info ~~~~~~~~~~~~~~~~~~~//

    /*!
     * \brief get_interface_info get interface by name if exists or all interfaces if exists
     * \param ifName QByteArray interfaces name. If ifName = NULL - get all interfaces
     * \param req QByteArray request
     * \return QByteArray with data or NULL QByteArray
     */
    QByteArray get_interface_info(QByteArray ifName = NULL,QByteArray req = "/interface/print");

    //~~~~~~~~~~~~~ END get mikrotik RouterOS info END ~~~~~~~~~~~~~~~//

signals:

    /*!
     * \brief error emit if error occurred
     * \param socketError
     * \param message
     */
    void error(int socketError, const QString &message);

    /*!
     * \brief rawDataReady emit when RouterOS host data is ready for reading.
     * For use in GUI applications.
     */
    void rawDataReady();        // for use in gui app

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

public slots:


};

#endif // QMIKROTIKAPI_H


