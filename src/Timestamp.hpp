#pragma once

#include <chrono>

namespace mudong {

namespace ev {

using std::chrono::system_clock;

using Nanoseconds = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;
using Minutes = std::chrono::minutes;
using Hours = std::chrono::hours;

// chrono中有system_clock和steady_clock两种时钟，第一种适合用来表示日期，时间戳用，第二种适合用来计算两者之间的相对时间间隔，不受系统时间调整的影响
using Timestamp = std::chrono::time_point<system_clock, Nanoseconds>;

namespace clock {

inline Timestamp now() { return system_clock::now(); }
// 从当前时刻开始，时间流转一段长度后的时刻对应的时间戳
inline Timestamp nowAfter(Nanoseconds interval) { return system_clock::now() + interval; }
// 从当前时刻开始，时间倒退一段长度后的时刻对应的时间戳
inline Timestamp nowBefore(Nanoseconds interval) { return system_clock::now() - interval; }

} // namespace clock 

namespace {

template <typename T>
struct IntervalTypeCheckImpl
{
    static constexpr bool value =
            std::is_same<T, Nanoseconds>::value ||
            std::is_same<T, Microseconds>::value ||
            std::is_same<T, Milliseconds>::value ||
            std::is_same<T, Seconds>::value ||
            std::is_same<T, Minutes>::value ||
            std::is_same<T, Hours>::value;
};

} // anonymous namespace

#define IntervalTypeCheck(T) \
    static_assert(IntervalTypeCheckImpl<T>::value, "bad interval type")

} // namespace ev

} // namespace mudong