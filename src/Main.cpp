#include <Arduino.h>

#define debug false

bool laser_on = false;

unsigned long firedPulses = 0;
unsigned long totalPulses = 0;

unsigned long last_fired;
unsigned long pulse_period=10000;

const byte pin_laser = 9;
const byte pin_relay = 3;
const byte pin_speaker = 8;

void listen_to_serial();
void fire_pulse();
void alarm();
void blocking_tone(byte pin, unsigned int frequency, unsigned long duration);

void setup()
{
    //Set Pin mode
    pinMode(pin_laser, OUTPUT);
    digitalWrite(pin_laser, LOW);

    pinMode(pin_relay, OUTPUT);
    digitalWrite(pin_relay, LOW);

    pinMode(pin_speaker,OUTPUT);

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

                alarm();
                firedPulses = 0;
                laser_on = true;
            }

            if(strncmp(commandBuffer, "pause", 5) == 0)
            {
                laser_on = false;
                Serial.println("#pause");
            }

            
            if(strncmp(commandBuffer, "kill_power", 10) == 0)
            {
                laser_on = false;
                digitalWrite(pin_relay, HIGH);
                Serial.println("#power killed");
            }

            if(strncmp(commandBuffer, "restore_power", 13) == 0)
            {
                laser_on = false;
                digitalWrite(pin_relay, LOW);
                Serial.println("#power restored");
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
                alarm();
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
    digitalWrite(9, HIGH);
    delayMicroseconds(100);
    digitalWrite(9, LOW);
}

void blocking_tone(byte pin, unsigned int frequency, unsigned long duration)
{
    tone(pin, frequency, duration);
    delay(duration);
}

void alarm()
{
    blocking_tone(pin_speaker, 500, 500);
    delay(250);
    blocking_tone(pin_speaker, 500, 500);
    delay(250);
    blocking_tone(pin_speaker, 500, 500);
}