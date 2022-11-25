
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <string.h>
#include <Wire.h>

// Inicializa o display no endereco
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definindo as Portas das I/Os
#define VS 2  // Output, Valvula Superior
#define VI 4  // Output, Valvula Inferior
#define AQ 15 // Output, Aquecedor
#define L 34  // Input, Botao Liga
#define ST 32 // Input, Sensor de Temperatura
#define SP 35 // Input, Sensor da Porta
#define buzzer 14

int t1, t2;

String topicomqtt = "Estado";

const char *broker = "broker.hivemq.com";
const int port = 1883;

WiFiClient wifiClient;
PubSubClient cliente;

// Variavel que armazanam o tempo de espera dos diferentes esta2000s o aquecimento da Agua
// const int Tcl = 1200000; // um milhão e dozentos mil milissegundos que dariam 20 minutos
int Tcl = 1000; // um minutos para teste
// const int Tsa = 300000;  // trezentos mil milissegundos que dariam 5 minutos
int const Tsa = 10000;
// const int Tdl = 120000;  // cento e vinte mil milissegundos que dariam 2 minutos
int const Tdl = 5000;
// const int Tel = 120000;  // cento e vinte mil milissegundos que dariam 2 minutos
int const Tel = 2000;

// Definindo os Est,
enum Estado
{
    A,
    B,
    C,
    D,
    E,
    F
};

Estado estado;

const char *ssid = "Vini Casa";
const char *password = "leidebiavi";

WiFiServer server(80);

String msg;

void callback(const char *topic, byte *payload, unsigned int length)
{
    //Serial.println("callback");
    if((String)topic == "Timer"){
       
        String valor = "";
        valor = msg;

         for (int i = 0; i < length; i++)
        {
            msg += (char)payload[i];
        }

        Tcl = msg.toInt();
        Tcl = Tcl * 1000;
        Serial.println(Tcl);
    }
    
    //delay(750);
}

int TempoS(int segundos)
{
    int TempoSecundos;
    TempoSecundos = segundos * 10000;

    return TempoSecundos;
}


void setup() {
  // pinMode(LED, OUTPUT);
    Serial.begin(115200);
    delay(10);

    t1 = millis();       // iniciando t1 com o tempo de agora
    lcd.begin(16,2);    // Iniciando a variavel que controlara o Display
    pinMode(L, INPUT);   // Definindo Input Botão ON OFF
    pinMode(ST, INPUT_PULLUP);  // Definindo Input Sensor de Temperatura
    pinMode(SP, INPUT_PULLUP);  // Definindo Input Sensor de Porta
    pinMode(VS, OUTPUT); // Definindo Output Valvula de Agua Superior
    pinMode(VI, OUTPUT); // Definindo Output Valvula de Agua Inferior
    pinMode(AQ, OUTPUT); // Definindo Output Dispositivo Aquecedor
    pinMode(buzzer, OUTPUT);

    t2 = millis();

    // Maquina Inicia no Estado Desligado
    estado = A;
    lcd.init();

    delay(10);

    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");

    // Conectar ao HiveMQ
    cliente.setClient(wifiClient);
    cliente.setServer("broker.hivemq.com", 1883);
    cliente.connect("Esp32", "Esp32MICE6", "ifsp");
    cliente.subscribe("Estado");
    cliente.subscribe("Timer");
    cliente.setCallback(callback);


}

