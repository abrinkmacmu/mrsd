/* Pin Definitions **********************************************/
const int pin_B0 = 2;
const int pin_B1 = 3;
const int pin_Pot = A0;
const int pin_Red = 9;
const int pin_Green = 10;
const int pin_Blue = 11;

/* Global Vars ****************************************************/
const unsigned long B0_debounce = 5; //ms
const unsigned long B1_debounce = 5; //ms

bool last_B0_value = LOW;
unsigned long MODE = 0;
volatile unsigned long last_time_B0 = 0;
volatile bool B0_state_change  = 0;

bool last_B1_value = LOW;
bool B1_pressed = LOW;
volatile unsigned long last_time_B1 = 0;
volatile bool B1_state_change = 0;

bool MODE_0_Flag = LOW;
volatile int random_red, random_green, random_blue;


void setup() {
  pinMode(pin_B0, INPUT);
  pinMode(pin_B1, INPUT);
  pinMode(pin_Red, OUTPUT);
  pinMode(pin_Green, OUTPUT);
  pinMode(pin_Blue, OUTPUT);
  
  last_B0_value = digitalRead(pin_B0);
  last_B1_value = digitalRead(pin_B1);
  
  attachInterrupt(0, Button0_Callback, CHANGE);
  attachInterrupt(1, Button1_Callback, CHANGE); 
  
  Serial.begin(9600);
  Serial.println("TeamD_abrinkma_Task2.ino");
  Serial.println("Written by Alex Brinkman, 20150912\n");
  Serial.println( "In Mode 0, Press Button 1 to Toggle to Random Color");
}

void loop() { 
  
  /* handle B0 interrupt and MODE value */
  
  if(B0_state_change){
    if(last_B0_value == LOW){
      MODE = MODE + 1;
      switch(MODE %3){
        case 0:
          Serial.println( "In Mode 0, Press Button 1 to Toggle to Random Color");
          break;
        case 1:
          Serial.println( "In Mode 1, Turn Potentiometer to adjust While light Brightness");
          break;
          
        case 2:
          Serial.println( "In Mode 2, Type single or multiple commands for rgb values");
          Serial.println(" may omit color but order must be r->g->b ");  
          break;
      }
    }      
    B0_state_change = 0;
  }else if( ( millis() - last_time_B0) > B0_debounce) {
    last_B0_value = digitalRead(pin_B0);
  }
  
  
  /* handle B1 interrupt and button event */
  
  if(B1_state_change){
    if(last_B1_value == LOW){
      B1_pressed = HIGH;
     // Serial.println( "Button 1 pressed");
    }      
    B1_state_change = 0;
  }else if( ( millis() - last_time_B1) > B1_debounce) {
    last_B1_value = digitalRead(pin_B1);
  }
  
  /* MODE and B1 Pressed Logic */
  
    switch (MODE % 3){
      case 0:
        MODE_0();
        break;
      
      case 1:
        MODE_1();
        break;
      
      case 2:
        MODE_2();
        break;
      
      default:
        Serial.print ("Entered unknown state");
        break;
    }
    B1_pressed = LOW;
  delay(5);
}

/* ISR **************************************************************************/

void Button0_Callback() {
  unsigned long this_time_B0 = millis();
  if( this_time_B0 - last_time_B0 > B0_debounce){ // rollover NOT handled since unsigned longs used
    B0_state_change = 1;
    last_time_B0 = this_time_B0;
  }  
}

void Button1_Callback() {
  unsigned long this_time_B1 = millis();
  if( this_time_B1 - last_time_B1 > B1_debounce){ // rollover NOT handled since unsigned longs used
    B1_state_change = 1;
    last_time_B1 = this_time_B1;
  }  
}


/* Functions *******************************************************/

int myRandom(){
  int rand = random(0,100);
    if(rand > 50){
      return 255;
    }else{
      return 0;
    }
}
void LED_ctrl(const int R, const int G, const int B){
  /* input ints are between 0 (0v) and 255 (5v) */
  
  analogWrite(pin_Red, R);
  analogWrite(pin_Green, G);
  analogWrite(pin_Blue, B);
  
}
  
void MODE_0(){
  if(B1_pressed){
    MODE_0_Flag ^= 1;
    random_red = myRandom();
    random_green = myRandom();
    random_blue = myRandom();
    if (random_red + random_green + random_blue > 700){
      random_red = 0;
    }
  }
  
  if(MODE_0_Flag){
    
    LED_ctrl(random_red, random_green, random_blue);
  }else{
    LED_ctrl(255,255,255);
  }
    
}

void MODE_1(){
  int intensity = map(analogRead(pin_Pot), 0, 1023, 0, 255);
  LED_ctrl(intensity, intensity, intensity);
}

void MODE_2(){
  
  if(Serial.available() > 0){
    String msg = Serial.readString();
    Serial.print("Echo: ");
    Serial.println(msg);
    decode_serial_cmd(msg);
  }
  
}

void decode_serial_cmd(const String msg){
  int r_index = msg.indexOf('r');
  int g_index = msg.indexOf('g');
  int b_index = msg.indexOf('b');
  int r_value, g_value, b_value;
  int error = 0;
  
  
  if (r_index + g_index + b_index == -3){
    Serial.println( "invalid Command, try again");
  }else{
  
    if(r_index != -1){
      String r_value_string;
      if(g_index > r_index){
        r_value_string = msg.substring(r_index+1, g_index-1);
        
      }else if(b_index > r_index){
        r_value_string = msg.substring(r_index+1, b_index-1);
      
      }else{
        r_value_string = msg.substring(r_index+1);
        
      }
      for(int i = 0; i< r_value_string.length(); i++){
        if( 0 == isDigit( r_value_string.charAt(i) )){
          error = 1;
        }
      }     
             
      r_value = r_value_string.toInt();  
    }else{
      r_value = 0;
    }
    
    if(g_index != -1){
      String g_value_string;
      if(b_index > g_index){
        g_value_string = msg.substring(g_index+1, b_index-1);
      }else{
        g_value_string = msg.substring(g_index+1);
      }
      
      for(int i = 0;i< g_value_string.length(); i++){
        if( 0 == isDigit( g_value_string.charAt(i) )){
          error = 1;
        }
      }
      g_value = g_value_string.toInt();
      
    }else{
      g_value = 0;
    }
    
    if(b_index != -1){
      String b_value_string = msg.substring(b_index+1);
      
      for(int i = 0;i< b_value_string.length(); i++){
        if( 0 == isDigit( b_value_string.charAt(i) )){
          error = 1;
        }
      }
      b_value = b_value_string.toInt();
      
      
    }else{
      b_value = 0;
    }
    
    if( error){
      Serial.println( "invalid Command, try again" );
    }else{
      Serial.print("Successful Command! writing LED to RGB value (");
      Serial.print(r_value);
      Serial.print(",");
      Serial.print(g_value);
      Serial.print(",");
      Serial.print(b_value);
      Serial.println(")");
    LED_ctrl(255 - r_value,255 -  g_value,255 - b_value);
    }   
  }
}
