#include <unity.h>
#include "actuators.h"

ActuatorManager actuators;

void setUp(void) {
    actuators.init();
}

void tearDown(void) {
    actuators.emergencyStop();
}

void test_actuator_initialization(void) {
    // 초기 상태는 모든 액추에이터가 OFF여야 함
    TEST_ASSERT_FALSE(actuators.isPumpActive());
    TEST_ASSERT_FALSE(actuators.isFanActive());
    TEST_ASSERT_FALSE(actuators.isLEDActive());
    TEST_ASSERT_FALSE(actuators.isWateringActive());
}

void test_pump_control(void) {
    // 펌프 켜기 테스트
    actuators.activatePumps(true);
    TEST_ASSERT_TRUE(actuators.isPumpActive());
    
    // 펌프 끄기 테스트
    actuators.activatePumps(false);
    TEST_ASSERT_FALSE(actuators.isPumpActive());
}

void test_fan_control(void) {
    // 팬 켜기 테스트
    actuators.activateFans(true);
    TEST_ASSERT_TRUE(actuators.isFanActive());
    
    // 팬 끄기 테스트
    actuators.activateFans(false);
    TEST_ASSERT_FALSE(actuators.isFanActive());
}

void test_led_control(void) {
    // LED 켜기 테스트
    actuators.activateLED(true);
    TEST_ASSERT_TRUE(actuators.isLEDActive());
    
    // LED 끄기 테스트
    actuators.activateLED(false);
    TEST_ASSERT_FALSE(actuators.isLEDActive());
}

void test_watering_system(void) {
    // 물주기 시작 테스트
    actuators.startWatering();
    TEST_ASSERT_TRUE(actuators.isWateringActive());
    TEST_ASSERT_TRUE(actuators.isPumpActive());
    
    // 물주기 중지 테스트
    actuators.stopWatering();
    TEST_ASSERT_FALSE(actuators.isWateringActive());
    TEST_ASSERT_FALSE(actuators.isPumpActive());
}

void test_watering_duration(void) {
    // 짧은 지속시간으로 테스트 (실제로는 30초)
    actuators.startWatering();
    TEST_ASSERT_TRUE(actuators.isWateringActive());
    
    // 지속시간이 끝나지 않았을 때
    bool completed = actuators.checkWateringDuration();
    TEST_ASSERT_FALSE(completed); // 아직 완료되지 않음
    TEST_ASSERT_TRUE(actuators.isWateringActive());
}

void test_emergency_stop(void) {
    // 모든 액추에이터 켜기
    actuators.activatePumps(true);
    actuators.activateFans(true);
    actuators.activateLED(true);
    actuators.startWatering();
    
    // 비상 정지 실행
    actuators.emergencyStop();
    
    // 모든 액추에이터가 꺼져야 함
    TEST_ASSERT_FALSE(actuators.isPumpActive());
    TEST_ASSERT_FALSE(actuators.isFanActive());
    TEST_ASSERT_FALSE(actuators.isLEDActive());
    TEST_ASSERT_FALSE(actuators.isWateringActive());
}

void test_linear_actuator(void) {
    // 선형 액추에이터 이동 테스트
    // 실제 하드웨어 없이는 동작만 확인
    actuators.moveLinearActuator();
    // 이동 완료 후에도 다른 상태는 변경되지 않아야 함
    TEST_ASSERT_FALSE(actuators.isPumpActive());
    TEST_ASSERT_FALSE(actuators.isFanActive());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_actuator_initialization);
    RUN_TEST(test_pump_control);
    RUN_TEST(test_fan_control);
    RUN_TEST(test_led_control);
    RUN_TEST(test_watering_system);
    RUN_TEST(test_watering_duration);
    RUN_TEST(test_linear_actuator);
    RUN_TEST(test_emergency_stop);
    
    UNITY_END();
    
    return 0;
}