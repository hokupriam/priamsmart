#include <stdint.h>


#include "src/PriamSmartInterface.h"
#include "src/PriamDrive.h"

using namespace Priam;

PriamSmart smartInterface;
PriamDrive priamDrive(smartInterface);

#define PINKLED 19

void setup() {
  Serial.begin(115200);
  Serial.print(F("Priam Smart Interface routine\n"));

  if (!smartInterface.Open(false))
    Serial.print(F("Interface class open error!\n"));
  else
    Serial.println(F("Interface class is open, no reset issued"));


  digitalWrite(PINKLED, HIGH);   // turn the LED off
  pinMode(PINKLED, OUTPUT); //19 A5 led on shield

}

//Command execution functions
void Spinup()
{
  Serial.println(F("Spin up and wait for completion, can take upto 30 seconds"));
  
  TransactionStatus spinupStatus = priamDrive.SpinupWait(0);
  
  if (spinupStatus.CommsError())
  {
    Serial.println(F("Spinup command error. comms failure"));
  }
  else
  {
    Serial.print(F("Spinup command finished with status: Drive "));
    Serial.print(spinupStatus.Drive());
    Serial.print(F(", Completion type:  "));
    Serial.print(spinupStatus.CompType());
    Serial.print(F(", Completion code:  0x"));
    Serial.println(spinupStatus.Code(), HEX);
  }

}

void Spindown()
{
  Serial.println(F("Spin down"));
  
  TransactionStatus spindownStatus = priamDrive.SpinDown(0);
  
  if (spindownStatus.CommsError())
  {
    Serial.println(F("Spin down command error. comms failure"));
  }
  else
  {
    Serial.print(F("Spin down command finished with status: Drive "));
    Serial.print(spindownStatus.Drive());
    Serial.print(F(", Completion type:  "));
    Serial.print(spindownStatus.CompType());
    Serial.print(F(", Completion code:  0x"));
    Serial.println(spindownStatus.Code(), HEX);
  }

}

void ReadDriveParams()
{
  Serial.println(F("Read drive paramaters"));
  
  ResultDriveParams parmStatus = priamDrive.ReadParams(0);
  
  if (parmStatus.GetStatus().CommsError())
  {
    Serial.println(F("Read parameters error, comms failure"));
  }
  else
  {
    Serial.print(F("Read parameters command finished with status: Drive "));
    Serial.print(parmStatus.GetStatus().Drive());
    Serial.print(F(", Completion type:  "));
    Serial.print(parmStatus.GetStatus().CompType());
    Serial.print(F(", Completion code:  0x"));
    Serial.println(parmStatus.GetStatus().Code(), HEX);

    Serial.print(F("Drive parameters: "));
    Serial.print(parmStatus.Heads());
    Serial.print(F(" heads, "));
    Serial.print(parmStatus.Cylinders());
    Serial.print(F(" cylinders, "));
    Serial.print(parmStatus.SectorsPerTrack());
    Serial.print(F(" sectors/track, "));
    Serial.print(parmStatus.LogicalSectorSize());
    Serial.println(F(" logical sector size"));

  }


}

bool Seek(uint8_t head, uint16_t cylinder)
{
  bool retval = false;

  Serial.print(F("Seek head "));
  Serial.print(head);
  Serial.print(F(" cylinder "));
  Serial.println(cylinder);
  
  ResultCylinder resCyl = priamDrive.Seek(0, head, cylinder);
  
  if (resCyl.GetStatus().CommsError())
  {
    Serial.println(F("Seek error, comms failure"));
    retval = false;
  }
  else
  {
    Serial.print(F("Seek command finished with status: Drive "));
    Serial.print(resCyl.GetStatus().Drive());
    Serial.print(F(", Completion type:  "));
    Serial.print(resCyl.GetStatus().CompType());
    Serial.print(F(", Completion code:  0x"));
    Serial.println(resCyl.GetStatus().Code(), HEX);

    Serial.print(F("Drive is now at cylinder: "));
    Serial.print(resCyl.Cylinder());
    
    retval = !resCyl.GetStatus().IsErrorStatus();

  }

  return retval;
}

bool SeekFirstCylinder()
{
  Serial.println(F("Seek to first cylinder"));


  return Seek(0, 0);
}

bool SeekLastCylinder()
{
  Serial.println(F("Seek to last cylinder"));

  ResultDriveParams parmStatus = priamDrive.ReadParams(0);
  
  if (parmStatus.GetStatus().CommsError() || parmStatus.GetStatus().IsErrorStatus())
  {
    Serial.println(F("Error getting drive parameters"));
    return false;
  }

  return Seek(0, parmStatus.Cylinders() - 1);
}

