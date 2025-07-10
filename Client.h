#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <atomic>
#include <string>
#include <iostream>
#include <sstream>

using boost::asio::ip::tcp;

class Client {
private:
    boost::asio::io_service io_service;
    tcp::socket socket;
    std::atomic<bool> running;
    std::string lastMessage;
    std::pair<int, int> winView; // Coordenadas en (x, y)

public:
    // Constructor
    Client(const std::string& host, short port);
    void setWinView(std::pair<int, int> wv);
    std::pair<int, int> getWinView();
    void sendJSON(const nlohmann::json request_json);
    void sendRequest(const std::string& message);
    void receiveMessages(std::function<void(const std::string&)> callback);
    void closeConnection();
};

#endif // CLIENT_H