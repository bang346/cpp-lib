#include <gtest/gtest.h>
#include <cmath>
#include <iostream>
#include "sixstep.hpp"

#include <vector>

struct RecordingBridge
{
    bool init_called_;
    std::vector<SixStep_NS::channel_state_e> state;
    RecordingBridge()
        : init_called_{false},
          state{
              SixStep_NS::channel_state_e::UNINITIALISED,
              SixStep_NS::channel_state_e::UNINITIALISED,
              SixStep_NS::channel_state_e::UNINITIALISED}
    {
    }
    void init()
    {
        init_called_ = true;
    }
    int set_pwm(SixStep_NS::channel_e chan)
    {
        state[(int)chan] = SixStep_NS::channel_state_e::PWM;
        return 0;
    }
    int set_high_z(SixStep_NS::channel_e chan)
    {
        state[(int)chan] = SixStep_NS::channel_state_e::FLOATING;
        return 0;
    }
    int set_gnd(SixStep_NS::channel_e chan)
    {
        state[(int)chan] = SixStep_NS::channel_state_e::GND;
        return 0;
    }
};

TEST(SixStep, HallDecode)
{
    RecordingBridge bridge;
    SixStep_NS::SixStep<RecordingBridge> sixstep{bridge};

    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0, 0), -1);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b001, 1), 1);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b011, 1), 2);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b010, 1), 3);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b110, 1), 4);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b100, 1), 5);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b101, 1), 6);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b111, 1), -1);

    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b001, 0), 4);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b011, 0), 5);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b010, 0), 6);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b110, 0), 1);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b100, 0), 2);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b101, 0), 3);
    ASSERT_EQ(SixStep_NS::SixStep<RecordingBridge>::decode(0b111, 0), -1);
}

TEST(SixStep, Commtation)
{
    using namespace SixStep_NS;
    std::vector<std::vector<SixStep_NS::channel_state_e>> seq =
        {
            {channel_state_e::GND, channel_state_e::FLOATING, channel_state_e::PWM},
            {channel_state_e::GND, channel_state_e::PWM, channel_state_e::FLOATING},
            {channel_state_e::FLOATING, channel_state_e::PWM, channel_state_e::GND},
            {channel_state_e::PWM, channel_state_e::FLOATING, channel_state_e::GND},
            {channel_state_e::PWM, channel_state_e::GND, channel_state_e::FLOATING},
            {channel_state_e::FLOATING, channel_state_e::GND, channel_state_e::PWM}};

    RecordingBridge bridge;
    SixStep_NS::SixStep<RecordingBridge> sixstep{bridge};

    sixstep.init();
    ASSERT_TRUE(bridge.init_called_);
    EXPECT_EQ(bridge.state.size(), 3);
    auto x = bridge.state == std::vector<SixStep_NS::channel_state_e>{channel_state_e::UNINITIALISED, channel_state_e::UNINITIALISED, channel_state_e::UNINITIALISED};
    EXPECT_TRUE(x);

    for (size_t i = 0; i < 5; i++)
    {
        EXPECT_EQ(sixstep.commutation_step((step_e)i), 0);
        x = seq.at(i) == bridge.state;
        EXPECT_TRUE(x);
    }
}
