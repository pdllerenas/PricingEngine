#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <system_error>
#include <utility>

class Socket {
 private:
  int fd{-1};

 public:
  explicit Socket(uint16_t port) {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
      throw std::system_error(errno, std::system_category(),
                              "Critical Failure: socket creation");
    }

    struct sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) <
        0) {
      close(fd);
      throw std::system_error(errno, std::system_category(),
                              "Critical Failure: bind");
    }
  }

  Socket(const Socket&) = delete;             // copy
  Socket& operator=(const Socket&) = delete;  // copy assignment

  // return other.fd and set other.fd = -1
  Socket(Socket&& other) noexcept : fd(std::exchange(other.fd, -1)) {}

  Socket& operator=(Socket&& other) noexcept {
    if (this != &other) {
      if (fd >= 0) {
        close(fd);
      }
      fd = std::exchange(other.fd, -1);
    }
    return *this;
  }

  ~Socket() {
    if (fd >= 0) {
      close(fd);
    }
  }

  [[nodiscard]] int get_fd() const noexcept { return fd; }
};