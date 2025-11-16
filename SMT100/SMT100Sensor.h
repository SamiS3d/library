#ifndef SMT100SENSOR_H
#define SMT100SENSOR_H

#include <ModbusMaster.h>
#include "config.h"

class SMT100Sensor {
public:
    struct Reading {
        float temperature;
        float waterContent;
        float permittivity;
        uint16_t rawCount;
        bool isValid;
    };

    SMT100Sensor(uint8_t modbusAddress);
    void begin();
    Reading read();
    void quickWarmup();  // ADDED: للإحماء السريع
    
private:
    ModbusMaster node;
    uint8_t address;
    Reading lastReading;
    float medianFilterTemp[FILTER_WINDOW];
    float medianFilterVWC[FILTER_WINDOW];
    float medianFilterPerm[FILTER_WINDOW];
    int filterIndex;
    bool isWarmedUp;  // ADDED: للتحقق من الإحماء
    
    float calculateMedian(float newValue, float* buffer);
    float quickSelectMedian(float arr[], int n);
};

#endif // SMT100SENSOR_H