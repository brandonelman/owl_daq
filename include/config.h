#ifndef CONFIG_HH
#define CONFIG_HH 1

typedef struct Config{
  int trigger_threshold_mv;
  char mode[MAX_STRING_LENGTH];
  char tag[MAX_STRING_LENGTH];
  char output_folder[MAX_STRING_LENGTH];
  char pmt_serial[MAX_STRING_LENGTH];
  char descriptor[MAX_STRING_LENGTH];
  uint32_t num_pulses;
  uint32_t run_num;
  uint32_t pmt_voltage;
  uint32_t lamp_voltage;
  uint32_t lamp_frequency;
} Config;

void saveConfig(Config * config, FILE * conf_file){
  char s[MAX_STRING_LENGTH];
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(s, "#Date %4d-%02d-%02d Time %02d:%02d\n", tm.tm_year+1900, 
                                                     tm.tm_mon+1, tm.tm_mday, 
                                                     tm.tm_hour, tm.tm_min); 
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "#GIT VERSION %10s\n", VERSION);
  fwrite(s, 1, strlen(s), conf_file);

  sprintf(s, "%s\n","[VME]"); //Contains all DAQ info
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10u\n", "num-pulses", config->num_pulses);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10d\n", "trigger-threshold-mv", config->trigger_threshold_mv);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10s\n", "output-folder", config->output_folder);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%s\n","[Hardware]"); //Contains all DAQ info
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10s\n", "pmt-id", config->pmt_serial]); 
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10s\n", "descriptor", config->descriptor);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10u\n", "pmt-voltage", config->pmt_voltage);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10u\n", "lamp-voltage", config->lamp_voltage);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10u\n", "lamp-frequency", config->lamp_frequency);
  fwrite(s, 1, strlen(s), conf_file);
}
#endif
