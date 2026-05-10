#include "Display_ST7789.h"

void setup()
{       
//  Flash_test();
  LCD_Init();
//  Lvgl_Init();
//  SD_Init();   

  // Lvgl_Example1();     
  // lv_demo_widgets();               
  // lv_demo_benchmark();          
  // lv_demo_keypad_encoder();     
  // lv_demo_music();  
  // lv_demo_stress();   

  // Wireless_Test2();  

  ledcWrite(EXAMPLE_PIN_NUM_BK_LIGHT, 20);

}

void loop()
{
  //Timer_Loop();
  delay(5);
}
