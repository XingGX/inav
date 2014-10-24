/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>

#include <limits.h>

#include "common/axis.h"
#include "flight/flight.h"

#include "rx/rx.h"
#include "io/rc_controls.h"

#include "unittest_macros.h"
#include "gtest/gtest.h"

int constrain(int amt, int low, int high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

TEST(RcControlsTest, updateActivatedModesWithAllInputsAtMidde)
{
    // given
    modeActivationCondition_t modeActivationConditions[MAX_MODE_ACTIVATION_CONDITION_COUNT];
    memset(&modeActivationConditions, 0, sizeof(modeActivationConditions));

    // and
    rcModeActivationMask = 0;

    // and
    memset(&rxRuntimeConfig, 0, sizeof(rxRuntimeConfig_t));
    rxRuntimeConfig.auxChannelCount = MAX_SUPPORTED_RC_CHANNEL_COUNT - NON_AUX_CHANNEL_COUNT;

    // and
    uint8_t index;
    for (index = AUX1; index < MAX_SUPPORTED_RC_CHANNEL_COUNT; index++) {
        rcData[index] = PWM_RANGE_MIDDLE;
    }

    // when
    updateActivatedModes(modeActivationConditions);

    // then
    for (index = 0; index < CHECKBOX_ITEM_COUNT; index++) {
        printf("iteration: %d\n", index);
        EXPECT_EQ(false, IS_RC_MODE_ACTIVE(index));
    }
}

TEST(RcControlsTest, updateActivatedModesUsingValidAuxConfigurationAndRXValues)
{
    // given
    modeActivationCondition_t modeActivationConditions[MAX_MODE_ACTIVATION_CONDITION_COUNT];
    memset(&modeActivationConditions, 0, sizeof(modeActivationConditions));

    modeActivationConditions[0].modeId = (boxId_e)0;
    modeActivationConditions[0].auxChannelIndex = AUX1 - NON_AUX_CHANNEL_COUNT;
    modeActivationConditions[0].rangeStartStep = CHANNEL_VALUE_TO_STEP(1700);
    modeActivationConditions[0].rangeEndStep = CHANNEL_VALUE_TO_STEP(2100);

    modeActivationConditions[1].modeId = (boxId_e)1;
    modeActivationConditions[1].auxChannelIndex = AUX2 - NON_AUX_CHANNEL_COUNT;
    modeActivationConditions[1].rangeStartStep = CHANNEL_VALUE_TO_STEP(1300);
    modeActivationConditions[1].rangeEndStep = CHANNEL_VALUE_TO_STEP(1700);

    modeActivationConditions[2].modeId = (boxId_e)2;
    modeActivationConditions[2].auxChannelIndex = AUX3 - NON_AUX_CHANNEL_COUNT;
    modeActivationConditions[2].rangeStartStep = CHANNEL_VALUE_TO_STEP(900);
    modeActivationConditions[2].rangeEndStep = CHANNEL_VALUE_TO_STEP(1200);

    modeActivationConditions[3].modeId = (boxId_e)3;
    modeActivationConditions[3].auxChannelIndex = AUX4 - NON_AUX_CHANNEL_COUNT;
    modeActivationConditions[3].rangeStartStep = CHANNEL_VALUE_TO_STEP(900);
    modeActivationConditions[3].rangeEndStep = CHANNEL_VALUE_TO_STEP(2100);

    modeActivationConditions[4].modeId = (boxId_e)4;
    modeActivationConditions[4].auxChannelIndex = AUX5 - NON_AUX_CHANNEL_COUNT;
    modeActivationConditions[4].rangeStartStep = CHANNEL_VALUE_TO_STEP(900);
    modeActivationConditions[4].rangeEndStep = CHANNEL_VALUE_TO_STEP(925);

    EXPECT_EQ(0, modeActivationConditions[4].rangeStartStep);
    EXPECT_EQ(1, modeActivationConditions[4].rangeEndStep);

    modeActivationConditions[5].modeId = (boxId_e)5;
    modeActivationConditions[5].auxChannelIndex = AUX6 - NON_AUX_CHANNEL_COUNT;
    modeActivationConditions[5].rangeStartStep = CHANNEL_VALUE_TO_STEP(2075);
    modeActivationConditions[5].rangeEndStep = CHANNEL_VALUE_TO_STEP(2100);

    EXPECT_EQ(47, modeActivationConditions[5].rangeStartStep);
    EXPECT_EQ(48, modeActivationConditions[5].rangeEndStep);

    modeActivationConditions[6].modeId = (boxId_e)6;
    modeActivationConditions[6].auxChannelIndex = AUX7 - NON_AUX_CHANNEL_COUNT;
    modeActivationConditions[6].rangeStartStep = CHANNEL_VALUE_TO_STEP(925);
    modeActivationConditions[6].rangeEndStep = CHANNEL_VALUE_TO_STEP(950);

    EXPECT_EQ(1, modeActivationConditions[6].rangeStartStep);
    EXPECT_EQ(2, modeActivationConditions[6].rangeEndStep);

    // and
    rcModeActivationMask = 0;

    // and
    memset(&rxRuntimeConfig, 0, sizeof(rxRuntimeConfig_t));
    rxRuntimeConfig.auxChannelCount = MAX_SUPPORTED_RC_CHANNEL_COUNT - NON_AUX_CHANNEL_COUNT;

    // and
    uint8_t index;
    for (index = AUX1; index < MAX_SUPPORTED_RC_CHANNEL_COUNT; index++) {
        rcData[index] = PWM_RANGE_MIDDLE;
    }

    rcData[AUX1] = PWM_RANGE_MAX;
    rcData[AUX2] = PWM_RANGE_MIDDLE;
    rcData[AUX3] = PWM_RANGE_MIN;
    rcData[AUX4] = PWM_RANGE_MAX;
    rcData[AUX5] = 899; // value lower that range minimum should be treated the same as the lowest range value
    rcData[AUX6] = 2101; // value higher than the range maximum should be treated the same as the highest range value
    rcData[AUX7] = 950; // value equal to range step upper boundary should not activate the mode

    // and
    uint32_t expectedMask = 0;
    expectedMask |= (1 << 0);
    expectedMask |= (1 << 1);
    expectedMask |= (1 << 2);
    expectedMask |= (1 << 3);
    expectedMask |= (1 << 4);
    expectedMask |= (1 << 5);
    expectedMask |= (0 << 6);

    // when
    updateActivatedModes(modeActivationConditions);

    // then
    for (index = 0; index < CHECKBOX_ITEM_COUNT; index++) {
        printf("iteration: %d\n", index);
        EXPECT_EQ(expectedMask & (1 << index), rcModeActivationMask & (1 << index));
    }
}

enum {
    COUNTER_GENERATE_PITCH_ROLL_CURVE = 0
};
#define CALL_COUNT_ITEM_COUNT 1

static int callCounts[CALL_COUNT_ITEM_COUNT];

#define CALL_COUNTER(item) (callCounts[item])

void generatePitchRollCurve(controlRateConfig_t *) {
    callCounts[COUNTER_GENERATE_PITCH_ROLL_CURVE]++;
}

void resetCallCounters(void) {
    memset(&callCounts, 0, sizeof(callCounts));
}

uint32_t fixedMillis = 0;

uint32_t millis(void) {
    return fixedMillis;
}

#define DEFAULT_MIN_CHECK 1100
#define DEFAULT_MAX_CHECK 1900

rxConfig_t rxConfig;

extern uint32_t adjustmentFunctionStateMask;

TEST(RcControlsTest, processRcAdjustmentsSticksInMiddle)
{
    // given
    controlRateConfig_t controlRateConfig = {
            .rcRate8 = 90,
            .rcExpo8 = 0,
            .thrMid8 = 0,
            .thrExpo8 = 0,
            .rollPitchRate = 0,
            .yawRate = 0,
    };

    // and
    memset(&rxConfig, 0, sizeof (rxConfig));
    rxConfig.mincheck = DEFAULT_MIN_CHECK;
    rxConfig.maxcheck = DEFAULT_MAX_CHECK;
    rxConfig.midrc = 1500;

    // and
    uint8_t index;
    for (index = AUX1; index < MAX_SUPPORTED_RC_CHANNEL_COUNT; index++) {
        rcData[index] = PWM_RANGE_MIDDLE;
    }

    // and
    resetCallCounters();

    // when
    processRcAdjustments(&controlRateConfig, &rxConfig);

    // then
    EXPECT_EQ(controlRateConfig.rcRate8, 90);
    EXPECT_EQ(CALL_COUNTER(COUNTER_GENERATE_PITCH_ROLL_CURVE), 0);
    EXPECT_EQ(adjustmentFunctionStateMask, 0);
}

TEST(RcControlsTest, processRcAdjustmentsWithRcRateFunctionSwitchUp)
{
    // given
    controlRateConfig_t controlRateConfig = {
            .rcRate8 = 90,
            .rcExpo8 = 0,
            .thrMid8 = 0,
            .thrExpo8 = 0,
            .rollPitchRate = 0,
            .yawRate = 0,
    };

    // and
    memset(&rxConfig, 0, sizeof (rxConfig));
    rxConfig.mincheck = DEFAULT_MIN_CHECK;
    rxConfig.maxcheck = DEFAULT_MAX_CHECK;
    rxConfig.midrc = 1500;

    // and
    uint8_t index;
    for (index = AUX1; index < MAX_SUPPORTED_RC_CHANNEL_COUNT; index++) {
        rcData[index] = PWM_RANGE_MIDDLE;
    }

    // and
    resetCallCounters();

    // and
    rcData[AUX3] = PWM_RANGE_MAX;

    // and
    uint32_t expectedAdjustmentFunctionStateMask =
            (1 << (ADJUSTMENT_RC_RATE - ADJUSTMENT_INDEX_OFFSET));

    // and
    fixedMillis = 496;

    // when
    processRcAdjustments(&controlRateConfig, &rxConfig);

    // then
    EXPECT_EQ(controlRateConfig.rcRate8, 91);
    EXPECT_EQ(CALL_COUNTER(COUNTER_GENERATE_PITCH_ROLL_CURVE), 1);
    EXPECT_EQ(adjustmentFunctionStateMask, expectedAdjustmentFunctionStateMask);


    //
    // now pretend a short amount of time has passed, but not enough time to allow the value to have been increased
    //

    // given
    fixedMillis = 497;

    // when
    processRcAdjustments(&controlRateConfig, &rxConfig);

    EXPECT_EQ(controlRateConfig.rcRate8, 91);
    EXPECT_EQ(adjustmentFunctionStateMask, expectedAdjustmentFunctionStateMask);


    //
    // moving the switch back to the middle should immediately reset the state flag without increasing the value
    //


    // given
    rcData[AUX3] = PWM_RANGE_MIDDLE;

    // and
    fixedMillis = 498;

    // and
    expectedAdjustmentFunctionStateMask = adjustmentFunctionStateMask &
            ~(1 << (ADJUSTMENT_RC_RATE - ADJUSTMENT_INDEX_OFFSET));

    // when
    processRcAdjustments(&controlRateConfig, &rxConfig);

    EXPECT_EQ(controlRateConfig.rcRate8, 91);
    EXPECT_EQ(adjustmentFunctionStateMask, expectedAdjustmentFunctionStateMask);


    //
    // flipping the switch again, before the state reset would have occurred, allows the value to be increased again

    // given
    rcData[AUX3] = PWM_RANGE_MAX;

    // and
    expectedAdjustmentFunctionStateMask =
            (1 << (ADJUSTMENT_RC_RATE - ADJUSTMENT_INDEX_OFFSET));

    // and
    fixedMillis = 499;

    // when
    processRcAdjustments(&controlRateConfig, &rxConfig);

    // then
    EXPECT_EQ(controlRateConfig.rcRate8, 92);
    EXPECT_EQ(CALL_COUNTER(COUNTER_GENERATE_PITCH_ROLL_CURVE), 2);
    EXPECT_EQ(adjustmentFunctionStateMask, expectedAdjustmentFunctionStateMask);

    //
    // leaving the switch up, after the original timer would have reset the state should now NOT cause
    // the rate to increase, it should only increase after another 500ms from when the state was reset.
    //

    // given
    fixedMillis = 500;

    // when
    processRcAdjustments(&controlRateConfig, &rxConfig);

    // then
    EXPECT_EQ(controlRateConfig.rcRate8, 92);
    EXPECT_EQ(adjustmentFunctionStateMask, expectedAdjustmentFunctionStateMask);

    //
    // should still not be able to be increased
    //

    // given
    fixedMillis = 997;

    // when
    processRcAdjustments(&controlRateConfig, &rxConfig);

    // then
    EXPECT_EQ(controlRateConfig.rcRate8, 92);
    EXPECT_EQ(adjustmentFunctionStateMask, expectedAdjustmentFunctionStateMask);

    //
    // 500ms has now passed since the switch was returned to the middle, now that
    // switch is still in the UP position after the timer has elapses it should
    // be increased again.
    //

    // given
    fixedMillis = 998;

    // when
    processRcAdjustments(&controlRateConfig, &rxConfig);

    // then
    EXPECT_EQ(controlRateConfig.rcRate8, 93);
    EXPECT_EQ(CALL_COUNTER(COUNTER_GENERATE_PITCH_ROLL_CURVE), 3);
    EXPECT_EQ(adjustmentFunctionStateMask, expectedAdjustmentFunctionStateMask);

}

void changeProfile(uint8_t) {}
void accSetCalibrationCycles(uint16_t) {}
void gyroSetCalibrationCycles(uint16_t) {}
void applyAndSaveAccelerometerTrimsDelta(rollAndPitchTrims_t*) {}
void handleInflightCalibrationStickPosition(void) {}
void mwArm(void) {}
void feature(uint32_t) {}
void sensors(uint32_t) {}
void mwDisarm(void) {}

uint8_t armingFlags = 0;
int16_t heading;
uint8_t stateFlags = 0;
int16_t rcData[MAX_SUPPORTED_RC_CHANNEL_COUNT];
rxRuntimeConfig_t rxRuntimeConfig;
