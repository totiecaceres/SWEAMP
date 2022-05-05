
// SENSORS
int gwdSensorA = A0;
int gwdSensorB = A1;
int pirSensor = A2;
int tempSensor = A3;
int proximSensor = A4;


// RELAYS (Active Low)
int gwdAlarmA = 2;
int gwdAlarmB = 3;
int intruderAlarm = 4;
int doorAlarm = 5;
int tempAlarm = 6;
int hightempfan = 7;
int acu_1 = 12;
int acu_2 = 13;
int acu_faulty_alarm = 11;
int sweamp_alarm = 10;

//For Temperature convertion
float c1= 1.009249522e-03, c2=2.37840544e-04,c3=2.019202697e-07;
float R1=1000, R2,logR2;
double T,Tc ;

//For Temperature sensor time delay 
int tmp_count = 0;
int tmp_trig_count = 10;
int tmp_trig_reset_count = 5;  //15 sec
int tmp_loop_count = 0;

//For ACU time delay 
int acu_count = 0;
int acu_loop_count = 0;

//For ACU faulty variation count
int acu_faulty_count = 0;
int index_count = 0;

//For SWEAMP alarm
int sweamp = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  
  pinMode(intruderAlarm, OUTPUT);
  pinMode(doorAlarm, OUTPUT);    
  pinMode(tempAlarm, OUTPUT);
  pinMode(gwdAlarmA, OUTPUT);
  pinMode(gwdAlarmB, OUTPUT);
  pinMode(hightempfan, OUTPUT);
  pinMode(acu_1, OUTPUT);
  pinMode(acu_2, OUTPUT);
  pinMode(acu_faulty_alarm, OUTPUT);
  pinMode(sweamp_alarm, OUTPUT);
  pinMode(pirSensor, INPUT);
  pinMode(proximSensor, INPUT);
  pinMode(tempSensor, INPUT);
  pinMode(gwdSensorA, INPUT);
  pinMode(gwdSensorB, INPUT);  
}

void loop() {
  // Sensor variables
 int pirValue =  digitalRead(pirSensor);
 int proximValue = analogRead(proximSensor);
 int tempValue = analogRead(tempSensor);
 int gwdValueA = analogRead(gwdSensorA);
 int gwdValueB = analogRead(gwdSensorB);

// SWEAMP Alarm
  if (sweamp_alarm == 1){
    digitalWrite(acu_faulty_alarm, LOW);
  }
   
// Temperature computaion
  R2 = R1 * (1023.0 / (float)tempValue - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  Tc = T - 273.15;
  
//Condition for Motion sensor

  if (pirValue == 1){
    digitalWrite(intruderAlarm, LOW);
    Serial.print("Motion detected \t");
    Serial.println();
  }
  else{
    digitalWrite(intruderAlarm, HIGH);
  }

//Condition for Temperature sensor

  if ((Tc >= 35) && (Tc < 100)){
    index_count = 1;
    digitalWrite(acu_1, LOW);
    digitalWrite(acu_2, LOW);
    tmp_count = tmp_count + 1 - tmp_loop_count;
    tmp_trig_reset_count = 15; // 15 sec 
    if (tmp_count >= 30){
    Serial.print("High Temp\t");
    Serial.println();
    digitalWrite(tempAlarm, LOW);
    digitalWrite(hightempfan, LOW);
    Serial.print(" Fan: ");
    Serial.print("activated\t");
    Serial.println();
    }
    else{
      digitalWrite(tempAlarm, HIGH);
      digitalWrite(hightempfan, HIGH);
    }
    if (tmp_count <= 60){
      tmp_loop_count = 0;
    }
    else {
      tmp_loop_count = 30; //30 sec
      digitalWrite(tempAlarm, HIGH);
      digitalWrite(hightempfan, HIGH);
    }
      Serial.print("temp: ");
      Serial.print(Tc);
      Serial.println();
  }
  
  else if ((Tc < 35) && (Tc > 0)){
     
    if (tmp_trig_reset_count!=0){
      tmp_trig_reset_count = tmp_trig_reset_count - 1;
      digitalWrite(tempAlarm, HIGH);
      digitalWrite(hightempfan, HIGH);
    }
    else{
      tmp_count = 0;
      tmp_trig_reset_count = 15;
    }
      Serial.print("temp: ");
      Serial.print(Tc);
      Serial.println();

    // ACU Time Delay
    if (Tc <= 30) {
      acu_count = acu_count + 1 - acu_loop_count;
      index_count = 0;
      if (acu_count <= 1800){ // acu 2 ON w/ in 1 - 30 min
      digitalWrite(acu_1, HIGH);
      digitalWrite(acu_2, LOW);
      Serial.print("ACU 1");
      Serial.print("\t");
      }
      else if (acu_count > 1802) { //acu 1 ON w/ in 31 - 60 min
      digitalWrite(acu_1, LOW);
      digitalWrite(acu_2, HIGH);
      Serial.print("ACU 2");
      Serial.print("\t");
      }
      // 60 min limit acu count. acu_count reset to 0 when acu count > 60
      if (acu_count <= 3599){ 
        acu_loop_count = 0;
      }
      else {
        acu_loop_count = 3600;
      }
      Serial.print(acu_count);
      Serial.print(" sec");
      Serial.println();
    }
    
    else {
      acu_count = 0;
      acu_loop_count = 0;
      index_count = 1;
      digitalWrite(acu_1, LOW);
      digitalWrite(acu_2, LOW);
    }
  }

  else {
      acu_faulty_count = 0;
      index_count = 0;
      digitalWrite(acu_1, LOW);
      digitalWrite(acu_2, LOW);
      digitalWrite(tempAlarm, HIGH);
      digitalWrite(hightempfan, HIGH);
      Serial.print("Thermistor not working: ");
      Serial.print(Tc);
      Serial.println();
  }

//ACU Faulty Alarm Monitoring
 
  acu_faulty_count = acu_faulty_count + index_count;
  if (acu_faulty_count >= 4){
    digitalWrite(acu_faulty_alarm,LOW);
  }
  else{
    digitalWrite(acu_faulty_alarm,HIGH);
  }
  
// Condition for door alarm

  if (proximValue <= 10){
  digitalWrite(doorAlarm, LOW);
  Serial.print(" Proximity Sensor: ");
  Serial.print(" Door is Open\t");
  Serial.println();
  }
  else {
    digitalWrite(doorAlarm, HIGH);
  }

// Condition for ground thief alarm
  if (gwdValueA == 0){
  digitalWrite(gwdAlarmA, HIGH);
  }
  else {
  digitalWrite(gwdAlarmA, LOW);
  Serial.print("Gwd1 Check: ");
  Serial.print(gwdValueA);
  Serial.print(" disconnected\t");
  Serial.println();
  }
  if (gwdValueB == 0){
  digitalWrite(gwdAlarmB, HIGH);
  }
  else {
  digitalWrite(gwdAlarmB, LOW);
  Serial.print("Gwd2 Check: ");
  Serial.print(gwdValueB);
  Serial.print(" disconnected\t");
  Serial.println();
  }

  delay(1000);
}
