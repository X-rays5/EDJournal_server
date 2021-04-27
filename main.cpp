#include <set>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <utility>
#include "journal_logger.hpp"
#include "arghandler.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class broadcast_server {
public:
    broadcast_server() {
        m_server.init_asio();

        m_server.set_open_handler(bind(&broadcast_server::on_open,this,::_1));
        m_server.set_close_handler(bind(&broadcast_server::on_close,this,::_1));
        m_server.set_message_handler(bind(&broadcast_server::on_message,this,::_1,::_2));
    }

    void on_open(const connection_hdl& hdl) {
        std::error_code ec;
        m_server.send(hdl, "hello", websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cout << "Error occured code: " << ec << "\n";
        } else {
            m_connections.insert(hdl);
            if (this->CacheEvents_) {
                for (auto&& event : this->Events_) {
                    std::error_code ec2;
                    m_server.send(hdl, event, websocketpp::frame::opcode::text, ec2);
                    if (ec2) {
                        std::cout << "Error occured code: " << ec2 << "\n";
                    }
                }
            }
        }
    }

    void on_close(const connection_hdl& hdl) {
        m_connections.erase(hdl);
    }

    void on_message(connection_hdl hdl, const server::message_ptr& msg) {
        std::error_code ec;
        m_server.send(std::move(hdl), "Messages sent to this server get ignored", websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cout << "Error occured code: " << ec << "\n";
        }
    }

    void run(uint16_t port) {
        EDJournalLogger::Logger journal(1000, false);
        journal.SetEventHandler([this](std::string event, std::string raw){
            for (auto&& hdl : this->m_connections) {
                std::error_code ec;
                m_server.send(hdl, raw, websocketpp::frame::opcode::text, ec);
                if (ec) {
                    std::cout << "Error occured code: " << ec << "\n";
                }
            }
            if (this->CacheEvents_) {
                this->Events_.emplace_back(raw);
            }
            std::cout << "Broadcasted event: " << event << "\n";
        });
        auto journalT = std::thread([&journal]{
            goagain:
            journal.StartListening();
            goto goagain; // gets here on game close
        });
        typedef websocketpp::log::alevel loglevel;
        m_server.clear_access_channels(loglevel::none);
        m_server.set_access_channels(loglevel::none);
        m_server.set_access_channels(loglevel::connect | loglevel::disconnect | loglevel::fail);
        m_server.listen(port);
        m_server.start_accept();
        std::cout << "Server online\n";
        m_server.run();
        journalT.join();
    }

    void SetCacheEvents(bool set) {
        CacheEvents_ = set;
    }
private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    server m_server;
    con_list m_connections;
    bool CacheEvents_ = false;
    std::vector<std::string> Events_;
};

bool StringToBool(std::string Bool) {
    if (Bool == "true" || Bool == "TRUE" || Bool == "1")
        return true;
    return false;
}

int main(int argc, char* argv[]) {
    try {
        argshandler handler("--", true);
        handler.AddKnownArg("p", "Set port the program will run on");
        handler.AddKnownArg("c", "Set if all events should be cached and sent on a new connection");
        handler.HandleArgs(argc, argv);

        broadcast_server server;
        int port = handler.GetArg("p") != "" ? std::stoi(handler.GetArg("p")) : 5234;
        server.SetCacheEvents(handler.GetArg("c") != "" ? StringToBool(handler.GetArg("c")) : false);

        std::cout << "Server starting on port: " << port << " " << "With cache events: " << handler.GetArg("c") << "\n";
        server.run(port);
    } catch(websocketpp::exception &e) {
        std::cout << "Websocket exception\n" << e.what() << "\n";
    } catch(std::exception &e) {
        std::cout << "STD exception\n" << e.what() << "\n";
    } catch (...) {}
}