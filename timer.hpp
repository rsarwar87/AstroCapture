#ifndef __TIMER__
#define __TIMER__
#include <chrono>

class Timer {
private:
    std::chrono::steady_clock::time_point pr_StartTime;
    std::chrono::steady_clock::time_point pr_EndTime;

public:
    Timer() 
    {
    }

    ~Timer() 
    {
        Finish();
    }

    void Start() 
    {
        pr_StartTime = std::chrono::steady_clock::now();
    }

    uint32_t Finish()
    {
        using namespace std::chrono;
        pr_EndTime = steady_clock::now();
        auto Duration = duration_cast<milliseconds>(pr_EndTime-pr_StartTime);
        return Duration.count();
    }
};


#endif
