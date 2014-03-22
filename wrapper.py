import sys
import os
import time 
import ConfigParser #Necessary for config file parsing
from subprocess import call #Used to call DAQ program 
from commands import getstatusoutput #Used to get output of terminal

"""
Purposes of this module: 
  Calls DAQ and Production Module for standard runs.
    1. Parse necessary configuration files
    2. Interactively ask user which PMT serials are being used and other
       hardware related questions for changing some configuration settings.
    3. Save new configuration file in /daq/tmp
    4. Call DAQ script with correct configuration file, run number, and tag.
       Run Number comes from number of files in expected output directory, 
       while tag must be determined by user options. Maybe set two
       possibilities: qa (which handles spe and gain together) and surf
"""

DAQ_BIN = "/home/brandon/coding/owl_daq/bin/camac"
VOLT_UP_BIN = "/home/brandon/coding/vme_daq/bin/volt_up"
VOLT_DOWN_BIN = "/home/brandon/coding/vme_daq/bin/volt_down"
WAIT_TIME = 0 #Wait time in minutes for PMTs/Lamp to heat up
TMP_DIR = "/daq/tmp"
MASTER_CONFIG_LOC = "/daq/conf/camac"

def getConfig(conf_fn): 
  """Returns a ConfigParser object containing data parsed from conf_fn """
  parser = ConfigParser.SafeConfigParser()
  parser.readfp(open(str(conf_fn)))
  return parser

def getRunNumber(mode):
  """Checks folder based on mode to determine number of files within, which is  
     the desired run_number"""
  mode = mode.lower()
  status = 0
  output = ""
  if(mode == 'spe'):
    status, output = getstatusoutput("ls /daq/prod | grep spe | wc -l")
    output = int(output) 
  elif(mode == 'gain'):
    status, output = getstatusoutput("ls /daq/prod | grep gain | wc -l")
    output = int(output) 
  elif(mode == 'surf'):
    status, output = getstatusoutput("ls /daq/prod | grep surf | wc -l")
    output = int(output)
  elif(mode == 'misc'):
    status, output = getstatusoutput("ls /daq/misc | grep misc | wc -l")
    output = int(output)
  else:
    print "Invalid mode {} for getting run number".format(mode)
    sys.exit(1)
  return output

if __name__ == '__main__':
  usage = "wrapper.py [RUN_MODE (run, surf, misc)] [TAG (pre,post,,etc)] [INTEGRATE, 0 (false) or 1 (true)]"

  if (len(sys.argv) < 3):
    print usage
  #Mode corresponds to the name of the schema in the database: "run" is for QA
  #testing (i.e. gain or spe), "surf" is for surface consistency testing, and
  #misc is for everything else, while tag is the suffix on the created file name
  #for indexing purposes. Some possible tags are pre and post for all run modes, 
  #and any miscellaneous tags necessary for descriptiveness.
  integrate = 0
  mode = str(sys.argv[1]).lower()
  tag = str(sys.argv[2])
  run_number = getRunNumber(mode)
  pmt_serial = "" 
  cfg_parser = 0
  tmp_files = []
  pmt_voltage = 0 
  hv_channel = -1 
  #Open Master config files based on mode
  if (mode == 'gain'):
    cfg_parser = getConfig("{}/gain.conf".format(MASTER_CONFIG_LOC))
    tmp_files.append(open("{}/gain.conf".format(TMP_DIR), 'w'))
  elif (mode == 'spe'):
    cfg_parser = getConfig("{}/spe.conf".format(MASTER_CONFIG_LOC))
    tmp_files.append(open("{}/spe.conf".format(TMP_DIR), 'w'))
  elif (mode == 'surf'):
    cfg_parser = getConfig("{}/surf.conf".format(MASTER_CONFIG_LOC))
    tmp_files.append(open("{}/surf_ang000.conf".format(TMP_DIR), 'w'))
    tmp_files.append(open("{}/surf_ang090.conf".format(TMP_DIR), 'w'))
    tmp_files.append(open("{}/surf_ang180.conf".format(TMP_DIR), 'w'))
    tmp_files.append(open("{}/surf_ang270.conf".format(TMP_DIR), 'w'))
  elif (mode == 'misc'):
    cfg_parser = getConfig("{}/misc.conf".format(MASTER_CONFIG_LOC))
    tmp_files.append(open("{}/misc.conf".format(TMP_DIR), 'w'))
  else:
    print "Invalid mode: {}".format(mode)
    sys.exit(1)

  output_folder = cfg_parser.get('DAQ', 'output-folder')
  subaddress = cfg_parser.get('Hardware', 'subaddress')

  try:
    if not os.path.isdir("{0}/{1}_{2:05d}".format(output_folder, mode, run_number)):
      os.makedirs("{0}/{1}_{2:05d}".format(output_folder, mode, run_number))
  except OSError as exception:
    if exception.errno != errno.EEXIST:
      raise

  pmt_serial = cfg_parser.get('Hardware', 'pmt-id')
  pmt_voltage = cfg_parser.get('Hardware', 'pmt-voltage')
  num_pulses = int(cfg_parser.get('DAQ', 'num-pulses'))
  #Get Hardware Info for each Run


  print "Current expected number of pulses is {}.".format(num_pulses)
  print "If correct, press enter. Otherwise, enter correct number of pulses."
  input = raw_input('-->')
  if (input != ''):
    num_pulses = int(input)
  
  print "Current PMT Serial for Subaddress {} is {}.".format(subaddress, pmt_serial)
  print "If correct, press enter. Otherwise, enter correct serial."
  input = raw_input('-->')
  if (input != ''):
    pmt_serial = input
  input = raw_input("Enter voltage for PMT in Subaddress {} of ADC (default: {})(Sign matters!)".format(subaddress, pmt_voltage))

  if (input != ''):
    pmt_voltage = int(input)
  
