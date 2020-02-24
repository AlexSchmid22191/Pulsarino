#include <Arduino.h>

#define debug false

bool laser_on = false;

unsigned long firedPulses = 0;
unsigned long totalPulses = 0;

unsigned long last_fired;
unsigned long pulse_period=10000;

void listen_to_serial();
void fire_pulse();


void setup()
{
    //Set Pin mode
    pinMode(12,OUTPUT);
    digitalWrite(12,LOW);

    //Initialize time keeping
    last_fired = millis();

    //Serial Communication
    Serial.begin(9600);
}

void loop()
{
    listen_to_serial();

    if(laser_on && (millis() - last_fired) > pulse_period && firedPulses < totalPulses)
    {
        fire_pulse();
        last_fired = millis();
        firedPulses +=1;
    }
    if(firedPulses == totalPulses)
    {
        laser_on = false;
    }
}


void listen_to_serial()
{

    while(Serial.available())
    {
        //Read bytes until the start character (0x02) is encountered
        int x = Serial.read();
        if (x == 0x23)
        {
            char commandBuffer[25] = "";
            //memset(commandBuffer, 0, sizeof(commandBuffer));
            //Read into buffer
            Serial.readBytesUntil(0x0D, commandBuffer, sizeof(commandBuffer)-1);

            if(strncmp(commandBuffer, "start", 5) == 0)
            {
                //Search for p and f
                char *pulse_ptr = strchr(commandBuffer, 'p');
                char *freq_ptr = strchr(commandBuffer, 'f');

                totalPulses = strtoul(pulse_ptr + 1, nullptr, 0);

                double frequency = strtod(freq_ptr + 1, nullptr);
                pulse_period = (unsigned long) (1000/frequency);

                Serial.print("#start,");
                Serial.print(totalPulses);
                Serial.print(",");
                Serial.println(frequency,1);

                firedPulses = 0;
                laser_on = true;
            }

            if(strncmp(commandBuffer, "pause", 5) == 0)
            {
                laser_on = false;
                Serial.println("#pause");
            }

            if(strncmp(commandBuffer, "stop", 4) == 0)
            {
                totalPulses = 0;
                firedPulses = 0;
                laser_on = false;
                Serial.println("#stop");
            }

            if(strncmp(commandBuffer, "cont", 4) == 0)
            {
                laser_on = true;
                Serial.println("#cont");
            }

            if(strncmp(commandBuffer, "Py to Pulsar", 12) == 0)
            {
                Serial.println("Pulsar to Py");
            }

            if(strncmp(commandBuffer, "update", 6) ==0)
            {
                if(laser_on) Serial.print("#fire,");
                else Serial.print("#nfire,");

                Serial.print(firedPulses);
                Serial.print(',');
                Serial.println(totalPulses);
            }
        }
    }
}


void fire_pulse()
{
    digitalWrite(12, HIGH);
    delayMicroseconds(100);
    digitalWrite(12, LOW);
}