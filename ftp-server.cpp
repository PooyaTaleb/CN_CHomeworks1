#include <iostream>
#include <string>
#include <boost/asio.hpp>

#include <fstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using boost::asio::ip::tcp;

using namespace std;


class ftp_session
{
public:
    ftp_session(boost::asio::io_context& io_context)
        : socket_(io_context)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    handle_request(length);
                }
                else
                {
                    delete this;
                }
            });
    }

private:
    void handle_request(std::size_t length)
    {
        // Parse the request and determine the appropriate action
        std::string request(data_, length);
        std::cout << "Received request: " << request << std::endl;

        // Send the response
        std::string response = "200 OK\r\n";
        boost::asio::async_write(socket_, boost::asio::buffer(response),
            [this](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    start();
                }
                else
                {
                    delete this;
                }
            });
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};



class ftp_server
{
public:
    ftp_server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
        io_context_(io_context)
    {
        start_accept();
    }

private:
    void start_accept()
    {
        ftp_session* new_session = new ftp_session(io_context_);
        cout<<-1<<endl;
        acceptor_.async_accept(new_session->socket(),
            [this, new_session](boost::system::error_code ec)
            {
                cout<<-2<<endl;
                if (!ec)
                {
                    cout<<-3<<endl;
                    new_session->start();
                }
                else
                {
                    delete new_session;
                }

                start_accept();
            });
    }

    tcp::acceptor acceptor_;
    boost::asio::io_context& io_context_;
};

int main(int argc, char* argv[]){

    ifstream f("config.json");
    json config_data= json::parse(f);

    int command_port=config_data["commandChannelPort"];
    int data_port=config_data["dataChannelPort"];
    
    try
    {
        boost::asio::io_context io_context;

        ftp_server server(io_context, command_port);

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
