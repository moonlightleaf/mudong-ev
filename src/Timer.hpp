#pragma once

#include <cassert>

#include "noncopyable.hpp"
#include "Callbacks.hpp"
#include "Timestamp.hpp"

namespace mudong {

namespace ev{

class Timer: noncopyable {

public:
    Timer(TimerCallback callback, Timestamp when, Nanoseconds interval)
            : callback_(callback),
              when_(when),
              interval_(interval),
              repeat_(interval_ > Nanoseconds::zero()),
              canceled_(false)
    {}

    void run() {
        if (callback_) callback_();
    }

    bool repeat() const {
        return repeat_;
    }

    bool expired(Timestamp now) const {
        return now >= when_;
    }

    Timestamp when() const {
        return when_;
    }

    void restart() {
        assert(repeat_);
        when_ += interval_;
    }

    void cancel() {
        assert(!canceled_);
        canceled_ = true;
    }

    bool canceled() const {
        return canceled_;
    }


private:
    TimerCallback callback_;
    Timestamp when_;
    const Nanoseconds interval_;
    bool repeat_;
    bool canceled_;
};

} // namespace ev

} // namespace mudong