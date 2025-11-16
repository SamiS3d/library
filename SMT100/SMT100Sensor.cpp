#include "SMT100Sensor.h"

SMT100Sensor::SMT100Sensor(uint8_t modbusAddress) : filterIndex(0), address(modbusAddress), isWarmedUp(false) {
    memset(medianFilterTemp, 0, sizeof(medianFilterTemp));
    memset(medianFilterVWC, 0, sizeof(medianFilterVWC));
    memset(medianFilterPerm, 0, sizeof(medianFilterPerm));
}

void SMT100Sensor::begin() {
    Serial2.begin(9600, SERIAL_8E1, SMT100_RX_PIN, SMT100_TX_PIN);
    node.begin(address, Serial2);
}

void SMT100Sensor::quickWarmup() {
    Serial.printf("Warming up sensor %d...\n", address);
    
    for(int i = 0; i < FILTER_WINDOW; i++) {
        uint8_t result;
        uint16_t raw;
        
        // قراءة سريعة بدون delay كبير
        result = node.readHoldingRegisters(0x0000, 1);
        if (result == node.ku8MBSuccess) {
            raw = node.getResponseBuffer(0);
            medianFilterTemp[i] = raw / 100.0 - 100;
        }
        delay(50);  // delay قصير جداً
        
        result = node.readHoldingRegisters(0x0001, 1);
        if (result == node.ku8MBSuccess) {
            raw = node.getResponseBuffer(0);
            medianFilterVWC[i] = raw / 100.0;
        }
        delay(50);
        
        result = node.readHoldingRegisters(0x0002, 1);
        if (result == node.ku8MBSuccess) {
            raw = node.getResponseBuffer(0);
            medianFilterPerm[i] = raw / 10.0;
        }
        delay(50);
    }
    
    isWarmedUp = true;
    Serial.printf("Sensor %d ready!\n", address);
}

SMT100Sensor::Reading SMT100Sensor::read() {
    Reading reading;
    reading.isValid = true;
    uint8_t result;
    uint16_t raw;
    
    result = node.readHoldingRegisters(0x0000, 1);
    if (result == node.ku8MBSuccess) {
        raw = node.getResponseBuffer(0);
        float temp = raw / 100.0 - 100;
        reading.temperature = isWarmedUp ? calculateMedian(temp, medianFilterTemp) : temp;  // MODIFIED
    } else {
        reading.isValid = false;
    }
    
    delay(50);  // MODIFIED: خفضنا من 100 إلى 50
    
    result = node.readHoldingRegisters(0x0001, 1);
    if (result == node.ku8MBSuccess) {
        raw = node.getResponseBuffer(0);
        float vwc = raw / 100.0;
        reading.waterContent = isWarmedUp ? calculateMedian(vwc, medianFilterVWC) : vwc;  // MODIFIED
    } else {
        reading.isValid = false;
    }
    
    delay(50);  // MODIFIED: خفضنا من 100 إلى 50

    result = node.readHoldingRegisters(0x0002, 1);
    if (result == node.ku8MBSuccess) {
        raw = node.getResponseBuffer(0);
        float perm = raw / 10.0;
        reading.permittivity = isWarmedUp ? calculateMedian(perm, medianFilterPerm) : perm;  // MODIFIED
    } else {
        reading.isValid = false;
    }
    
    delay(50);  // MODIFIED: خفضنا من 100 إلى 50

    result = node.readHoldingRegisters(0x0003, 1);
    if (result == node.ku8MBSuccess) {
        reading.rawCount = node.getResponseBuffer(0);
    } else {
        reading.isValid = false;
    }

    lastReading = reading;
    return reading;
}

float SMT100Sensor::calculateMedian(float newValue, float* buffer) {
    buffer[filterIndex] = newValue;
    
    float sortBuffer[FILTER_WINDOW];
    for (int i = 0; i < FILTER_WINDOW; i++) {
        sortBuffer[i] = buffer[i];
    }
    
    float median = quickSelectMedian(sortBuffer, FILTER_WINDOW);
    
    filterIndex = (filterIndex + 1) % FILTER_WINDOW;
    
    return median;
}

float SMT100Sensor::quickSelectMedian(float arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                float temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
    
    if (n % 2 == 0) {
        return (arr[n/2 - 1] + arr[n/2]) / 2.0;
    } else {
        return arr[n/2];
    }
}