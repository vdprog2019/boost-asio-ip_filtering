//created by vdprog2019
#include <QtCore/QCoreApplication>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>


void connection_to_server() {
    try
    {

        boost::asio::io_service io_service;
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 1234);
        std::cout << ep.address().to_string().c_str() << std::endl;

        boost::asio::ip::tcp::socket socket(io_service);
        socket.connect(ep); 
        if (socket.is_open())
        {
            std::cout << "Connection established" << std::endl;
        }
        //std::string const& message = "Hello from client";
        std::string str_send = ep.address().to_string().c_str();  std::cout << "To the server sent: " << str_send << std::endl; //std::cin >> str_send;
        socket.send(boost::asio::buffer(str_send));

    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        std::cout << "Connection refused" << std::endl;
    }

}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout << "Try to connect ..." << std::endl;

    connection_to_server();

    return a.exec();
}
