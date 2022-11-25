/*************************************************************

  You’ll need:
   - Blynk IoT app (download from App Store or Google Play)
   - ESP32 board
   - Decide how to connect to Blynk
     (USB, Ethernet, Wi-Fi, Bluetooth, ...)

  There is a bunch of great example sketches included to show you how to get
  started. Think of them as LEGO bricks  and combine them as you wish.
  For example, take the Ethernet Shield sketch and combine it with the
  Servo example, or choose a USB sketch and add a code from SendData
  example.
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPLAUm3kvs4"
#define BLYNK_DEVICE_NAME "Blynk"
#define BLYNK_AUTH_TOKEN "KaGWDUR3aNj5m-cQwaYWMB53xyUczwzq"


// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#define VS 15   // Output, Valvula Superior
#define VI 34   // Output, Valvula Inferior
#define AQ 32  // Output, Aquecedor
#define L 2   // Input, Botao Liga
#define ST 4  // Input, Sensor de Temperatura
#define SP 12  // Input, Sensor da Porta
#define buzzer 14 // Output, Buzina
#define LevelWater 13
int valorSensor = 0;  // Essa variavel inicia com valor zero
int valorSaida = 0;   // Essa variavel inicia com valor zero

// int t1Tcl, t2Tcl, t1Tsa, t2Tsa,t1Tel, t2Tel ;
int t1, t2;

// Variavel que armazanam o tempo de espera dos diferentes esta2000s o aquecimento da Agua
// const int Tcl = 1200000; // um milhão e dozentos mil milissegundos que dariam 20 minutos
int const Tcl = 10000;  // um minutos para teste
// const int Tsa = 300000;  // trezentos mil milissegundos que dariam 5 minutos
int const Tsa = 10000;
// const int Tdl = 120000;  // cento e vinte mil milissegundos que dariam 2 minutos
int const Tdl = 10000;
// const int Tel = 120000;  // cento e vinte mil milissegundos que dariam 2 minutos
int const Tel = 2000;

enum Estado {
  A,
  B,
  C,
  D,
  E,
  F
};

Estado estado;

#include <WiFi.h> // Biblioteca para ultilizar o Wifi no ESP
#include <WiFiClient.h> // Biblioteca para usar o ESP como Cliente
#include <BlynkSimpleEsp32.h> // Biblioteca para usarmos o Blink
#include <LiquidCrystal_I2C.h> // Biblioteca para ultilizar o Display LCD com I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Configurando o Display

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "IFSP - IFMaker";
char pass[] = "gbbme8dn";

WidgetLED led1(V26); // LED do APP para o Estado B Aquecendo
WidgetLED led2(V25); // LED do APP para o Estado C Ciclo de Lavagem
WidgetLED led3(V2); // LED do APP para o Estado D Saida de Agua
WidgetLED led4(V3); // LED do APP para o Estado E Dispersao do Liquido Secante
WidgetLED led5(V4); // LED do APP para o Estado A Desligado
WidgetLED led6(V5); // LED do APP para o Estado F Escoamento do Liquido Secante

BlynkTimer timer;

// V1 LED Widget is blinking
void blinkLedWidget() // Funcao para ultilizar os LEDs no APP
{
  if (estado == A) {
    led5.on();
  } else {
    led5.off();
  }
  if (estado == B) {
    led1.on();
  } else {
    led1.off();
  }
  if (estado == C ) {
    led2.on();
  } else {
    led2.off();
  }
   if (estado == D ) {
    led3.on();
  } else {
    led3.off();
  }
   if (estado == E ) {
    led4.on();
  } else {
    led4.off();
  }
   if (estado == F) {
    led6.on();
  } else {
    led6.off();
  }
}


void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
  t1 = millis();             // iniciando t1 com o tempo de agora
  lcd.begin(16, 2);          // Iniciando a variavel que controlara o Display
  pinMode(L, INPUT);  // Definindo Input Botão ON OFF
  pinMode(ST, INPUT);        // Definindo Input Sensor de Temperatura
  pinMode(SP, INPUT);        // Definindo Input Sensor de Porta
  pinMode(VS, OUTPUT);       // Definindo Output Valvula de Agua Superior
  pinMode(VI, OUTPUT);       // Definindo Output Valvula de Agua Inferior
  pinMode(AQ, OUTPUT);       // Definindo Output Dispositivo Aquecedor
  pinMode(buzzer, OUTPUT);   // Definindo Output Dispositivo Sonoro para Fim de Estado

  t2 = millis(); // Atualizando T2
  // Maquina Inicia no Estado Desligado
  estado = A; // Iniciando a Maquina no Estado A
  lcd.init();
   timer.setInterval(1000L, blinkLedWidget); // Requisitando a cada um Segundo a funcao que mostrar os Estado para o usuario

}

void loop()
{
  Blynk.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
  timer.run();

  t2 = millis();  // Atualizando t2
  valorSensor = analogRead(LevelWater); // Ajustando o sensor de nivel de Agua
  valorSaida = map(valorSensor, 0, 100, 0, 2000); // configurando o ranger Sensor do nivel de Agua
  // Serial.print("\t Detection Water = ");
   //Serial.println(valorSaida);


  if (estado == A) {
    

    digitalWrite(VS, LOW);  
    digitalWrite(VI, LOW);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, LOW);
    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Desligado");
    delay(1000);

    if (digitalRead(L)==1 and digitalRead(SP)==1) {

      // Fechando a Porta e Acionando o Botão ON OFF para Alto
      estado = B;
    } else {

      estado = A;
    }
  }
  // Segundo Estado B = Aquecendo
  if (estado == B) {
     
    digitalWrite(VI, LOW);
    digitalWrite(VS, LOW);
    digitalWrite(AQ, HIGH);
    digitalWrite(buzzer, LOW);
    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Aquecendo");
    delay(1000);
    if (digitalRead(ST) == 1 ) {
      //Serial.print(data.temperature);
      estado = C;
      t1 = millis();
    }
    // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
    if (digitalRead(SP) == 0 || digitalRead(L)== 0) {
    
      estado = A;
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);

    }
  }

  // Terceiro Estado C = Ciclo de Lavagem
  if (estado == C) {
   
    digitalWrite(VS, HIGH);
    digitalWrite(VI, HIGH);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, LOW);
    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Ciclo de Lavagem");
    delay(1000);

    if ((t2 - t1) >= Tcl) {  // TCL Timer de Tempo de Lavagem 20 Minutos
      estado = D;
      t1 = millis();  // Atualizando t1 apos o encerramento do ciclo de Lavagem para ultilizar o parametro Tsa - Tempo de Saida de Saida de Agua
    }
    // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
    if (digitalRead(SP) == 0 || digitalRead(L)== 0 ) {
      
      estado = A;
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);
    }
  }

  // Quarto Estado D = Saida de Agua
  if (estado == D) {
    
    digitalWrite(VS, LOW);
    digitalWrite(VI, LOW);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, LOW);
    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Saida de Agua");
    delay(1000);

    if ((t2 - t1) >= Tsa and valorSaida < 200) {  // D Timer de Tempo de Saida de Agua 5 Minutos
      estado = E;
      t1 = millis();  // Atualizando t1 apos o encerramento da Saida de Agua para ultilizar o parametro Tdl - Tempo de Dispersão do Liquido
    }
    // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
    if (digitalRead(SP) == 0 ||  digitalRead(L)== 0 ) {
     
      estado = A;
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);
    }
  }

  // Quinto Estado E = Dispersão do Liquido Secante
  if (estado == E) {
    
    digitalWrite(VS, LOW);
    digitalWrite(VI, LOW);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, LOW);
    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Dispersao Liquido");
    delay(1000);
    if ((t2 - t1) >= Tsa) {  // D Timer de Tempo de Dispersão do Liquido Secante 3 Minutos
      estado = F;
      t1 = millis();  // Atualizando t1 apos o encerramento da dispersão do Liquido e inicio do escoamento do mesmo pelo Tempo de Escoamento do Liquido.
    }
    // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
    if (digitalRead(SP) == 0 || digitalRead(L)== 0) {
      estado = A;
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);
     
    }
  }

  // Ultimo Estado F = Escoamento do Liquido Secante
  if (estado == F) {
    
    digitalWrite(VS, LOW);
    digitalWrite(VI, LOW);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, HIGH);
    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Escoamento do Liquido Secante");
    delay(1000);
    if ((t2 - t1) >= Tel) {  // Timer para o Tempo de Escoamento do Liquido de 2 Minutos
      estado = A;
      t1 = millis();
      
    }
    if (digitalRead(SP) == 0 || digitalRead(L)== 0) {  // Usuario abre a porta a maquina desliga
      estado = A;
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);
    }
  }
}
