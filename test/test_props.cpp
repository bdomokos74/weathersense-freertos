#include <unity.h>
#include "props.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_props_init() {
    Props *p = new Props();

    TEST_ASSERT_EQUAL(1, p->getDoSleep());
}


int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_props_init);

    UNITY_END();
}