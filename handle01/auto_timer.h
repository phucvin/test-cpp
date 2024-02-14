#ifndef _AUTO_TIMER_H_
#define _AUTO_TIMER_H_

#include <ctime>
#include <iomanip>
#include <iostream>

class AutoTimer
{
    static short const mDefaultPlaces = 6;

public:

    AutoTimer(short places = mDefaultPlaces)
        : mOut(std::cout), mStart(std::clock()), mPlaces(places)
    {
    }

    AutoTimer(std::ostream& out, short places = mDefaultPlaces)
        : mOut(out), mStart(std::clock()), mPlaces(places)
    {
    }

    ~AutoTimer()
    {
        report();
    }

    inline void
    report() const
    {
        mOut << std::fixed << std::setprecision(mPlaces) << (std::clock() - mStart) / static_cast<double>(CLOCKS_PER_SEC) << "s" << std::endl;
    }

private:
    std::ostream& mOut;
    std::clock_t const mStart;
    short const mPlaces;
};

#endif /* _AUTO_TIMER_H_ */