/*--------------------------------------------------------------------
Name:   Luke Doughty
Date:   4/10/2025
File:   utils.c
UNL AerospaceExperimental Payloads Team

Purp:   General purpose methods, including methods helpful for debugging.
--------------------------------------------------------------------*/
#include <Arduino.h>

void blink(int time){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(time);
  digitalWrite(LED_BUILTIN, LOW);
  delay(time);
}

double average(double *array, int size) {
  double sum = 0.0;
  for (int i = 0; i < size; i++) {
    sum += array[i];
  }
  return sum / size;
}

int measureStringLength(char *string) {
  int length = 0;
  while (string[length] != '\0') {
    length++;
  }
  return length;
}