void loop() {
 if (!cliente.connected())
    {
        cliente.connect("Esp32", "Esp32MICE6", "ifsp");
        cliente.subscribe("Timer");
    }

    delay(750);
    msg = "";
    cliente.loop();
    //delay(1);

    t2 = millis(); // Atualizando t2

    if (estado == A)
    {
        //cliente.connect("Esp32", "Esp32MICE6", "ifsp");
        //cliente.subscribe("Tcl");
        digitalWrite(VS, LOW); // Definindo a Valvula de Agua Superior
        digitalWrite(VI, LOW);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, LOW);

        lcd.clear();
        lcd.setBacklight(HIGH);
        lcd.setCursor(0, 0);
        lcd.print("Desligado");
        
        cliente.publish("Estado", "Desligado");
        msg = "";
        Serial.println("Desligado");
        msg = ("Desligado");
        delay(1);

        if (digitalRead(L) == 1 and digitalRead(SP) == 1)
        {
            // Fechando a Porta e Acionando o Botão ON OFF para Alto
            estado = B;
        }
        else
        {
            estado = A;
        }
    }

    // Segundo Estado B = Aquecendo
    if (estado == B)
    {

        digitalWrite(VI, LOW);
        digitalWrite(VS, LOW);
        digitalWrite(AQ, HIGH);
        digitalWrite(buzzer, LOW);

        lcd.clear();
        lcd.setBacklight(HIGH);
        lcd.setCursor(0, 0);
        lcd.print("Aquecendo");
        
        cliente.publish("Estado", "Aquecendo");
        delay(100);
        msg = "";
        Serial.println("Aquecendo");
        msg = ("Aquecendo");

        if (digitalRead(ST) == 1)
        {
            estado = C;
            t1 = millis();
        }

        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {

            estado = A;
            cliente.publish("Estado", "Desligado");
            Serial.println("Desligado");
            lcd.clear();
            lcd.setBacklight(HIGH);
            lcd.setCursor(0, 0);
            lcd.print("Desligado");
            delay(1000);
            
        }
    }
    // Terceiro Estado C = Ciclo de Lavagem
    if (estado == C)
    {
        cliente.publish("Estado", "Ciclo de Lavagem");
        delay(100);
        digitalWrite(VS, HIGH);
        digitalWrite(VI, HIGH);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, LOW);

        lcd.clear();
        lcd.setBacklight(HIGH);
        lcd.setCursor(0, 0);
        lcd.print("Ciclo de Lavagem");

        msg = "";
        msg = ("Ciclo de Lavagem");

        Serial.println("Ciclo de Lavagem");
        Serial.println(Tcl);

        // transformar milisegundos em segundo
        int Temporizador = TempoS(Tcl);
        Temporizador = Tcl / 1000;

        // imprimir tempo do ciclo de lavagem no lcd
        if ((t2 - t1) <= Tcl)
        {
            do
            {
                Temporizador--;
                lcd.setCursor(0, 1);
                lcd.print("Tempo: ");
                lcd.print(".               ");
                lcd.setCursor(7, 1);
                lcd.print(Temporizador);
                delay(1000);
            } while (digitalRead(SP) == 1 && Temporizador);
            estado = D;
            t1 = millis(); // Atualizando t1 apos o encerramento da Saida de Agua para ultilizar o parametro Tdl - Tempo de Dispersão do Liquido
        }

        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {
            cliente.publish("Estado", "Desligado");
            estado = A;
            lcd.clear();
            lcd.setBacklight(HIGH);
            lcd.setCursor(0, 0);
            lcd.print("Desligado");
            delay(1000);
        }
    }

    // Quarto Estado D = Saida de Agua
    if (estado == D)
    {

        cliente.publish("Estado", "Saida de Agua");
        delay(100);
        digitalWrite(VS, LOW);
        digitalWrite(VI, LOW);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, LOW);

        lcd.clear();
        lcd.setBacklight(HIGH);
        lcd.setCursor(0, 0);
        lcd.print("Saida de Agua");
        // delay(1000);
        msg = "";
        msg = ("Saida de Agua");

        Serial.println("Saida de Agua");

        // transformar milisegundos em segundo
        int seg = Tsa / 1000;

        // imprimir tempo do ciclo de lavagem no lcd
        if ((t2 - t1) <= Tsa)
        {
            do
            {
                seg--;
                lcd.setCursor(0, 1);
                lcd.print("Tempo: ");
                lcd.print(" ");
                lcd.setCursor(7, 1);
                lcd.print(seg);
                delay(1000);
            } while (digitalRead(SP) == 1 && seg);
            estado = E;
            t1 = millis(); // Atualizando t1 apos o encerramento da Saida de Agua para ultilizar o parametro Tdl - Tempo de Dispersão do Liquido
        }

        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {
            cliente.publish("Estado", "Desligado");
            //delay(1000);
            estado = A;
            lcd.clear();
            lcd.setBacklight(HIGH);
            lcd.setCursor(0, 0);
            lcd.print("Desligado");
            delay(1000);
        }
    }

    // Quinto Estado E = Dispersão do Liquido Secante
    if (estado == E)
    {

        cliente.publish("Estado", "Dispersão do Liquido Secante");
        delay(100);
        digitalWrite(VS, LOW);
        digitalWrite(VI, LOW);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, LOW);
        lcd.clear();
        lcd.setBacklight(HIGH);
        lcd.setCursor(0, 0);
        lcd.print("Dispersao Liquido");
        // delay(1000);
        msg = "";
        msg = ("Dispersao Liquido Secante");

        Serial.println("Dispersao Liquido Secante");

        // transformar milisegundos em segundo
        int seg = Tdl / 1000;

        // imprimir tempo do ciclo de lavagem no lcd
        if ((t2 - t1) <= Tdl)
        {
            do
            {
                seg--;
                lcd.setCursor(0, 1);
                lcd.print("Tempo: ");
                lcd.print(" ");
                lcd.setCursor(7, 1);
                lcd.print(seg);
                delay(1000);
            } while (digitalRead(SP) == 1 && seg);
            estado = F;
            t1 = millis(); // Atualizando t1 apos o encerramento da dispersão do Liquido e inicio do escoamento do mesmo pelo Tempo de Escoamento do Liquido.
        }

        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {

            cliente.publish("Estado", "Desligado");
            //delay(1000);
            estado = A;
            lcd.clear();
            lcd.setBacklight(HIGH);
            lcd.setCursor(0, 0);
            lcd.print("Desligado");
            delay(1000);
        }
    }

    if (estado == F)
    {

        cliente.publish("Estado", "Escoamento do Liquido Secante");
        //delay(100);
        digitalWrite(VS, LOW);
        digitalWrite(VI, LOW);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, HIGH);

        lcd.clear();
        lcd.setBacklight(HIGH);
        lcd.setCursor(0, 0);
        lcd.print("Escoamento do Liquido Secante");
        //delay(1000);
        msg = "";
        msg = ("Escoamento do Liquido Secante");

        Serial.println("Escoamento do Liquido Secante");

        // transformar milisegundos em segundo
        int seg = Tel / 1000;

        // imprimir tempo do ciclo de lavagem no lcd
        if ((t2 - t1) <= Tdl)
        {
            do
            {
                seg--;
                lcd.setCursor(0, 1);
                lcd.print("Tempo: ");
                lcd.print(" ");
                lcd.setCursor(7, 1);
                lcd.print(seg);
                delay(1000);
            } while (digitalRead(SP) == 1 && seg);
            estado = A;
            t1 = millis(); // Atualizando t1 apos o encerramento da dispersão do Liquido e inicio do escoamento do mesmo pelo Tempo de Escoamento do Liquido.
        }
        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {
            cliente.publish("Estado", "Desligado");
            //delay(1000);
            estado = A;
            lcd.clear();
            lcd.setBacklight(HIGH);
            lcd.setCursor(0, 0);
            lcd.print("Desligado");
            delay(1000);
        }
    }

}