# if (pmt_voltage > 0):
#   hv_channel = 3 
# elif(pmt_voltage < 0):
#   hv_channel = 0
  

# input = raw_input("Attach PMT high voltage to HV Channel {} in V6521M.".format(hv_channel))
  ####
  #Add more hardware settings here later
  ####

  
  #Set hardware values in config files
  cfg_parser.set('Hardware', 'pmt-voltage', str(pmt_voltage))
  cfg_parser.set('Hardware', 'pmt-id', str(pmt_serial))
  cfg_parser.set('DAQ', 'num-pulses', str(num_pulses))
  cfg_parser.set('DAQ', 'mode', str(mode))
  cfg_parser.set('DAQ', 'tag', str(tag))
  cfg_parser.set('DAQ', 'run-num', str(run_number))

  #Save config files to temp folder for temporary storage before running
  for i in range(len(tmp_files)):
    cfg_parser.write(tmp_files[i]) 
    tmp_files[i].close()

# input = raw_input("Turn on voltage? [y/n].")
# if (input.lower() == 'y'):

#   print "Turning on voltage source..."
#   if (hv_channel == 3):
#     os.system("{} 0 0 0 {} 0 0".format(VOLT_UP_BIN, abs(pmt_voltage))) 
#   elif (hv_channel == 0):
#     os.system("{} {} 0 0 0 0 0".format(VOLT_UP_BIN, abs(pmt_voltage))) 
#   else:
#     print "Warning! Not turning on voltage!"      

  print 'Now wait for {} minutes for heating of lamp and PMT'.format(WAIT_TIME)
  for i in xrange(WAIT_TIME, 0, -1):
    time.sleep(1000)
    sys.stdout.write(str(i)+' ')
    sys.stdout.flush()

  #Make folder to store final data
  i = 0
  for fn in os.listdir("{}".format(TMP_DIR)):
    print "Current filename: {}\n".format(fn)
    tmp_tag = tag
    if (mode == 'surf'):
      raw_input("Press enter when PMT is prepared at {} degrees".format(i*90))
      tmp_tag = "{0}_ang{1:03d}".format(tag, i*90) 
      i = i+1
    elif(fn == "spe.conf"):
      raw_input("Press enter when PMT is prepared for s.p.e. testing")
    elif(fn == "gain.conf"):
      raw_input("Press enter when PMT is prepared for gain testing")
    print "{} {}/{}".format(DAQ_BIN, TMP_DIR, fn) 
    os.system("{} {}/{}".format(DAQ_BIN, TMP_DIR, fn)) 
    #Remove tmp file after run
    os.system("rm {}/{}".format(TMP_DIR, fn)) 

#  input = raw_input("Runs completed. Would you like to turn off voltage? [y/n]")
#  if (input.lower() == 'y'): 
#    os.system("{}".format(VOLT_DOWN_BIN))
