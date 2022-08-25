 #include <EEPROM.h>  
 #include <TimeAlarms.h>
 #include <DS3232RTC.h>  
 #include <avr/wdt.h>
 #define minutos  60000;
 
 // pins 
 int ledauto_pin = 44;
 int stopsw_pin = 45;
 int startsw_pin = 43; 
 int autosw_pin = 42; 
 int motor_relay_pin = 47;
  
 
 //running variables
 unsigned long timerun_millis = 0;
 unsigned long current_millis = 0;
 unsigned long timer1 = 0; 
 unsigned long timeauto = 0;  
 
 boolean run_bool = false;
 boolean auto1_bool = false; 
 boolean autopress_bool = true;
 boolean armed_bool = false;
 
 DS3232RTC myRTC;
 
void setup() {
  //pins and settings declared 
     myRTC.begin();    
     setSyncProvider(myRTC.get);

     pinMode(ledauto_pin, INPUT_PULLUP); 
     pinMode(motor_relay_pin, OUTPUT);
     
     pinMode(startsw_pin, INPUT_PULLUP);
     pinMode(stopsw_pin, INPUT_PULLUP);
     pinMode(autosw_pin, INPUT_PULLUP);        
          
     digitalWrite(motor_relay_pin, LOW);
     digitalWrite(ledauto_pin, LOW);
     
     //eeprom
     if(EEPROM.read(3) == 1) auto1_bool = true;          
     
     //led
     if(auto1_bool == true) digitalWrite(ledauto_pin, HIGH); 
     
     //alarms and dst
    
      Alarm.alarmRepeat(dowFriday,9,0,0,WeeklyAlarmdst);  // TEMPO EM UTC !!  hora real verao(dst) = utc   || hora real inverno = utc-1
      Alarm.alarmRepeat(dowFriday,10,0,0,WeeklyAlarm); // Alarme de inverno
    
     
      myRTC.writeRTC(16,0); // RTC aging offset (+1 = 0.1ppm) Fazer as contas para afinar na proxima "manutenção". Valor positivo=relogio anda mais devagar  24-08-2022 actualizaçao
      wdt_enable(WDTO_8S);  // watchdog       
 }
 
 
void loop() {   
   //---------------------------------------------------------
   //              [ 1 ]    RUN MOTOR: relay and start/stop   
   
    //start/stop  


   if(digitalRead(startsw_pin) == LOW && armed_bool == true) {
       run_bool = true;     
       timerun_millis = millis();
       armed_bool = false;
       timeauto = 30*minutos;          
   }   

   if(digitalRead(stopsw_pin) == LOW) {       
      run_bool = false;        
      armed_bool = true;  
   }
   
   // timer   
   if( milli(timerun_millis) > timeauto) run_bool = false;  
   
   //run motor
   if(run_bool == true){
    digitalWrite(motor_relay_pin, HIGH);            
   }
   
   if(run_bool == false){
    digitalWrite(motor_relay_pin, LOW);        
   }
     
   
     //---------------------------------------------------------
     //                 [ 2 ]    AUTO, led and eeprom
   
   
   if(digitalRead(autosw_pin) == HIGH) {
     timer1 = millis();     //reset timer   
     autopress_bool = false;
   }     
   
   if(digitalRead(autosw_pin) == LOW && autopress_bool == false) { 
                  
      if( milli(timer1) > 2500) {
        autopress_bool = true;
                  
        if(auto1_bool == true) {
          auto1_bool = false;
          EEPROM.write(3,0);
          digitalWrite(ledauto_pin, LOW);          
        }        
        else {
          auto1_bool = true;
          EEPROM.write(3,1);
          digitalWrite(ledauto_pin, HIGH);          
        }         
      }  
   }      
   
        
       
   
   
     //---------------------------------------------------------   
     //    [ 3 ]    WATCHDOG
     
    wdt_reset(); 

     //---------------------------------------------------------
     //    [ 4 ]    RTC 
     
   // alarms declared at setup
   
     Alarm.delay(10);  // runs scheduler  SHOULD BE LAST THING !     
} 

 
unsigned long milli(unsigned long teste){  // millis since "teste"
  unsigned long timerlong;
  current_millis = millis();
  timerlong = current_millis - teste;
  return timerlong;
}

 
void WeeklyAlarm(){  
   if(month() < 4 || month() > 10){
    alarm_func();
    }
 } 

void WeeklyAlarmdst(){  // alarme verao
  if(month() > 3 && month() < 11){
    alarm_func();
    }
   
 }

void alarm_func(){
   if(auto1_bool == true && run_bool == false) {
     timerun_millis = millis();
     run_bool = true;     
     armed_bool= false;
     timeauto = 20*minutos ;      //Nov,Dez,Jan,Fev,Mar
     if(month() > 3 && month() < 11){
      timeauto = 25*minutos;      //Abr,Mai,Set,Out
      }
     if(month() > 5 && month() < 9){
      timeauto = 35*minutos;  //Jun,Jul,Ago
      }          
    }  
}
 
