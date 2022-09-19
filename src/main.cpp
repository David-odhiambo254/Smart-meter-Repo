#include <Arduino.h>
#include <SoftwareSerial.h> // Library for serial communication

#define buttonRest  7  //The reset pin 
#define valve 5  //Relay pin for the valve
//----------------------------------------------------------------------------------------------------------------
volatile int flow_frequecy;
// calculate litres/hour
float vol = 0.0,l_minutes;
unsigned char flowsensor = 2; //Sensor Input pin
unsigned long currentTime;
unsigned long cloopTime; 

void flow()
{
  flow_frequecy++;
}
//----------------------------------------------------------------------------------------------------------------

bool button_state;
String Data_sms;
//Add relevant variables for the water reader here
SoftwareSerial sim800l(2,3); //RX, TX for arduino and the module
float volLimit = 1; // User's subsribed amount once payment is made. /Change accordingly


void setup() {
  //-------------------------------------------------------------------------------------------------------------
  pinMode(flowsensor,INPUT);
  digitalWrite(flowsensor,HIGH); // optional internal pull-up
  attachInterrupt(digitalPinToInterrupt(flowsensor),flow,RISING); // setup interrupt
  currentTime = millis();
  cloopTime = currentTime;
  //-------------------------------------------------------------------------------------------------------------

  pinMode(buttonRest,INPUT_PULLUP);
  pinMode(valve,OUTPUT); // Setting relay pin as output
  sim800l.begin(9600);
  Serial.begin(9600);
//Start water reader here /Done
  delay(1000);
}

void Send_Confirmation()
{
  Serial.print("Sending confrimation");
  sim800l.print("AT+CMGF=1\r"); //setting shield to sms mode
  delay(100);
  sim800l.print("AT+CMGS=\"+254773767104\"\r"); // Replace *** with phone number
  delay(500);
  Data_sms = "confirmed Payment";

  sim800l.print(Data_sms);
  delay(500);
  sim800l.print((char)26);
  delay(500);
  sim800l.println();
  Serial.print("Data sent");
  delay(500);
}
void Send_Alert()
{
  Serial.print("Sending alert");
  sim800l.print("AT+CMGF=1\r"); //setting shield to sms mode
  delay(100);
  sim800l.print("AT+CMGS=\"+254773767104\"\r"); // Replace *** with phone number
  delay(500);
  Data_sms = "Token ran out";

  sim800l.print(Data_sms);
  delay(500);
  sim800l.print((char)26);
  delay(500);
  sim800l.println();
  Serial.print("Data sent");
  delay(500);
}
void Serialcom()
{
  delay(500);
  while (Serial.available())
  {
    sim800l.write(Serial.read());
  }
  while (sim800l.available())
  {
    Serial.write(sim800l.read());
  }
  
  
}
void loop() {
  //----------------------------------------------------------------------------------------------------------------
  currentTime = millis();
  // Every second, calculate and print litres/hour
  if (flow_frequecy != 0)
  {
    // Pulse frequency (Hz) 7.5Q, Q is flow rate in L/min
    l_minutes = (flow_frequecy / 7.5); // (Pulse frequency x 60 min)/7.5 = flowrate in L/hour
    
    l_minutes = l_minutes/60;
    vol = vol+ l_minutes; // Saving volume in litres
    flow_frequecy = 0;
    Serial.print(l_minutes,DEC);
    Serial.print("L/sec");
  }
  else
  {
    Serial.print("Flow rate = 0");
    Serial.print("Volume");Serial.print(vol);
  }
  
  //-----------------------------------------------BUSINESS LOGICS-----------------------------------------------------------------
  Serialcom();
  

  button_state = digitalRead(buttonRest);
  if(button_state == LOW)
  {
    Serial.print("Reset button pressed");
    Serial.print("Resetting values...");
    //Reset the variables and send confirmation message for payment
    digitalWrite(valve,LOW);
    vol = 0.00;
    Send_Confirmation();
  }

  if (vol >= volLimit) // If volume exceeds volume bought:
  {
    Serial.print("Client credits depleated");
    Serial.print("Disconnecting...");
    digitalWrite(valve,HIGH); // Shut the water valve
    Send_Alert(); // Alert customer to recharge
  }
  else{
    digitalWrite(valve,LOW);
  }
  delay(3000);
  
}