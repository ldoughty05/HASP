/*--------------------------------------------------------------------
Name:   Luke Doughty
Date:   4/10/2025
File:   utils.h
UNL AerospaceExperimental Payloads Team

Purp:   General purpose methods, including methods helpful for debugging.
--------------------------------------------------------------------*/
#include <Arduino.h>

void blink(int time);

double average(double *array, int size);

int measureStringLength(char *string);