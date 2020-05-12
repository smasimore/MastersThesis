/**
 * Integration test to verify AnalogInDevice read functionality.
 *
 *                         ---- TEST PROCEDURE ----
 *
 * 1) This script can only test 4 analog in pins at a time. Set AIN_START_PIN
 *    in AnalogInDeviceTest.cpp to 0, 4, 8, or 12. This will test
 *    AnalogInDevices on pins AIN_START_PIN, AIN_START_PIN + 1,
 *    AIN_START_PIN + 2, and AIN_START_PIN + 3.
 *
 * 2) Connect breakout board to sbRIO. Wire AOUT0-3 to the 4 AINs being tested
 *    in ascending order. For example, if AIN_START_PIN is 4, you would wire
 *
 *      AOUT0 ---- AIN4
 *      AOUT1 ---- AIN5
 *      AOUT2 ---- AIN6
 *      AOUT3 ---- AIN7
 *
 * 3) This step is only necessary if AIN_START_PIN < 8 (see note (1) at the top
 *    of AnalogInDevice.hpp).
 *
 *    Wire each AIN's differential partner to AGND on the breakout board. For
 *    an AIN pin X < 8, its partner is pin X + 8. For example, if AIN_START_PIN
 *    is 4, you would wire
 *
 *      AIN12 ---- AGND
 *      AIN13 ---- AGND
 *      AIN14 ---- AGND
 *      AIN15 ---- AGND
 *
 *    This removes noise that would otherwise skew voltage differentials.
 *      
 * 4) Build and run script. Script should complete within 10 seconds.
 *
 * 5) Verify that all tests passed. The average test duration printed at the
 *    bottom should not exceed 10,500,000 ns. The max test duration should not
 *    exceed 13,000,000 ns.
 */

 #ifndef ANALOG_IN_DEVICE_TEST_HPP
 #define ANALOG_IN_DEVICE_TEST_HPP

namespace AnalogInDeviceTest
{
    void main (int, char**);
}

 #endif