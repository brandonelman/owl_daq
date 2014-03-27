#ifndef CONFIG_HH
#define CONFIG_HH 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#define ADC_POSITION 10
#define MAX_STRING_LENGTH 100
#define TIMEOUT 2

typedef struct Config{
  int trigger_threshold_mv;
  int subaddress;
  int pmt_voltage;
  char mode[MAX_STRING_LENGTH];
  char tag[MAX_STRING_LENGTH];
  char output_folder[MAX_STRING_LENGTH];
  char pmt_serial[MAX_STRING_LENGTH];
  char descriptor[MAX_STRING_LENGTH];
  uint32_t num_pulses;
  uint32_t run_num;
  uint32_t lamp_voltage;
  uint32_t lamp_frequency;
} Config;

//String Manipulation functions for Parsing Config Files
void replace(char *str, char old, char new);
void strstrip(char *str);
void removeSpaces(char *str);

//Save Config data to File
void saveConfig(FILE * conf_file, Config * config);
void parseConfig(const char *fn, Config *config);
#endif
