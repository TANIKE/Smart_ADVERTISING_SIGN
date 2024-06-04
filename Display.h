#include <Wire.h>
#include "SSD1306.h"

SSD1306Wire display(0x3c, 2, 14);
#include <OneButton.h>
OneButton button(SW, true);

///Menu
bool toMenu = 0; // 0: main; 1: Menu
bool toDetail = 0; // 0: Menu; 1: Detail
int Menu = 0;
int Menu_count = 0;

////
int sensor_index = 0;
int8_t numbers_cate = 5;
String sensor_states[3] = {"No Target", "Motion", "Station"};
String categories[5] = {"Close", "Data", "Return", "Reset", "Setting"};
/////Screen
int brightness = 0;
int brightness_count = 0;
uint8_t brightness2;

void display_Main(){
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(10,50, "Latest: 27/2/2003");
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(100,5, sensor_states[sensor_index]);
  display.display();
}
void change_Mainstate(){
  if(toMenu == 0){
    display.clear();

    display.drawLine(9, 18, 40, 18);
    display.drawLine(9, 46, 40, 46);

    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(24,23,categories[Menu]);
    display.fillRect(54,5, 70, 54);
    
    display.display();
  }
  toMenu = !toMenu;
}
void change_Menustate(){
  toDetail = 1;
}
void display_Menu(){
  bool changed = check_rotary(&Menu_count, &Menu, 0, numbers_cate - 1);
  if(changed){
    display.clear();

    display.drawLine(9, 18, 40, 18);
    display.drawLine(9, 46, 40, 46);

    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(24,23,categories[Menu]);
    display.fillRect(54,5, 70, 54);

    display.display();
  }
}
void return_Menu(){
  toDetail = 0;
  display.setColor(WHITE);
  display.fillRect(54,5, 70, 54);
}
void setting(){
  bool changed = check_rotary(&brightness_count, &brightness, 0, 150);
  brightness2 = brightness;
  display.setColor(WHITE);
  display.fillRect(54,5, 70, 54);
  display.setColor(BLACK);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(59, 10, "Brightness");

  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(89, 32, String(255 - brightness));

  display.setBrightness(255 - brightness2);
  display.display();

  button.attachClick(return_Menu);
}
void menu_change(){
  if(!toMenu) {
    display_Main();
    button.attachClick(change_Mainstate);
  }
  else{
    if(!toDetail){
      display_Menu();
      button.attachClick(change_Menustate);
    }
    else{
      switch (Menu){
        case 0:

          break;
        case 1:

          break;
        case 2:
          toDetail = 0;
          toMenu = 0;
          break;
        case 3:

          break;
        case 4:
          setting();
          break;
      }
    }
  }
}