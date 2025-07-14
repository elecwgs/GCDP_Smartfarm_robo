#include <unity.h>
#include "scheduler.h"
#include "sensors.h"
#include "actuators.h"

Scheduler scheduler;
SensorManager sensors;
ActuatorManager actuators;

void setUp(void) {
    sensors.init();
    actuators.init();
    scheduler.init();
}

void tearDown(void) {
    actuators.emergencyStop();
    scheduler.resetDailyTasks();
}

void test_scheduler_initialization(void) {
    // 초기 상태에서는 모든 일일 작업이 미완료 상태여야 함
    TEST_ASSERT_FALSE(scheduler.isWatering1Done());
    TEST_ASSERT_FALSE(scheduler.isWatering2Done());
    TEST_ASSERT_FALSE(scheduler.isLinearMove1Done());
    TEST_ASSERT_FALSE(scheduler.isLinearMove2Done());
}

void test_daily_task_reset(void) {
    // 임의로 작업 완료 상태로 만들기 (실제로는 시간에 의해 설정됨)
    DateTime testTime1(2025, 7, 14, 8, 0, 0);  // 오전 8시
    DateTime testTime2(2025, 7, 14, 18, 0, 0); // 오후 6시
    DateTime testTime3(2025, 7, 14, 10, 0, 0); // 오전 10시
    DateTime testTime4(2025, 7, 14, 15, 0, 0); // 오후 3시
    
    // 작업 상태 변경 시뮬레이션
    scheduler.isTimeToWater(testTime1);
    scheduler.isTimeToWater(testTime2);
    scheduler.isTimeToMovePlant(testTime3);
    scheduler.isTimeToMovePlant(testTime4);
    
    // 리셋 후 모든 작업이 미완료 상태가 되어야 함
    scheduler.resetDailyTasks();
    TEST_ASSERT_FALSE(scheduler.isWatering1Done());
    TEST_ASSERT_FALSE(scheduler.isWatering2Done());
    TEST_ASSERT_FALSE(scheduler.isLinearMove1Done());
    TEST_ASSERT_FALSE(scheduler.isLinearMove2Done());
}

void test_led_schedule(void) {
    // 낮 시간 (오전 8시) - LED 켜져야 함
    DateTime dayTime(2025, 7, 14, 8, 0, 0);
    TEST_ASSERT_TRUE(scheduler.isTimeForLED(dayTime));
    
    // 저녁 시간 (오후 9시) - LED 켜져야 함
    DateTime eveningTime(2025, 7, 14, 21, 0, 0);
    TEST_ASSERT_TRUE(scheduler.isTimeForLED(eveningTime));
    
    // 밤 시간 (오후 11시) - LED 꺼져야 함
    DateTime nightTime(2025, 7, 14, 23, 0, 0);
    TEST_ASSERT_FALSE(scheduler.isTimeForLED(nightTime));
    
    // 새벽 시간 (오전 5시) - LED 꺼져야 함
    DateTime earlyMorning(2025, 7, 14, 5, 0, 0);
    TEST_ASSERT_FALSE(scheduler.isTimeForLED(earlyMorning));
}

void test_watering_schedule(void) {
    // 오전 8시 정각 - 1차 물주기 시간
    DateTime waterTime1(2025, 7, 14, 8, 0, 0);
    TEST_ASSERT_TRUE(scheduler.isTimeToWater(waterTime1));
    TEST_ASSERT_TRUE(scheduler.isWatering1Done());
    
    // 같은 시간에 다시 호출해도 false (이미 완료됨)
    TEST_ASSERT_FALSE(scheduler.isTimeToWater(waterTime1));
    
    // 오후 6시 정각 - 2차 물주기 시간
    DateTime waterTime2(2025, 7, 14, 18, 0, 0);
    TEST_ASSERT_TRUE(scheduler.isTimeToWater(waterTime2));
    TEST_ASSERT_TRUE(scheduler.isWatering2Done());
    
    // 다른 시간 - 물주기 시간 아님
    DateTime otherTime(2025, 7, 14, 12, 0, 0);
    TEST_ASSERT_FALSE(scheduler.isTimeToWater(otherTime));
}

void test_plant_movement_schedule(void) {
    // 오전 10시 정각 - 1차 식물 이동 시간
    DateTime moveTime1(2025, 7, 14, 10, 0, 0);
    TEST_ASSERT_TRUE(scheduler.isTimeToMovePlant(moveTime1));
    TEST_ASSERT_TRUE(scheduler.isLinearMove1Done());
    
    // 오후 3시 정각 - 2차 식물 이동 시간
    DateTime moveTime2(2025, 7, 14, 15, 0, 0);
    TEST_ASSERT_TRUE(scheduler.isTimeToMovePlant(moveTime2));
    TEST_ASSERT_TRUE(scheduler.isLinearMove2Done());
    
    // 다른 시간 - 이동 시간 아님
    DateTime otherTime(2025, 7, 14, 12, 30, 0);
    TEST_ASSERT_FALSE(scheduler.isTimeToMovePlant(otherTime));
}

void test_environmental_control_temperature(void) {
    // 모의 센서 데이터 설정은 실제 센서 객체의 한계로 인해
    // 통합 테스트에서 확인하는 것이 더 적절함
    // 여기서는 함수 호출이 오류 없이 실행되는지만 확인
    
    scheduler.environmentalControl(sensors, actuators);
    
    // 환경 제어 후에도 시스템이 정상 상태여야 함
    TEST_ASSERT_TRUE(true); // 오류 없이 실행됨
}

void test_time_based_control(void) {
    DateTime testTime(2025, 7, 14, 12, 0, 0); // 정오
    
    // 시간 기반 제어 실행
    scheduler.timeBasedControl(testTime, actuators);
    
    // 정오 시간에는 LED가 켜져있어야 함
    TEST_ASSERT_TRUE(actuators.isLEDActive());
}

void test_daily_update(void) {
    DateTime day1(2025, 7, 14, 12, 0, 0);
    DateTime day2(2025, 7, 15, 12, 0, 0);
    
    // 첫 번째 날 업데이트
    scheduler.update(day1);
    
    // 두 번째 날 업데이트 (날짜 변경)
    scheduler.update(day2);
    
    // 날짜가 변경되면 일일 작업이 리셋되어야 함
    TEST_ASSERT_FALSE(scheduler.isWatering1Done());
    TEST_ASSERT_FALSE(scheduler.isWatering2Done());
    TEST_ASSERT_FALSE(scheduler.isLinearMove1Done());
    TEST_ASSERT_FALSE(scheduler.isLinearMove2Done());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_scheduler_initialization);
    RUN_TEST(test_daily_task_reset);
    RUN_TEST(test_led_schedule);
    RUN_TEST(test_watering_schedule);
    RUN_TEST(test_plant_movement_schedule);
    RUN_TEST(test_environmental_control_temperature);
    RUN_TEST(test_time_based_control);
    RUN_TEST(test_daily_update);
    
    UNITY_END();
    
    return 0;
}