#ifndef LINUX
#define LINUX 1
#endif


//#include <time.h>
//#include <stdio.h>
#include "crate_lib.h" 
#include "config.h"

int scanCrate(short crate_id){
  short err;
  int i;
  unsigned int scan_result;
  err = CSCAN(crate_id, &scan_result);
  if (err != CRATE_OK){
    printf("Error occurs scanning the CRATE: %d\n", err);
    return 1;
  }

  for (i = 0; i < 24; i++) 
    if (scan_result & (1 << i)) 
      printf("The slot %d is filled with a card.\n", i+1);
  return 0;
}

int saveData(FILE *file, int num_pulses, unsigned short *data){
  int i;
  char s[30];
  for(i  = 0; i < num_pulses; i++) {
      sprintf(s, "%u\n", data[i]);
      fwrite(s, 1, strlen(s), file);
  }
  return 0;
}

int main(int argc, char **argv) {
  char s[30];
  FILE *conf_file;
  FILE *data_file;
  short crate_id;
  short err;
  int i,j;
  CRATE_OP cr_op;
  char file_name[100];
  unsigned int scan_result;
  short found_lam = 0;
  Config config;


  parseConfig(argv[1], &config);

  sprintf(file_name, "%s/%s_%05d/%s_%05d_%s.conf", config.output_folder, 
          config.mode, config.run_num, config.mode, config.run_num, config.tag);
  conf_file = fopen(file_name, "w+b");
  printf("Filename for conf_file = %s\n", file_name);
  saveConfig(conf_file, &config);
  fclose(conf_file);
 
  sprintf(file_name, "%s/%s_%05d/%s_%05d_%s.dat",config.output_folder,  
          config.mode, config.run_num, config.mode, config.run_num, config.tag);
  data_file = fopen(file_name, "w+b"); 

  unsigned short integrated_pulses[config.num_pulses];


  //Initialize crate_id, which is necessary for all CAMAC functions
  crate_id = CROPEN("192.168.0.98");
  if (crate_id < 0) {
    printf("error %d opening connection with CAMAC Controller\n", crate_id);
    return 1;
  }
  else
    printf("Opened board!\n");

  cr_op.N = ADC_POSITION; //Position in Crate of the ADC
  cr_op.A = config.subaddress; //Subaddress 
  printf("cr_op.N = %d ; cr_op.A = %d\n", cr_op.N, cr_op.A);

  err = CCCC(crate_id);
  if (err != CRATE_OK){
    printf("Error occuring with Crate Clear: %d\n", err);
    return 1;
  }

  err = CRTOUT(crate_id, TIMEOUT);
  if (err < 0) {
    printf("Error occurs setting timeout: %d\n", err);
    return 1;
  }

  cr_op.F = 26; //Enable LAM 
  err = CFSA(crate_id, &cr_op);
  if (err < 0 || cr_op.X != 1){ 
    printf("error executing CSSA Operation: %d\n", err);
    CRCLOSE(crate_id);
    return 1;
  }

  // MAIN ACQUISITION LOOP
  for (i = 0; i < config.num_pulses; i++){
//  err = CCCC(crate_id);
//  if (err != CRATE_OK){
//    printf("Error occuring with Crate Clear: %d\n", err);
//    break;
//  }

//    err = CCLWT(crate_id, ADC_POSITION); //Waits for LAM Event on ADC slot
    while (!found_lam){
      err = CCCC(crate_id);
      if (err != CRATE_OK){
        printf("Error occuring with Crate Clear: %d\n", err);
        break; 
      }

      err = CTLM(crate_id, ADC_POSITION, &found_lam);
      if (err < 0){ 
        printf("error occurs waiting for LAM: %d\n", err);
        printf("cr_op.X = %d\n", cr_op.X);
        CRCLOSE(crate_id);
        crate_id = CROPEN("192.168.0.98");
        if (crate_id < 0){ 
          printf("error %d opening connection with CAMAC Controller\n", crate_id);
          return 1;
        }
        //i -= 1;
        continue;
      }
    }

    found_lam = 0;
    cr_op.F = 0; //Read Function
    err = CFSA(crate_id, &cr_op);
    if (err < 0) { 
      printf("error executing CSSA Operation: %d\n", err);
      CRCLOSE(crate_id);
      break; 
    }
    integrated_pulses[i] = cr_op.DATA;
    //if (i%100 == 0){
    printf("integrated_pulses[%d] = %d\n", i, integrated_pulses[i]); 
    //}

    cr_op.F = 10;
    err = CFSA(crate_id, &cr_op);
    if (err < 0){ 
      printf("error executing CFSA Operation: %d\n", err);
      break; 
    }

    err = CCCC(crate_id);
    if (err != CRATE_OK){
      printf("Error occuring with Crate Clear: %d\n", err);
      break; 
    }

//  err = CCCC(crate_id);
//  if (err != CRATE_OK){
//    printf("Error occuring with Crate Clear: %d\n", err);
//    break; 
//  }

  }
  saveData(data_file, config.num_pulses, integrated_pulses);
  fclose(data_file);
  
  //Close crate_id, which ends the connection with the C111C controller
  err = CRCLOSE(crate_id);
  if (err < 0){
    printf("error %d closing connection with CAMAC controller \n", crate_id);
  return 1;
  }
  
  return 0;
}
