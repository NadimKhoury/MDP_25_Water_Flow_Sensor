#include "C:\\Users\\Nadim\\Documents\\Arduino\\libraries\\Maqiatto_MQTT\\Credentials.h"
volatile int flow_frequency; // Measures flow sensor pulses
unsigned int l_hour; // Calculated litres/hour
unsigned char flowsensor = 2; // Sensor Input
unsigned long currentTime;
unsigned long cloopTime;
unsigned long cloopTime2;
String command; // variable to store the command
String message; // variable to store message to send to nodemcu for publishing
boolean newData = false; // is true when there is new data from Serial 
boolean newCommand = false; // is true when there is a new command from Serial3
const int voltageSensor = A0; //input of the voltage sensor 
float R1 = 30000.0;           //resistance used in the circuit (TO BE REVIEWED)
float R2 = 7500.0;            //resistance used in the circuit (TO BE REVIEWED)
int value = 0;                //measured value of voltage sensor on pin A0
int duration;
unsigned int valve;
int valve_pin=3;              // ATTENTION: this pin should receive open and close commands (to be reviewed).
                              //            Then, the status of this pin will be known below using "IF" statement to charge payload[12] of the valve's status.
const unsigned int len = 16;
byte payload[len];
String device_id = DEVICE_ID;
void flow () // Interrupt function
{
   flow_frequency++;
}
void setup()
{
  Serial.begin(9600);
  Serial3.begin(9600);
  while(!Serial3 && !Serial){}
  Serial.println("<Arduino is ready>");
  duration = 3000;
  payload[0]=1;
   pinMode(flowsensor, INPUT);
   pinMode(valve_pin, OUTPUT);
   pinMode(voltageSensor, INPUT);
   digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
   attachInterrupt(0, flow, RISING); // Setup Interrupt
   sei(); // Enable interrupts
   currentTime = millis();
   cloopTime = currentTime;
   cloopTime2 = currentTime;
   payload[15] = device_id.toInt();
}

void loop()
{
  currentTime = millis();
  recvWithStartEndMarkers();
  if(currentTime >= (cloopTime + 1000))
   {
      cloopTime = currentTime; // Updates cloopTime
      measureflow(payload);
   }
   if(currentTime >= (cloopTime2 + duration))
   {
    cloopTime2 = currentTime;
    Battery_Voltage(value, R1, R2, payload);
    Valve_Check(valve_pin, payload);
    PublishData(payload);
    Serial.print("[");
      Serial.print(payload[0]);
      for (int x = 1; x < 16; x++) { 
      Serial.print(",");
      Serial.print(payload[x]);
      }
      Serial.println("]");  
   }
   if(newCommand)
   {
    Serial.println(command);
    ExecuteCommand(payload, duration, valve_pin);
    command="";
    newCommand =false;
   }
}
/*CIRCUIT PART*/
void Battery_Voltage(int value,float R1,float R2, byte payload[]){
      float vOUT = 0.0;
      float vIN = 0.0;
      value = analogRead(voltageSensor);      //read voltage's signal
      vOUT = (value * 5.0) / 1024;            //output of voltage divider of the sensor
      vIN = vOUT / (R2/(R1+R2));              //input of the voltage sensor (which is the Battery)
      payload[14]=vIN;                        // change byte number 15 in the payload to battery status
  }
  
void Valve_Check(int pin,byte payload[]){
        valve=digitalRead(pin);           //LOW value means valve is closed(no water flow), HIGH means valve is open.
        if(valve==HIGH){                    
        payload[12]=1;
      }else{
        payload[12]=0;}      // load the 12th byte of the payload valve status "1" => Open, "0" => Closed.
  }

void measureflow(byte payload[]){
      static double consumption;
      static int index =0;
      double consumption_m3_and2digits;
      long consumption_to_convert;
      byte Measurement_0=0; 
      byte Measurement_1=0;
      byte Measurement_2=0;
      byte Measurement_3=0;
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
      flow_frequency = 0; // Reset Counter
      //Serial.print(l_hour, DEC); // Print litres/hour
      //Serial.println(" L/hour");
      consumption = consumption + l_hour*(1.0/3600);
      index++;
      //Serial.print(consumption,6);
      //Serial.println(" L");
      consumption_m3_and2digits=consumption/1000*100;
      //Serial.print(consumption_m3_and2digits,6);
      //Serial.println(" L/10");
      consumption_to_convert= (long)consumption_m3_and2digits;
      //Serial.println(consumption_to_convert,6);
      Measurement_0= consumption_to_convert & 255;
      consumption_to_convert=consumption_to_convert/256;
      Measurement_1= consumption_to_convert & 255;
      consumption_to_convert=consumption_to_convert/256;
      Measurement_2= consumption_to_convert & 255;
      consumption_to_convert=consumption_to_convert/256;
      Measurement_3 = consumption_to_convert& 255;
      payload[1]=Measurement_0;
      payload[2]=Measurement_1;
      payload[3]=Measurement_2;
      payload[3]=Measurement_3;
      
}

/*COMMUNICATION PART*/

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial3.available() > 0 && newCommand == false) {
        rc = Serial3.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                command.concat(rc);
            }
            else {
                recvInProgress = false;
                newCommand = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void PublishData(byte message[])
{
  Serial3.write("<");
  Serial3.print(convertarray(message));
  Serial3.write(">");
  Serial.println("sent");
}
/*Function to convert an array of byte to their equivalent in hexadecimal as a String*/
String convertarray(byte message[])
{
  String returned ="";
  String element;
  returned = convertbyte(message[0]);
  for(int i=1; i<len;i++)
  {
    element = convertbyte(message[i]);
    returned= returned+" "+ element;
  }
  return returned;
}

/*Function to convert one byte to equivalent hex number as a string*/
String convertbyte(byte onebyte)
{
  char onechar;
  String test ="";
  for(int k=0;k<2;k++)
  {
    onechar = onebyte & 15;
    if(onechar >= 10)
    {
      onechar+= 7;
    }
    onechar+=48;
    test=((char)onechar)+test;
    onebyte/=16;
  }
  return test;
}
void ExecuteCommand(byte payload[], int &duration, int &valve_pin)
{
  String commandNumber = command.substring(2,4);
  String exec = command.substring(4);
  Serial.println(commandNumber);
  Serial.println(exec);
  if(commandNumber.toInt() != payload[0])
  {
    Serial.println("wrong command number");
  }
  else
  {
    if(exec.equals("01C363"))
    {
      Serial.println("closing valve");
      digitalWrite(valve_pin,LOW);
      payload[0]++;  
    }
    else if(exec.equals("01C36F"))
    {
      Serial.println("opening valve");
      digitalWrite(valve_pin,HIGH);
      payload[0]++;    
    }
    else if(exec.equals("01D33C00"))
    {
      Serial.println("setting time to one hour");
      payload[0]++; 
      duration = 10000;   
    }
    else if(exec.equals("01D37800"))
    {
      Serial.println("setting time to two hour");
      duration = 10000*2;
      payload[0]++; 
    }
    else if(exec.equals("01D3F000"))
    {
      Serial.println("setting time to four hour");
      duration = 10000*4;
      payload[0]++; 
    }
    else
    {
      Serial.println("invalid command2");
    }
  }
}
