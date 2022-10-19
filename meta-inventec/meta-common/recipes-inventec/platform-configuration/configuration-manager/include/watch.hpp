#include <functional>
#include <systemd/sd-event.h>
#include <string>

namespace ipmi::inv::configuration
{
    class Watch
    {
    public:
        Watch(sd_event *loop, std::function<int(std::string &)> configCallback);
        Watch(const Watch &) = delete;
        Watch &operator=(const Watch &) = delete;
        Watch(Watch &&) = delete;
        Watch &operator=(Watch &&) = delete;
        ~Watch();

    private:
        static int callback(sd_event_source *s, int fd, uint32_t revents, void *userdata);
        int wd = -1;
        int fd = -1;
        std::function<int(std::string &)> configCallback;
    };
}
