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
using TimeStamp = std::chrono::time_point<system_clock, Nanoseconds>;

namespace clock {

inline TimeStamp now() { return system_clock::now(); }
inline TimeStamp nowAfter(Nanoseconds interval) { return system_clock::now() + interval; }
inline TimeStamp nowBefore(Nanoseconds interval) { return system_clock::now() - interval; }

} // namespace clock 

namespace {

template <typename T>
struct IntervalTypeCheckImpl
{
    static constexpr bool value =
            std::is_same<T, Nanosecond>::value ||
            std::is_same<T, Microsecond>::value ||
            std::is_same<T, Millisecond>::value ||
            std::is_same<T, Second>::value ||
            std::is_same<T, Minute>::value ||
            std::is_same<T, Hour>::value;
};

} // anonymous namespace

#define IntervalTypeCheck(T) \
    static_assert(IntervalTypeCheckImpl<T>::value, "bad interval type")

} // namespace ev

} // namespace mudong