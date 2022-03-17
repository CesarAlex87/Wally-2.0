#include <Q2HX711.h>              
//Configuración LCD
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd(0x27,20,4); // En este caso, usaremos la dirección 0x27 por el tipo de LCD que estamos utilizando, si no funciona, utiliza 0x3f que es la que se utiliza por defecto.

//Pins 
const byte hx711_data_pin = 3;    //Data pin from HX711
const byte hx711_clock_pin = 2;   //Clock pin from HX711
int tara_button = 8;              //Botón de tara
int mode_button = 11;             //Botón de cambio de modo
Q2HX711 hx711(hx711_data_pin, hx711_clock_pin); // Preparar hx711

//Variables
/////////Cambia aquí tu variable de masa////////////
float y1 = 500.0; // Añade tu masa de calibración
//////////////////////////////////////////////////////////

long x1 = 0L;
long x0 = 0L;
float avg_size = 200.0; // Promedio de tasa de refresco de lectura 
float tara = 0;
bool tara_pushed = false;
bool mode_pushed = false;
int mode = 0;
float oz_conversion = 0.035274;
//////////////////////////////////////////////////////////



void setup() {
  Serial.begin(9600);                 //Preparar serial port
  PCICR |= (1 << PCIE0);              //Activar scan PCMSK0                                                 
  PCMSK0 |= (1 << PCINT0);            //Definir al  pin D8 como Botón de Tara con respecto a la interrupción
  PCMSK0 |= (1 << PCINT3);            //Definir al  pin D11 como Botón de Modo con respecto a la interrupción 
  pinMode(tara_button, INPUT_PULLUP);
  pinMode(mode_button, INPUT_PULLUP);

  lcd.init();                         //Iniciar la pantalla LCD
  lcd.backlight();                    //Activar backlight 
  
  delay(10);                        // Permitir celda de carga y setear hx711
  
  // Procedimiento de tara
  for (int ii=0;ii<int(avg_size);ii++){
    delay(1);
    x0+=hx711.read();
  }
  x0/=long(avg_size);
  Serial.println("Agregar masa de calibracion");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  Agregar masa ");
  lcd.setCursor(0,1);
  lcd.print(" de calibracion");
  // Procedimiento de calibración (La masa debe ser igual a la variable y1 definida con anterioridad)
  int ii = 1;
  while(true){
    if (hx711.read()<x0+10000)
    {
    } 
    else 
    {
      ii++;
      delay(50);
      for (int jj=0;jj<int(avg_size);jj++){
        x1+=hx711.read();
      }
      x1/=long(avg_size);
      break;
    }
  }
  Serial.println("Calibracion Completada");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  Calibracion   ");
  lcd.setCursor(0,1);
  lcd.print("   Completada   ");
}

void loop() {
  // Promedio de lectura
  long reading = 0;
  for (int jj=0;jj<int(avg_size);jj++)
  {
    reading+=hx711.read();
  }
  reading/=long(avg_size);

  
  
  // Cálculo de la masa basado en la calibración y el valor medido
  float ratio_1 = (float) (reading-x0);
  float ratio_2 = (float) (x1-x0);
  float ratio = ratio_1/ratio_2;
  float mass = y1*ratio;

  if(tara_pushed)
  {
    tara = mass;
    tara_pushed = false;
    Serial.print("TARA");
    Serial.print(".");
    lcd.setCursor(0,0);
    lcd.print("      TARA      ");
    lcd.setCursor(0,1);
    lcd.print("      .         ");    
    delay(30);
    Serial.print(".");
    lcd.setCursor(0,0);
    lcd.print("      TARA      ");
    lcd.setCursor(0,1);
    lcd.print("      ..        "); 
    delay(30);
    Serial.println(".");
    lcd.setCursor(0,0);
    lcd.print("      TARA      ");
    lcd.setCursor(0,1);
    lcd.print("      ...       "); 
    delay(30);   
  }
  if(mode_pushed)
  {
    mode = mode + 1;
    mode_pushed = false;
    if(mode > 2)
    {
      mode = 0;
    }
  }
  //Modos de la balanza, alternar para que el LCD muestre distintos valores.
  if(mode == 0)
  {
    float masa=mass-tara;
    int canmas=abs(masa)/3;
     
    Serial.println(canmas);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Mascarillas     ");
    lcd.setCursor(0,1);
    lcd.print(abs(canmas));
     
  }
  else if(mode == 1)
  {
    Serial.print(mass - tara);
    Serial.println(" g");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Peso Total     ");
    lcd.setCursor(0,1);
    lcd.print(mass - tara);
    lcd.print(" g");
  }
  else
  {
    Serial.print((mass - tara)*oz_conversion);
    Serial.println(" oz");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Peso Total     ");
    lcd.setCursor(0,1);
    lcd.print((mass - tara)*oz_conversion);
    lcd.print(" oz");
  }
  
}


//Interrupción por boton detectado
ISR(PCINT0_vect)
{
  if (!(PINB & B00000001))
  {
    tara_pushed = true;           //Botón de tara presionado
  }
  
  if (!(PINB & B00001000))
  {
    mode_pushed = true;           //Botón de modo presionado
  }
}
