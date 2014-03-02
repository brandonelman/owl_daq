#include "config.h"

void replace(char *str, char old, char new){
  char *temp = str;
  while(*temp)
  {
    if(*temp == old)
      *temp = new;
    ++temp;
  }
}

void strstrip(char *str){
  size_t size;
  char *end;
  
  size = strlen(str);
  if (!size)
    return;

  end = str + size - 1;
  while (end >= str && isspace(*end))
    end--;
  *(end+1) = '\0';

  while(*str && isspace(*str))
    str++;
}

void removeSpaces(char *str){
  char *inp = str;
  char *outp = str;
  int prevSpace = 0;

  while(*inp){
    if(isspace(*inp)) {
      if(!prevSpace){
        *outp++ = ' ';
        prevSpace = 1;
      }
    }
    else {
      *outp++ = *inp;
      prevSpace = 0;
    }
    ++inp;
  }
  *outp = '\0';
}

void saveConfig(FILE * conf_file, Config * config){
  char s[MAX_STRING_LENGTH];

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(s, "#Date %4d-%02d-%02d Time %02d:%02d\n", tm.tm_year+1900, 
                                                     tm.tm_mon+1, tm.tm_mday, 
                                                     tm.tm_hour, tm.tm_min); 
  fwrite(s, 1, strlen(s), conf_file);
  //sprintf(s, "#GIT VERSION %10s\n", VERSION);
  //fwrite(s, 1, strlen(s), conf_file);

  sprintf(s, "%s\n","[DAQ]"); //Contains all DAQ info
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10u\n", "num-pulses", config->num_pulses);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10d\n", "trigger-threshold-mv", config->trigger_threshold_mv);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10s\n", "output-folder", config->output_folder);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10s\n", "mode", config->mode);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10s\n", "tag", config->tag);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%s\n","[Hardware]"); //Contains all DAQ info
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10d\n", "subaddress", config->subaddress);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10s\n", "pmt-id", config->pmt_serial); 
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10s\n", "descriptor", config->descriptor);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10d\n", "pmt-voltage", config->pmt_voltage);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10u\n", "lamp-voltage", config->lamp_voltage);
  fwrite(s, 1, strlen(s), conf_file);
  sprintf(s, "%-23s = %10u\n", "lamp-frequency", config->lamp_frequency);
  fwrite(s, 1, strlen(s), conf_file);
}

void parseConfig(const char *fn, Config *config){
 FILE * fp;

 int value;
 ssize_t read;
 size_t len = 0;

 char * line = (char*)malloc(MAX_STRING_LENGTH*sizeof(char));
 char * paraF = (char*)malloc(MAX_STRING_LENGTH*sizeof(char)); //parameter name from file
 char * paraN = (char*)malloc(MAX_STRING_LENGTH*sizeof(char)); //Expected Parameter Names
 char * paraV = (char*)malloc(MAX_STRING_LENGTH*sizeof(char)); 

 fp = fopen(fn, "r");
 if (fp == NULL)
   exit(1);

 while((read = getline(&line, &len, fp)) != -1) {

   if (strstr(line, "=") != NULL) {
     replace(line, '=', ' ');
   }
   if (strstr(line, "#") != NULL) {
    replace(line, '#', '\0');   
   }
   removeSpaces(line);
   strstrip(line);
   sscanf(line, "%s %s", paraF, paraV);  //Set element of struct corresponding to "para" to "value" 

   sprintf(paraN, "%s", "pmt-id");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){
     strncpy(config->pmt_serial, paraV, MAX_STRING_LENGTH); 
     continue;
   }
   
   sprintf(paraN, "%s", "output-folder");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){
     strncpy(config->output_folder, paraV, MAX_STRING_LENGTH); 
     continue;
   }

   sprintf(paraN, "%s", "descriptor");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){
     strncpy(config->descriptor, paraV, MAX_STRING_LENGTH); 
     continue;
   }

   sprintf(paraN, "%s", "mode");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){
     strncpy(config->mode, paraV, MAX_STRING_LENGTH); 
     continue;
   }
   
   sprintf(paraN, "%s", "tag");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){
     strncpy(config->tag, paraV, MAX_STRING_LENGTH); 
     continue;
   }

   value = atoi(paraV); //All other values are integers rather than strings 
   sprintf(paraN, "%s", "trigger-threshold-mv");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){
     config->trigger_threshold_mv = value;
     continue;
   }
   sprintf(paraN, "%s", "num-pulses");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){ 
     config->num_pulses = value;
     continue;
   }
   sprintf(paraN, "%s", "run-num");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){ 
     config->run_num = value;
     continue;
   }
   sprintf(paraN, "%s", "subaddress");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){ 
     config->subaddress = value;
     continue;
   }
   sprintf(paraN, "%s", "pmt-voltage");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){ 
     config->pmt_voltage = value;
     continue;
   }
   sprintf(paraN, "%s", "lamp-voltage");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){ 
     config->lamp_voltage = value;
     continue;
   }
   sprintf(paraN, "%s", "lamp-frequency");
   if (strncmp(paraF, paraN, MAX_STRING_LENGTH) == 0){ 
     config->lamp_frequency = value;
     continue;
   }
 }
 if(line)
   free(line);
 if(paraF)
  free(paraF);
 if(paraN)
  free(paraN);
 if(paraV)
   free(paraV);
 return;
}

char * toString(){
}
