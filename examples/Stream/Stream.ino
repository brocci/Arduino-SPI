#include "GPIO.h"
#include "SPI.h"
#include "Driver/SRAM.h"

// #define USE_SOFTWARE_SPI
#define USE_HARDWARE_SPI

#if defined(USE_SOFTWARE_SPI)
#include "Software/SPI.h"
#if defined(ARDUINO_attiny)
Software::SPI<BOARD::D1, BOARD::D2, BOARD::D3> spi;
#else
Software::SPI<BOARD::D11, BOARD::D12, BOARD::D13> spi;
#endif
#elif defined(USE_HARDWARE_SPI)
#include "Hardware/SPI.h"
Hardware::SPI spi;
#endif

SRAM<BOARD::D10> sram(spi);
SRAM<BOARD::D10>::Stream<10000> ios(sram);
SRAM<BOARD::D10>::Stream<10000> temps(sram, 10000);

const int N = 1000;
int count = 0;
uint32_t s0, m0, m1, m2, m3, m4, m5, m6, m7;

void setup()
{
  Serial.begin(57600);
  while (!Serial);
}

void loop()
{
  // Print N analog samples to serial stream
  s0 = micros();
  for (int i = 0; i < N; i++) {
    Serial.println(analogRead(A0));
  }
  Serial.println();
  Serial.flush();
  m0 = micros() - s0;

  // Print N analog samples to sram stream
  s0 = micros();
  for (int i = 0; i < N; i++) {
    ios.println(analogRead(A0));
  }
  ios.println();
  m1 = micros() - s0;

  // Transfer data between two sram streams
  count = ios.available();
  s0 = micros();
  while (ios.available())
    temps.write(ios.read());
  m2 = micros() - s0;

  // Print data from sram stream to serial stream
  s0 = micros();
  while (temps.available())
    Serial.write(temps.read());
  Serial.flush();
  m3 = micros() - s0;

  // Write to sram stream
  s0 = micros();
  for (int i = 0; i < count; i++) ios.write(i);
  m4 = micros() - s0;

  // Read from sram stream
  s0 = micros();
  uint16_t sum = 0;
  for (int i = 0; i < count; i++) sum += ios.read();
  m5 = micros() - s0;

  // Write N analog samples to sram stream
  s0 = micros();
  int sample;
  for (int i = 0; i < N; i++) {
    sample = analogRead(A0);
    ios.write((const uint8_t*) &sample, sizeof(sample));
  }
  m6 = micros() - s0;

  // Read N analog samples from sram stream and print to serial
  s0 = micros();
  while (ios.available()) {
    ios.readBytes((uint8_t*) &sample, sizeof(sample));
    Serial.println(sample);
  }
  Serial.flush();
  m7 = micros() - s0;

  Serial.print(F("Samples, N = "));
  Serial.println(N);
  Serial.print(F("Serial.print, m0 = "));
  Serial.println(m0 / N);
  Serial.print(F("SRAM::Stream.print, m1 = "));
  Serial.println(m1 / N);
  Serial.println();
  Serial.print(F("SRAM::Stream.available, count = "));
  Serial.println(count);
  Serial.print(F("SRAM::Stream.write/read, m2 = "));
  Serial.println(m2 / count);
  Serial.print(F("SRAM::Stream.read/Serial.write, m3 = "));
  Serial.println(m3 / count);
  Serial.print(F("SRAM::Stream.write, m4 = "));
  Serial.println(m4 / count);
  Serial.print(F("SRAM::Stream.read, m5 = "));
  Serial.println(m5 / count);
  Serial.println();
  Serial.print(F("SRAM::Stream.available, N*2 = "));
  Serial.println(N * 2);
  Serial.print(F("SRAM::Stream.write(2), m6 = "));
  Serial.println(m6 / N);
  Serial.print(F("SRAM::Stream.read(2)/Serial.print, m7 = "));
  Serial.println(m7 / N);
  Serial.println();

  delay(1000);
}
