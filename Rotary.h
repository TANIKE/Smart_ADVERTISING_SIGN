////Rotary
#define CLK_pin 4
#define DT_pin 13
#define SW 15

unsigned long lastDebounce = 0;

int8_t last_CLK;
int8_t last_DT;

bool check_rotary(int *pre, int *cur, uint8_t min, uint8_t max){
  bool once = 0;
  if ((millis() - lastDebounce) > 0.01) { //debounce 10ms
    if ((last_CLK == 0) && (last_DT == 1)) {
      if ((digitalRead(CLK_pin) == 1) && (digitalRead(DT_pin) == 0)) {
        *pre = *pre - 1; once = 1;
        Serial.println(*pre);
      }
      else if ((digitalRead(CLK_pin) == 1) && (digitalRead(DT_pin) == 1)) {
        *pre = *pre + 1; once = 1;
        Serial.println(*pre);
      }
    }

    else if ((last_CLK == 1) && (last_DT == 0)) {
      if ((digitalRead(CLK_pin) == 0) && (digitalRead(DT_pin) == 1)) {
        *pre = *pre - 1; once = 1;
        Serial.println(*pre);
      }
      else if ((digitalRead(CLK_pin) == 0) && (digitalRead(DT_pin) == 0)) {
        *pre = *pre + 1; once = 1;
        Serial.println(*pre);
      }
    }

    else if ((last_CLK == 1) && (last_DT == 1)) {
      if ((digitalRead(CLK_pin) == 0) && (digitalRead(DT_pin) == 1)) {
        *pre = *pre - 1; once = 1;
        Serial.println(*pre);
      }
      else if ((digitalRead(CLK_pin) == 0) && (digitalRead(DT_pin) == 0)) {
        *pre = *pre + 1; once = 1;
        Serial.println(*pre);
      }
    }  

    else if ((last_CLK == 0) && (last_DT == 0)) {
      if ((digitalRead(CLK_pin) == 1) && (digitalRead(DT_pin) == 0)) {
        *pre = *pre - 1; once = 1;
        Serial.println(*pre);
      }
      else if ((digitalRead(CLK_pin) == 1) && (digitalRead(DT_pin) == 1)) {
        *pre = *pre + 1; once = 1;
        Serial.println(*pre);
      }
    }

    last_CLK=digitalRead(CLK_pin);
    last_DT=digitalRead(DT_pin);
    
    lastDebounce = millis();  // Set variable to current millis() timer
    if(once == 1){
      if(*pre % 2 == 0){
        *cur = *pre/2 ;
        if(*cur > max) {*cur = min; *pre = min*2;}
        else if(*cur < min) {*cur = max; *pre = max*2;}
        Serial.println("DIS: " + String(*cur));
      }
    }
  }
  return once;
}