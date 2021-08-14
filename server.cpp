//created by vdprog2019
#include <QtCore/QCoreApplication>

#include <iostream>
#include <istream>
#include <ostream>
#include <chrono>
#include <exception>
#include <cstdlib>
#include <thread>
#include <list>
#include <string>
#include <ctime>
#include <streambuf>

#include <QtXml/qdom.h>
#include <QtXml/qxml.h>

#include <qfile.h>
#include <qstring.h>
#include <qstringlist.h>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

const QString ip_u = QString("ip");
const QString port_u = QString("port");
const QString b_list = QString("in_black_list");
const QString nb_list = QString("not_in_black_list");
const int max_length = 32;

QDomElement makeElement(QDomDocument& domDoc,
    const QString& strName,
    const QString& strAttr = QString::null,
    const QString& strText = QString::null
)
{
    QDomElement domElement = domDoc.createElement(strName);

    if (!strAttr.isEmpty()) {
        QDomAttr domAttr = domDoc.createAttribute("number");
        domAttr.setValue(strAttr);
        domElement.setAttributeNode(domAttr);
    }

    if (!strText.isEmpty()) {
        QDomText domText = domDoc.createTextNode(strText);
        domElement.appendChild(domText);
    }
    return domElement;
}

QDomElement contact(
    QDomDocument& domDoc,
    const QString& ip_,
    const QString& port_,
    const QString& b_list_,
    const QString& nb_list_
)
{
    static int nNumber = 1;

    QDomElement domElement = makeElement(domDoc,
        "DNS",
        QString().setNum(nNumber)
    );
    domElement.appendChild(makeElement(domDoc, ip_u, "", ip_));
    domElement.appendChild(makeElement(domDoc, port_u, "", port_));
    domElement.appendChild(makeElement(domDoc, b_list, "", b_list_));
    domElement.appendChild(makeElement(domDoc, nb_list, "", nb_list_));

    nNumber++;

    return domElement;
}


void create_xml() {
    QDomDocument doc("DNS");
    QDomElement  domElement = doc.createElement("DNS_INFO");
    doc.appendChild(domElement);

    QDomElement contact1 =
        contact(doc, "127.0.0.1", "3306", "false", "true");
    QDomElement contact2 =
        contact(doc, "129.213.3.3", "1321", "true", "true");
    QDomElement contact3 =
        contact(doc, "129.213.3.1", "1321", "false", "true");

    domElement.appendChild(contact1);
    domElement.appendChild(contact2);
    domElement.appendChild(contact3);

    QFile file("D:/Project_Network/DNS_Proj/DNS_Server/adressbook.xml");
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream(&file) << doc.toString();
        file.close();
    }
}

void session(tcp::socket sock)
{
    try
    {
        char data[max_length];
        for (;;)
        {
            //char data[max_length];

            boost::system::error_code error;
            size_t length = sock.read_some(boost::asio::buffer(data), error);
            if (error == boost::asio::error::eof)
                break;
            else if (error)
                throw boost::system::system_error(error);
            std::cout << "Received from client: " << data << std::endl;
            // boost::asio::write(sock, boost::asio::buffer(data, length));
        }
        QString data_ip = data;
        QFile file("D:/Project_Network/DNS_Proj/DNS_Server/adressbook.xml");

        if (file.open(QIODevice::ReadOnly))
        {
            //std::cout << "File open" << std::endl;
        }
        else {
            std::cerr << "Error while loading file" << std::endl;
            return;
        }
        QDomDocument doc;
        doc.setContent(file.readAll());
        //QDomNodeList list = doc.elementsByTagName("DNS_INFO");

        QDomElement root = doc.documentElement();
        QDomElement Component = root.firstChild().toElement();
        QString IP; QString in_b;
        while (!Component.isNull())
        {
            if (Component.tagName() == "DNS") {
                QDomElement Child = Component.firstChild().toElement();
                //QString IP;
                while (!Child.isNull())
                {
                    if (Child.tagName() == "ip") {
                        IP = Child.firstChild().toText().data();
                    }
                    if (Child.tagName() == "in_black_list") {
                        in_b = Child.firstChild().toText().data();
                    }
                    Child = Child.nextSibling().toElement();
                }
                if (IP == data_ip) {
                    if (in_b == "false") {
                        std::cout << data_ip.toStdString().c_str() << "    THIS IP IS VALID " << std::endl;
                        boost::array<char, 128> buf;
                        std::string message = data_ip.toStdString().c_str();
                        std::copy(message.begin(), message.end(), buf.begin());
                        boost::system::error_code error;
                        sock.write_some(boost::asio::buffer(message.data(), message.size()), error);
                        std::cout << "Data " + message + " sent" << std::endl;
                    }
                    else if (in_b == "true") {
                        std::cout << data_ip.toStdString().c_str() << "    THIS IP IS NOT RESOLVED" << std::endl;
                        boost::array<char, 128> buf;
                        std::string message = "Your ip is not resolved";
                        std::copy(message.begin(), message.end(), buf.begin());
                        boost::system::error_code error;
                        sock.write_some(boost::asio::buffer(message.data(), message.size()), error);
                    }
                }
                //std::cout << "   IP  = " << IP.toStdString().c_str() << "   In black list    " << in_b.toStdString().c_str() << std::endl;
            }
            Component = Component.nextSibling().toElement();
        }

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

void server(boost::asio::io_service& io_service, const unsigned short port)
{

    QFile file("D:/Project_Network/DNS_Proj/DNS_Server/adressbook.xml");

    if (file.open(QIODevice::ReadOnly))
    {
        std::cout << "File open" << std::endl;
    }
    else {
        std::cerr << "Error while loading file" << std::endl;
        return;
    }
    QDomDocument doc;
    doc.setContent(file.readAll());
    //QDomNodeList list = doc.elementsByTagName("DNS_INFO");

    QDomElement root = doc.documentElement();
    QDomElement Component = root.firstChild().toElement();
    QString IP; QString in_b;
    while (!Component.isNull())
    {
        if (Component.tagName() == "DNS") {
            QDomElement Child = Component.firstChild().toElement();
            //QString IP;
            while (!Child.isNull())
            {
                if (Child.tagName() == "ip") {
                    IP = Child.firstChild().toText().data();
                }
                if (Child.tagName() == "in_black_list") {
                    in_b = Child.firstChild().toText().data();
                }
                Child = Child.nextSibling().toElement();
            }

            std::cout << "   IP  = " << IP.toStdString().c_str() << "   In black list    " << in_b.toStdString().c_str() << std::endl;
        }
        Component = Component.nextSibling().toElement();
    }

    tcp::endpoint ep(tcp::endpoint(tcp::v4(), port));
    tcp::acceptor a(io_service, ep);

    for (;;)
    {
        tcp::socket sock(io_service);
        a.accept(sock);
        ////  std::cout << boost::lexical_cast<std::string>(sock.remote_endpoint()).c_str() << std::endl;
        std::string s = boost::lexical_cast<std::string>(sock.remote_endpoint()).c_str();
        std::vector<std::string> v; v.push_back(s); std::cout << "Server: " << v.data()->c_str() << "\n";
        std::thread(session, std::move(sock)).detach();
    }

}

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    //create_xml();
    try
    {
        boost::asio::io_service io_service;
        server(io_service, 1234);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return a.exec();
}
