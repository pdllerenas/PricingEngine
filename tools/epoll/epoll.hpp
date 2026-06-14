#include <sys/epoll.h>
#include <unistd.h>

#include <cerrno>
#include <system_error>
#include <utility>
#include <vector>

class Epoll {
 private:
  int epoll_fd{-1};

 public:
  Epoll() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
      throw std::system_error(errno, std::system_category(),
                              "Critical Failure: epoll_create1");
    }
  }

  Epoll(const Epoll&) = delete;
  Epoll& operator=(const Epoll&) = delete;

  Epoll(Epoll&& other) noexcept : epoll_fd(std::exchange(other.poll_fd, -1)) {}
  Epoll& operator=(Epoll&& other) noexcept {
    if (this != &other) {
      if (epoll_fd >= 0) close(epoll_fd);
      epoll_fd = std::exchange(other.epoll_fd, -1);
    }
    return *this;
  }

  ~Epoll() {
    if (epoll_fd >= 0) close(epoll_fd);
  }

  void add_socket(int fd, uint32_t events = EPOLLIN) const {
    struct epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
      throw std::system_error(errno, std::system_category(),
                              "Critical Failure: epoll_ctl ADD");
    }
  }

  [[nodiscard]] int wait(struct epoll_event* events, int max_events,
                         int timeout_ms = -1) {
    return epoll_wait(epoll_fd, events, max_events, timeout_ms);
  }
};