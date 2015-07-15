#define trigPin1 9 //sonar1
#define echoPin1 8
#define trigPin2 11  //sonar2
#define echoPin2 10
#define startButton 5
#define switchGateButton 7

int incomingByte = 0;   // for incoming serial data
int trigPin = 13;
int echoPin = 12;
void setup() {
  Serial.begin (115200);
  Serial.read();
  pinMode(trigPin1, OUTPUT);  //sonar1
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);  //sonar2
  pinMode(echoPin2, INPUT);
  
  //buttons
  pinMode(startButton, INPUT_PULLUP);
  pinMode(switchGateButton, INPUT_PULLUP);
}
String str;
String distance0 = "-1";
String distance1 = "-1";
String gate = "0";
String strt = "0";
void loop() {
     //send data only when you receive data:
   Serial.println(
   String("<id:5>")+
    "<start:"+strt+">"+
    "<gate:"+gate+">"+
    "<s1:"+distance0+">"+
    "<s2:"+distance1+">"
  );
  distance0 = getDistance(0);
  distance1 = getDistance(1);
  if(digitalRead(switchGateButton) == LOW){
    gate = "1";
  }
  else{
    gate = "0";
  }

  if(digitalRead(startButton) == LOW){
    strt = "1";
  }
  else{
    strt = "0";
  }  
}

String getDistance(int sonarId){
  if(sonarId == 0){
    trigPin = trigPin1;
    echoPin = echoPin1;
  }
  else if(sonarId == 1){
    trigPin = trigPin2;
    echoPin = echoPin2;
  }
  String result;
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, 2000);
  distance = (duration/2) / 29.1;
  
  if (distance >= 200 || distance <= 0){
    result = String(-1);
  }
  else {
    result = String(distance);
  }
  
  return result;
}




