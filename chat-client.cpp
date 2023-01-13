#include <deque>
#include <array>
#include <thread>
#include <iostream>
#include <cstring>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include "protocol.hpp"

#include <string>

using boost::asio::ip::tcp;

using namespace boost::placeholders;

using namespace std;

class client
{
public:
    client(const std::array<char, MAX_NICKNAME>& nickname,
            boost::asio::io_context& io_context,
            tcp::resolver::iterator endpoint_iterator) :
            io_context_(io_context), socket_(io_context)
    {

        strcpy(nickname_.data(), nickname.data());
        memset(read_msg_.data(), '\0', MAX_IP_PACK_SIZE);
        boost::asio::async_connect(socket_, endpoint_iterator, boost::bind(&client::onConnect, this, _1));
    }

    void write(string msg)
    {
        io_context_.post(boost::bind(&client::writeImpl, this, msg));
    }

    void close()
    {
        io_context_.post(boost::bind(&client::closeImpl, this));
    }

private:

    void onConnect(const boost::system::error_code& error)
    {
        if (!error)
        {
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(nickname_, nickname_.size()),
                                     boost::bind(&client::readHandler, this, _1));
        }
    }

    void readHandler(const boost::system::error_code& error)
    {
        std::cout << read_msg_.data() << std::endl;
        if (!error)
        {
            boost::asio::async_read(socket_,
                                    boost::asio::buffer(read_msg_, read_msg_.size()),
                                    boost::bind(&client::readHandler, this, _1));
        } else
        {
            closeImpl();
        }
    }

    void writeImpl(string msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(write_msgs_.front(), write_msgs_.front().size()),
                                     boost::bind(&client::writeHandler, this, _1));
        }
    }

    void writeHandler(const boost::system::error_code& error)
    {
        if (!error)
        {
            write_msgs_.pop_front();
            if (!write_msgs_.empty())
            {
                boost::asio::async_write(socket_,
                                         boost::asio::buffer(write_msgs_.front(), write_msgs_.front().size()),
                                         boost::bind(&client::writeHandler, this, _1));
            }
        } else
        {
            closeImpl();
        }
    }

    void closeImpl()
    {
        socket_.close();
    }

    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    std::array<char, MAX_IP_PACK_SIZE> read_msg_;
    std::deque<string> write_msgs_;
    std::array<char, MAX_NICKNAME> nickname_;
};

int main(int argc, char* argv[]){
    try{
        // our client takes two arguements. first is localhost:port and second is username.
        if (argc != 3){
            std::cerr << "try entering with this format: ./chat_client <host>:<port> <username>\n";
            return 1;
        }
        
        std::string address = argv[1];
        std::string host = address.substr(0,address.find(':'));
        std::string port = address.substr(address.find(':')+1,address.length());
        char* username = argv[2];


        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::query query(host, port);
        tcp::resolver::iterator iterator = resolver.resolve(query);
        
        std::array<char, MAX_NICKNAME> nickname;
        strcpy(nickname.data(), username);

        client c(nickname, io_context, iterator);

        std::thread t(boost::bind(&boost::asio::io_context::run, &io_context));

        string msg;

        while (true)
        {
            getline(cin,msg);
            // memset(msg.data(), '\0', msg.size());
            // if (!std::cin.getline(msg.data(), MAX_IP_PACK_SIZE - PADDING - MAX_NICKNAME))
            // {
            //     std::cin.clear(); //clean up error bit and try to finish reading
            // }
            
            c.write(msg);
        }

        c.close();
        t.join();
    }

    catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

