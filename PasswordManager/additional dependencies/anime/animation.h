#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <numbers>
#include <vector>
#include <chrono>
#include <ctime>

#ifndef min
using std::min;
#endif

#ifndef max
using std::max;
#endif

class Animation
{
public:
    enum State
    {
        any = -1,
        back = 0,
        forward = 1,
    };
    Animation(float duration = 1.0f);
    Animation(State startDir, float duration = 1.0f);
    bool SetActiveTimer(State AnimationDir = any);
    bool Start(State AnimationDir = any);
    bool Reverse();
    bool ForceReverse();
    bool StartAfter(Animation Animation, State EndState = any, State startDirection = any);
    void ForceDirection(State AnimationDir = any);
    void SetDuration(float time);
    float GetDuration();
    void SetDelay(float time);
    void Proceed();
    State GetState();
    // linear value
    float GetValue(bool autoProceed = false);
    bool AnimationEnded(State direction = any);
    bool IsRunning(State direction = any);
    // return functional value (fast, slow, fast)
    float GetValueArc(bool autoProceed = false);
    // return functional value (slow, fast, slow)
    float GetValueSin(bool autoProceed = false);
    float GetValueInOutSin(bool autoProceed = false);
    // NOT RECOMMEND - return functional value (incr, decr, incr) 
    float GetValueSpring(float k = 2.7f, bool autoProceed = false);

    float lerp(float a, float b);
    static float lerp(float a, float b, float t);
    static State ToAnimationState(bool value);

    float Time();
    static std::vector<Animation> InitializeArray(float duration, size_t size);

    //private:
        // You can set currentTime as you wish.
    static float GetTime();

private:
    float time = 0;
    float lastActiveTime = 0;
    float duration = 1;
    float delay = 0;
    State state = back;

};

inline Animation::Animation(float duration)
{
    this->duration = duration;
}

inline Animation::Animation(State startDir, float duration)
{
    this->duration = duration;
    this->state = startDir;
    this->SetActiveTimer(startDir);
}

inline bool Animation::SetActiveTimer(State AnimationDir)
{
    if (AnimationDir == this->state)
        return false;

    float reverseDelay = 0;
    if (this->IsRunning())
    {
        bool dir = this->state == forward;

        if (dir)
            reverseDelay = (this->time - 1) * this->duration;
        else
            reverseDelay = this->time * -this->duration;

        //reverseDelay = (this->time - !!dir) * ((float)dir - 0.5f) * 2 * this->duration;
    }

    if (AnimationDir == any)
        this->state = (this->state == forward) ? back : forward;
    else
        this->state = AnimationDir;

    this->lastActiveTime = this->GetTime() + this->delay + reverseDelay;
    return true;
}

inline bool Animation::Start(State AnimationDir)
{
    return this->SetActiveTimer(AnimationDir);
}

inline bool Animation::Reverse()
{
    if (!this->AnimationEnded(Animation::forward) && this->AnimationEnded(Animation::back))
        return this->Start(Animation::forward);
    if (!this->AnimationEnded(Animation::back) && this->AnimationEnded(Animation::forward))
        return this->Start(Animation::back);
    return false;
}

inline bool Animation::ForceReverse()
{
    this->state = this->state == Animation::forward ? this->state = Animation::back : Animation::forward;
    return true;
}

inline bool Animation::StartAfter(Animation Animation, State EndState, State startDirection)
{
    if (Animation.AnimationEnded(EndState))
    {
        this->Start(startDirection);
        return true;
    }
    return false;
}

inline void Animation::ForceDirection(State AnimationDir)
{
    this->state = AnimationDir;
}

inline void Animation::SetDuration(float time)
{
    if (this->duration != time)
        this->duration = time;
}

inline float Animation::GetDuration()
{
    return this->duration;
}

inline void Animation::SetDelay(float time)
{
    if (this->delay != time)
        this->delay = time;
}

inline void Animation::Proceed()
{
    float currentTime = this->GetTime();
    currentTime -= this->lastActiveTime;
    float tempTime = currentTime / this->duration;

    if (this->state == back)
        tempTime = (this->duration - currentTime) / this->duration;

    tempTime = max(min(tempTime, 1.f), 0.f);
    this->time = tempTime;
}

inline Animation::State Animation::GetState()
{
    return this->state;
}

inline float Animation::GetValue(bool autoProceed)
{
    if (autoProceed)
        this->Proceed();
    return this->time;
}

inline bool Animation::AnimationEnded(State direction)
{
    if (direction == any)
        return (this->time == 1 || this->time == 0);

    if (direction == forward)
        return (state == forward && this->time == 1);

    if (direction == back)
        return (state == back && this->time == 0);

    return (state == forward && this->time == 1) || (state == back && this->time == 0);
}

inline bool Animation::IsRunning(State direction)
{
    return (this->state == direction || direction == Animation::any) && !this->AnimationEnded(direction);
}

inline float Animation::GetValueArc(bool autoProceed)
{
    this->GetValue(autoProceed);
    return asinf(this->time) / (atanf(1) * 2);
}

inline float Animation::GetValueSin(bool autoProceed)
{
    this->GetValue(autoProceed);
    return sinf((this->time - 0.5f) * (atanf(1) * 4)) / 2 + 0.5f;
}

inline float Animation::GetValueInOutSin(bool autoProceed)
{
    this->GetValue(autoProceed);
    return 0.5f * (1 + sinf(atanf(1) * 4 * (this->time - 0.5f)));
}


inline float Animation::GetValueSpring(float k, bool autoProceed)
{
    float a = this->GetValueArc(autoProceed);
    return a * k - this->time * (k - 1);
}

inline float Animation::lerp(float a, float b)
{
    return lerp(a, b, this->time);
}

inline float Animation::lerp(const float a, const float b, float t)
{
    t = max(min(t, 1.f), 0.f);
    return a + (b - a) * t;
}

inline Animation::State Animation::ToAnimationState(bool value)
{
    return State((int)value);
}

inline float Animation::Time()
{
    return this->time;
}

inline std::vector<Animation> Animation::InitializeArray(float duration, size_t size)
{
    std::vector<Animation> arr(size);
    for (size_t i = 0; i < size; i++)
    {
        arr[i] = Animation(duration);
    }
    return arr;
}

inline float Animation::GetTime()
{
    auto timeNow = std::chrono::high_resolution_clock::now();
    float flTime = (float)(timeNow.time_since_epoch().count() / 1000) / 1000000.f;
    return flTime;
    //return (float)GImGui->Time;
}