void VerifyDisk()
{
  Serial.println(F("Verify Disk"));
  
  ResultHeadCylinderSector vrfStatus = priamDrive.VerifyDisk(0);
  
  if (vrfStatus.GetStatus().CommsError())
  {
    Serial.println(F("Verify disk command error. comms failure"));
  }
  else
  {
    Serial.print(F("Verify command finished with status: Drive "));
    Serial.print(vrfStatus.GetStatus().Drive());
    Serial.print(F(", Completion type:  "));
    Serial.print(vrfStatus.GetStatus().CompType());
    Serial.print(F(", Completion code:  0x"));
    Serial.println(vrfStatus.GetStatus().Code(), HEX);

    if (!vrfStatus.GetStatus().IsErrorStatus())
    {
      Serial.println(F("Verify disk: disk is good!"));
    }
    else
    {
      Serial.print(F("Verify disk: error found at head "));
      Serial.print(vrfStatus.Head());
      Serial.print(F(" cylinder "));
      Serial.print(vrfStatus.Cylinder());
      Serial.print(F(" sector "));
      Serial.println(vrfStatus.Sector());
    }
  }

}

void ReadData(uint8_t driveno, uint8_t head, uint16_t cylinder, uint8_t sector, uint8_t numsector, bool print = true)
{

  if (print)
  {
    Serial.print(F("Read "));
    Serial.print(numsector);
    Serial.print(F(" sectors starting at head "));
    Serial.print(head);
    Serial.print(F(" cylinder  "));
    Serial.print(cylinder);
    Serial.print(F(" sector  "));
    Serial.println(sector);
  }

  TransactionStatus parmStatus = priamDrive.ReadData(driveno, head, cylinder, sector, numsector);
  
  if (parmStatus.CommsError())
  {
    Serial.println(F("Read data command error. comms failure"));
  }
  else
  {
    if (print)
    {    
      Serial.print(F("Verify command finished with status: Drive "));
      Serial.print(parmStatus.Drive());
      Serial.print(F(", Completion type:  "));
      Serial.print(parmStatus.CompType());
      Serial.print(F(", Completion code:  0x"));
      Serial.println(parmStatus.Code(), HEX);
    }
    
    if (!parmStatus.IsErrorStatus())
    {
      if (print)
      {
        Serial.println(F("Read data: read successful!"));
      }
    }
    else
    {
      Serial.println(F("Read data: error"));
      Serial.print(F(", Completion type:  "));
      Serial.print(parmStatus.CompType());
      Serial.print(F(", Completion code:  0x"));
      Serial.println(parmStatus.Code(), HEX);
      
    }
  }
}

  


void ReadAllSectors()
{
  Serial.println(F("Read all sectors"));

  ResultDriveParams parmStatus = priamDrive.ReadParams(0);
  
  if (parmStatus.GetStatus().CommsError() || parmStatus.GetStatus().IsErrorStatus())
  {
    Serial.println(F("Error getting drive parameters"));
    return;
  }

  for (uint8_t cyl = 0; cyl < parmStatus.Cylinders(); cyl++)
  {
    for (uint8_t head = 0; head < parmStatus.Heads(); head++)
    {
      for (uint8_t sector = 0; sector < parmStatus.SectorsPerTrack(); sector++)
      {
        ReadData(0, head, cyl, sector, 1, false);
      }
    }
  }

}

// the loop function runs over and over again forever
void loop() {
  static bool startupDone = false;

  if (!startupDone)
  {
    smartInterface.WaitForDriveReady(1000, 10);
  
    Serial.println(F("Drive is ready for commands"));
    startupDone = true;
    digitalWrite(PINKLED, LOW); //turn LED on
  }
  
  //Get rid of anything in serial buffer
  while (Serial.available())
    Serial.read();

  Serial.print("\n");

  Serial.println(F("1) Spin up drive"));
  Serial.println(F("2) Spin down drive"));
  Serial.println(F("3) Read drive parameters"));
  Serial.println(F("4) Seek to first cylinder"));
  Serial.println(F("5) Seek to last cylinder"));
  Serial.println(F("6) Verify Disk"));
  Serial.println(F("7) Read 5 sectors from h:0 c:0 s:0"));
  Serial.println(F("8) Dump all sectors"));
  Serial.print(F("Your choice>"));

  //Wait for key
  while (!Serial.available()) ;
  
  char incoming = Serial.read();

  Serial.print(incoming);
  Serial.print("\n\n");
  

  switch (incoming)
  {
    case '1':
      Spinup();
      break;
    case '2':
      Spindown();
      break;
    case '3':
      ReadDriveParams();
      break;
    case '4':
      SeekFirstCylinder();
      break;
    case '5':
      SeekLastCylinder();
      break;
    case '6':
      VerifyDisk();
      break;
    case '7':
      ReadData(0, 0, 0, 0, 5);
      break;
    case '8':
      ReadAllSectors();
      break;
    default:
      Serial.println(F("Invalid selection"));
  }

  
  
}
