#ifndef LINUX
#define LINUX 1
#endif

#include "crate_lib.h" 
#include <time.h>
#include <stdio.h>

void IRQHandler(short crate_id, short irq_type, unsigned int irq_data) {
  switch (irq_type) {
    case LAM_INT: 
                 LACK(crate_id);
                  break;
    case COMBO_INT:
                  break;
    case DEFAULT_INT:
                  break;
  }
}

int main(int argc, char **argv) {
  time_t t = time(NULL);

  char s[30];
  struct tm tm = *localtime(&t);
  FILE *file;
  short crate_id;
  short err;
  int i,j;
  CRATE_OP cr_op;
  int num_pulses = atoi(argv[1]);
  unsigned short integrated_pulses[num_pulses];
  char file_name[100];
  unsigned int scan_result;
 
  //Initialize crate_id, which is necessary for all CAMAC functions
  crate_id = CROPEN("192.168.0.98");
  if (crate_id < 0) {
    printf("error %d opening connection with CAMAC Controller\n", crate_id);
    return 0;
  }
  else
    printf("Opened!\n");
 
  err = CRTOUT(crate_id, 10000);
  if (err < 0) {
    printf("Error occurs setting timeout: %d\n", err);
    return 1;
  }
  cr_op.N = 12; //Position in Crate is the ADC
  cr_op.A = 5; //Subaddress 

  err = CSCAN(crate_id, &scan_result);
  if (err != CRATE_OK){
    printf("Error occurs scanning the CRATE: %d\n", err);
    return 1;
  }

  err = CCCC(crate_id);
  if (err != CRATE_OK){
    printf("Error occuring with Dataway Init: %d\n", err);
    return 1;
  }
//for (i = 0; i < 24; i++) 
//  if (scan_result & (1 << i)) 
//    printf("The slot %d is filled with a card.\n", i+1);
//

  err = CCCZ(crate_id);
  if (err != CRATE_OK){
    printf("Error occuring with Dataway Init: %d\n", err);
    return 1;
  }

  //Change dataway inhibit to 0
  err = CCCI(crate_id, 0);
  if (err < 0){ 
    printf("error executing CCCI Operation: %d\n", err);
    return 1;
  }
  

  //                       
  // MAIN ACQUISITION LOOP
  //
  cr_op.F = 26; //Enable LAM 
  err = CFSA(crate_id, &cr_op);
  if (err < 0 || cr_op.X != 1){ 
    printf("error executing CFSA Operation: %d\n", err);
    return 1;
  }
  //printf("Enabling LAM DONE!\n");
  for (i = 0; i < num_pulses; i++){
   if (i % 1000 == 0)
     printf("%d pulses completed\n", i);
   
   err = CCLWT(crate_id, -1); //Waits for LAM Event on ADC slot
   if (err < 0 || cr_op.X != 1){ 
    printf("error occurs waiting for LAM: %d\n", err);
    return 0;
   }
//   printf("CCLWT DONE!\n");
   LACK(crate_id);
//  //handle interrupt
//  err = CRIRQ(crate_id, IRQHandler);
//  if (err < 0){ 
//    printf("error occurs registering callback: %d\n", err);
//    return 0;
//  }
//  else
//  printf("CRIRQ DONE!\n");

    cr_op.F = 0; //Read Function
    err = CFSA(crate_id, &cr_op);
    if (err < 0 || cr_op.X != 1 || cr_op.Q != 1){ 
      printf("error executing CFSA Operation: %d\n", err);
      return 0;
    }
  //printf("Reading Pulse %d DONE!\n", i);
  //printf("cr_op.data: %d\n", cr_op.DATA);
    integrated_pulses[i] = cr_op.DATA;

    cr_op.F = 9; //Clear LAM
    err = CFSA(crate_id, &cr_op);
    if (err < 0 || cr_op.X != 1){ 
      printf("error executing CFSA Operation: %d\n", err);
      return 0;
    }
    //printf("LAM Cleared!\n");
  }

  sprintf(file_name, "%d_%d_%d_%d_%d_data.dat", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min); 
  file = fopen(file_name, "w+b"); 
  for(i  = 0; i < num_pulses; i++) {
    sprintf(s, "%u\n", integrated_pulses[i]);
    fwrite(s, 1, strlen(s), file);
  }

  //Close crate_id, which ends the connection with the C111C controller
  err = CRCLOSE(crate_id);
  if (err < 0 || cr_op.X != 1) 
    printf("error %d closing connection with CAMAC controller \n", crate_id);
  return 1;
}
