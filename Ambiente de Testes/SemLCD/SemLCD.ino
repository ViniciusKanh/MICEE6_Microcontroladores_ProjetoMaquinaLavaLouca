#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Definindo as Portas das I/Os
#define VS 2  // Output, Valvula Superior
#define VI 4  // Output, Valvula Inferior
#define AQ 15 // Output, Aquecedor
#define L 34  // Input, Botao Liga
#define ST 32 // Input, Sensor de Temperatura
#define SP 35 // Input, Sensor da Porta
#define buzzer 14

int ID = 1;
int t1, t2;

String topicomqtt = "Estado";

const char *broker = "broker.hivemq.com";
const int port = 1883;

WiFiClient wifiClient;
PubSubClient cliente;

// Variavel que armazanam o tempo de espera dos diferentes esta2000s o aquecimento da Agua
// const int Tcl = 1200000; // um milhão e dozentos mil milissegundos que dariam 20 minutos
int Tcl = 4000; // um minutos para teste
// const int Tsa = 300000;  // trezentos mil milissegundos que dariam 5 minutos
int const Tsa = 3000;
// const int Tdl = 120000;  // cento e vinte mil milissegundos que dariam 2 minutos
int const Tdl = 3000;
// const int Tel = 120000;  // cento e vinte mil milissegundos que dariam 2 minutos
int const Tel = 3000;

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

String msg;

void callback(const char *topic, byte *payload, unsigned int length)
{

    if ((String)topic == "Tempo")
    {
        String aux = "";
        String id = "";
        String param = "";
        String value = "";

        for (int i = 0; i < length; i++)
        {
            msg += (char)payload[i];
        }

        aux = msg.substring(0, msg.indexOf(","));
        id = aux.substring(aux.indexOf(":") + 1, aux.indexOf(","));
        msg = msg.substring(msg.indexOf(",") + 1, length);
        if (ID == id.toInt())
        {
            aux = "";
            aux = msg.substring(0, msg.indexOf(","));
            param = aux.substring(0, aux.indexOf(":"));
            msg = msg.substring(msg.indexOf(":") + 1, length);
            Serial.println("Teste");
            if (param.compareTo("Tempo") == 0)
            {
                Serial.println("Entra no IF");
                value = "";
                value = msg;
                Tcl = value.toInt();
                Serial.println(Tcl);
            }
        }
    }

   // delay(750);
}

int TempoS(int segundos)
{
    int TempoSecundos;
    TempoSecundos = segundos * 10000;

    return TempoSecundos;
}

void setup()
{

    // pinMode(LED, OUTPUT);
    Serial.begin(9600);
    //delay(10);

    t1 = millis();       // iniciando t1 com o tempo de agora
    pinMode(L, INPUT);   // Definindo Input Botão ON OFF
    pinMode(ST, INPUT);  // Definindo Input Sensor de Temperatura
    pinMode(SP, INPUT);  // Definindo Input Sensor de Porta
    pinMode(VS, OUTPUT); // Definindo Output Valvula de Agua Superior
    pinMode(VI, OUTPUT); // Definindo Output Valvula de Agua Inferior
    pinMode(AQ, OUTPUT); // Definindo Output Dispositivo Aquecedor
    pinMode(buzzer, OUTPUT);

    t2 = millis();

    // Maquina Inicia no Estado Desligado
    estado = A;

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
    cliente.subscribe("Tempo");
    cliente.setCallback(callback);

    // Tcl = 3000;
}

void loop()
{

    if (!cliente.connected())
    {
        cliente.connect("Esp32", "Esp32MICE6", "ifsp");
        cliente.subscribe("Tempo");
    }

   // delay(750);
    msg = "";

    cliente.loop();

    t2 = millis(); // Atualizando t2

    if (estado == A)
    {

        digitalWrite(VS, LOW); // Definindo a Valvula de Agua Superior
        digitalWrite(VI, LOW);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, LOW);

        // delay(1000);
       // cliente.publish("Estado", "Desligado");
        msg = "";
        Serial.println("Desligado");
        msg = ("Desligado");

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

        // delay(1000);
       // cliente.publish("Estado", "Aquecendo");
       // delay(100);
        msg = "";
        Serial.println("Aquecendo");
        msg = ("Aquecendo");

        if (digitalRead(ST) == 1)
        {
            // Serial.print(data.temperature);
            estado = C;
            t1 = millis();
        }

        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {

            estado = A;
         //   cliente.publish("Estado", "Desligado");
            Serial.println("Desligado");
            delay(100);
            delay(1000);
        }
    }
    // Terceiro Estado C = Ciclo de Lavagem
    if (estado == C)
    {

       // cliente.publish("Estado", "Ciclo de Lavagem");
       // delay(100);
        digitalWrite(VS, HIGH);
        digitalWrite(VI, HIGH);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, LOW);

        // delay(1000);
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
               // delay(1000);
            } while (Temporizador);
            estado = D;
            t1 = millis(); // Atualizando t1 apos o encerramento da Saida de Agua para ultilizar o parametro Tdl - Tempo de Dispersão do Liquido
        }

        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {
       //     cliente.publish("Estado", "Desligado");
        //    delay(100);
            estado = A;
            delay(1000);
        }
    }

    // Quarto Estado D = Saida de Agua
    if (estado == D)
    {

       // cliente.publish("Estado", "Saida de Agua");
       // delay(100);
        digitalWrite(VS, LOW);
        digitalWrite(VI, LOW);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, LOW);

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
                delay(1000);
            } while (seg);
            estado = E;
            t1 = millis(); // Atualizando t1 apos o encerramento da Saida de Agua para ultilizar o parametro Tdl - Tempo de Dispersão do Liquido
        }

        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {

            cliente.publish("Estado", "Desligado");
           // delay(1000);
            estado = A;
            // delay(1000);
        }
    }

    // Quinto Estado E = Dispersão do Liquido Secante
    if (estado == E)
    {

      //  cliente.publish("Estado", "Dispersão do Liquido Secante");
      //  delay(100);
        digitalWrite(VS, LOW);
        digitalWrite(VI, LOW);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, LOW);
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
                delay(1000);
            } while (seg);
            estado = F;
            t1 = millis(); // Atualizando t1 apos o encerramento da dispersão do Liquido e inicio do escoamento do mesmo pelo Tempo de Escoamento do Liquido.
        }

        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {

            cliente.publish("Estado", "Desligado");
           // delay(1000);
            estado = A;
           // delay(1000);
        }
    }

    if (estado == F)
    {

      //  cliente.publish("Estado", "Escoamento do Liquido Secante");
       // delay(100);
        digitalWrite(VS, LOW);
        digitalWrite(VI, LOW);
        digitalWrite(AQ, LOW);
        digitalWrite(buzzer, HIGH);

       // delay(1000);
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
                delay(1000);
            } while (seg);
            estado = A;
            t1 = millis(); // Atualizando t1 apos o encerramento da dispersão do Liquido e inicio do escoamento do mesmo pelo Tempo de Escoamento do Liquido.
        }
        // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
        if (digitalRead(SP) == 0)
        {
      //      cliente.publish("Estado", "Desligado");
           // delay(1000);
            estado = A;
          //  delay(1000);
        }
    }
}