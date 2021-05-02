#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <vector>

namespace socket_comm {
class Client {
 public:
  Client() {}
  ~Client() {
    close(client__socket_id_);
  }

  bool init(const std::string ip = "127.0.0.1", int port = 12336) {
    client__socket_id_ = socket(AF_INET, SOCK_STREAM, 0);
    if (client__socket_id_ < 0) {
      std::cout << "[Client]: ERROR establishing socket\n" << std::endl;
      return false;
    }

    bool connected = false;
    int connection_attempts = 100;

    while ((!connected) && (connection_attempts > 0)) {
      struct sockaddr_in serv_addr;
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(port);
      inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);

      if (connect(
              client__socket_id_, (const struct sockaddr*)&serv_addr,
              sizeof(serv_addr)) == 0) {
        connected = true;
        std::cout << "[Client]: Cpp socket client connected." << std::endl;
      } else {
        sleep(1);
        connection_attempts -= 1;
        std::cout << "[Client]: Error connecting to " << ip << ":" << port
                  << std::endl;
      }
    }
    return connected;
  }

  void send_image(cv::Mat img) {
    std::vector<uchar> buf;
    cv::imencode(".jpg", img, buf);

    std::ostringstream ss;
    ss << std::setw(size_message_length_) << std::setfill('0') << buf.size()
       << "\n";

    send(client__socket_id_, "L83F", 4, 0);  // magic id
    send(client__socket_id_, ss.str().c_str(), size_message_length_, 0);
    send(client__socket_id_, buf.data(), buf.size(), 0);
  }

  std::string receive() {
      char buff[1024];
      recv(client__socket_id_,buff ,1024,0);
      return std::string(buff);
  }

 private:
  int client__socket_id_;
  const int size_message_length_ = 16;  // Buffer size for the length
};
}  // namespace socket_comm
