#include <unity.h>
#include <az_core.h>

#include <telemetry.h>
#include <basesensor.h>
#include "sensor_stub.h"
#include "stdio.h"

void hexDump (
    const char * desc,
    const void * addr,
    const int len,
    int perLine
);


char buf[200];

void setUp(void) {
    // set stuff up here
    //printf("setup");
}

void tearDown(void) {
    // clean stuff up here
    //printf("teardown");
}

void test_telemetry_store_measurement(void) {
    int bytesStored = 0;
    int numStored = 0;
    int telemetryId = 1;
    printf("\n");
    printf("---- buf=0x%x\n", buf);
    
    Telemetry *t = new Telemetry(
        buf, sizeof(buf), &bytesStored, &numStored, &telemetryId
    );

    memset(buf, 0, sizeof(buf));
    az_span m1 = az_span_create_from_str((char*)"{\"id\":14,\"ts\":1656142750,\"t1\":25.75,\"p\":96831.41,\"h\":50.54}");
    az_span m2 = az_span_create_from_str((char*)"{\"id\":15,\"ts\":1656145620,\"t1\":25.75,\"p\":96830.58,\"h\":51.42}");
    TEST_ASSERT_EQUAL(59, az_span_size(m2)); // strlen does not include terminating 0

    //az_span measSpan = az_span_create((uint8_t*)meas, strlen(meas));
    t->storeMeasurement(m1);

    TEST_ASSERT_EQUAL(1, numStored);
    TEST_ASSERT_EQUAL(60, t->getStoredSize());
    TEST_ASSERT_EQUAL(140, t->getRemainingSize());
    TEST_ASSERT_EQUAL_STRING_LEN(az_span_ptr(m1), buf, az_span_size(m1));

    hexDump("first", buf, sizeof(buf), 32);

    t->storeMeasurement(m2);
    TEST_ASSERT_EQUAL(2, numStored);
    TEST_ASSERT_EQUAL(120, t->getStoredSize());
    TEST_ASSERT_EQUAL(80, t->getRemainingSize());
    TEST_ASSERT_TRUE(t->doesMeasurementFit(m2));

    t->storeMeasurement(m2);
    TEST_ASSERT_EQUAL(20, t->getRemainingSize());
    TEST_ASSERT_FALSE(t->doesMeasurementFit(m2));

    az_span m3 = az_span_create_from_str((char*)"1234567890123456789");
    TEST_ASSERT_TRUE(t->doesMeasurementFit(m3));
    m3 = az_span_create_from_str((char*)"12345678901234567890");
    TEST_ASSERT_FALSE(t->doesMeasurementFit(m3));

    hexDump("third", buf, sizeof(buf), 32);
    printf("[%s]\n", buf);
    
    char statusBuf[100];
    t->buildStatus(statusBuf, sizeof(statusBuf));
    TEST_ASSERT_EQUAL_STRING("MEAS_STORE: numStored=3, storedBytes=180, remainingBytes=20", statusBuf);

    t->reset();
    TEST_ASSERT_EQUAL(0, numStored);
    TEST_ASSERT_EQUAL(0, t->getStoredSize());
    TEST_ASSERT_EQUAL(200, t->getRemainingSize());

    t->storeMeasurement(m1);

    TEST_ASSERT_EQUAL(1, numStored);
    TEST_ASSERT_EQUAL(60, t->getStoredSize());
    TEST_ASSERT_EQUAL(140, t->getRemainingSize());
    TEST_ASSERT_EQUAL_STRING_LEN(az_span_ptr(m1), buf, az_span_size(m1));

}

void test_telemetry_build_telemetry(void) {
    int bytesStored = 0;
    int numStored = 0;
    int telemetryId = 1;

    static uint8_t telemetry_payload[100];
    az_span meas = AZ_SPAN_FROM_BUFFER(telemetry_payload);
    Telemetry *t = new Telemetry(
        buf, sizeof(buf), &bytesStored, &numStored, &telemetryId
    );
    
    SensorStub *s1 = new SensorStub(1.0f, true, 2.0f, true, 3.0f, true);
    t->addSensor(s1);
    
    t->buildTelemetryPayload(meas, &meas);
    t->storeMeasurement(meas);

    // TODO: mock time
    TEST_ASSERT_EQUAL_STRING_LEN("{\"id\":1,", az_span_ptr(meas), 8);
    TEST_ASSERT_EQUAL_STRING_LEN(",\"t1\":1.00,\"p\":2.00,\"h\":3.00}", az_span_ptr(meas)+23, az_span_size(meas)-23);
    printf("meas: %s\n", az_span_ptr(meas));

    char statusBuf[100];
    t->buildStatus(statusBuf, sizeof(statusBuf));
    TEST_ASSERT_EQUAL_STRING("MEAS_STORE: numStored=1, storedBytes=53, remainingBytes=147", statusBuf);

    meas = AZ_SPAN_FROM_BUFFER(telemetry_payload);
    t->buildTelemetryPayload(meas, &meas);
    t->storeMeasurement(meas);

    TEST_ASSERT_EQUAL_STRING_LEN("{\"id\":2,", az_span_ptr(meas), 8);
    TEST_ASSERT_EQUAL_STRING_LEN(",\"t1\":1.00,\"p\":2.00,\"h\":3.00}", az_span_ptr(meas)+23, az_span_size(meas)-23);

    t->buildStatus(statusBuf, sizeof(statusBuf));
    TEST_ASSERT_EQUAL_STRING("MEAS_STORE: numStored=2, storedBytes=106, remainingBytes=94", statusBuf);

    SensorStub *s2 = new SensorStub(4.0f, true, 2.0f, false, 3.0f, false);
    t->addSensor(s2);

    meas = AZ_SPAN_FROM_BUFFER(telemetry_payload);
    t->buildTelemetryPayload(meas, &meas);
    t->storeMeasurement(meas);

    TEST_ASSERT_EQUAL_STRING_LEN("{\"id\":3,", az_span_ptr(meas), 8);
    TEST_ASSERT_EQUAL_STRING_LEN(",\"t1\":1.00,\"p\":2.00,\"h\":3.00,\"t2\":4.00}", az_span_ptr(meas)+23, az_span_size(meas)-23);
}


int main( int argc, char **argv) {
    printf("starting\n");
    UNITY_BEGIN();
    RUN_TEST(test_telemetry_store_measurement);
    printf("starting2\n");
    RUN_TEST(test_telemetry_build_telemetry);
    UNITY_END();
    printf("done");
    return 0;
}



void hexDump (
    const char * desc,
    const void * addr,
    const int len,
    int perLine
) {
    // Silently ignore silly per-line values.

    if (perLine < 4 || perLine > 64) perLine = 16;

    int i;
    unsigned char buff[perLine+1];
    const unsigned char * pc = (const unsigned char *)addr;

    // Output description if given.

    if (desc != NULL) printf ("%s:\n", desc);

    // Length checks.

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    // Process every byte in the data.

    for (i = 0; i < len; i++) {
        // Multiple of perLine means new or first line (with line offset).

        if ((i % perLine) == 0) {
            // Only print previous-line ASCII buffer for lines beyond first.

            if (i != 0) printf ("  %s\n", buff);

            // Output the offset of current line.

            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.

        printf (" %02x", pc[i]);

        // And buffer a printable ASCII character for later.

        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
            buff[i % perLine] = '.';
        else
            buff[i % perLine] = pc[i];
        buff[(i % perLine) + 1] = '\0';
    }

    // Pad out last line if not exactly perLine characters.

    while ((i % perLine) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII buffer.

    printf ("  %s\n", buff);
}
