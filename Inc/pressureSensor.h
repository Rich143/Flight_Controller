#ifndef __PRESSURE_SENSOR_H
#define __PRESSURE_SENSOR_H
FC_Status pressureSensor_GetTemp(int16_t *Tout);
FC_Status pressureSensor_GetPressure(int32_t *Pout);
FC_Status pressureSensor_GetAltitude(int32_t *altitude_out);

void vPressureSensorTask(void *pvParameters);

#endif
