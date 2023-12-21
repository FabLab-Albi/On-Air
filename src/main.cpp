#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <NeoPixelBus.h>

#include <micro.h>

//*******************************************************************************************************************
// Option de compilation pour faciliter le debugage

#define DEBUG_MAIN

#if defined(DEBUG_MAIN)
#define debugPrint(...)                \
  {                                    \
    Serial.print(String(__LINE__));    \
    Serial.print(": ");                \
    Serial.print(String(__VA_ARGS__)); \
    Serial.print(" ");                 \
  }

#define debugPrintln(...)                \
  {                                      \
    Serial.print(String(__LINE__));      \
    Serial.print(": ");                  \
    Serial.println(String(__VA_ARGS__)); \
  }

#else

#define debugPrint(...)
#define debugPrintln(...)

#endif

//*******************************************************************************************************************
// Option de compilation pour les differents tests



//*******************************************************************************************************************
// Parametres de connexion aux reseaux WiFi

// Domicile
//  #define WIFI_SSID  "Asus_DG"; // id de la connexion WiFi
//  #define WIFI_PASSWORD "deshaiesgalinie";

#define WIFI_SSID "devolo-30d32d310fc9"
#define WIFI_PASSWORD "HRAMNGBUZOKAFWLG"

// Radio Albiges
// #define WIFI_SSID "Livebox-Albiges"
// #define WIFI_PASSWORD "orange_albiges"

String ssid = WIFI_SSID;
String password = WIFI_PASSWORD;

//*******************************************************************************************************************
// Paramètres pour la platine X32
#define nbMicro 2               // nombre de micros à surveiller
#define delaisInterrogation 100 // Délais (en ms) d'attente entre deux interrogation de la platine X32
long instantInterrogation;      // Dernier instant d'interrogation de la platine
long dernierInstantEnvoieCommande = 0;
long dernierInstantAffichageEtat = 0;
long dernierInstantAffichageLED = 0;
Micro *micro[nbMicro];

//*******************************************************************************************************************
// Paramètres pour le ruban de LEDs adressables
#define PIN_RUBAN 34
#define MAX_LED 24
#define PIN_ONAIR 27
typedef NeoGrbwFeature MyPixelColorFeature;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(MAX_LED, PIN_RUBAN);

RgbColor couleurAttente = RgbColor(10, 10, 10);  // couleur de l'anneau en attente de la connexion WiFi
RgbColor couleurWiFiOK = RgbColor(15, 50, 15);   // couleur de l'anneau quand la connexion WiFi est réussie
RgbColor couleurWiFiFail = RgbColor(80, 15, 15); // couleur de l'anneauquand la connexion WiFi a echoué

RgbColor couleurActif = RgbColor(100, 0, 0);   // couleur des LED quand l'état du micro est actif
RgbColor couleurInactif = RgbColor(0, 100, 0); // couleur des LED quand l'état du micro est inactif
RgbColor couleurInconnu = RgbColor(0, 0, 100); // couleur des LED quand l'état du micro est inconnu

//*******************************************************************************************************************
void afficheLed()
{
  bool onAir = false;
  for (size_t i = 0; i < nbMicro; i++)
  {

    switch (micro[i]->etat())
    {
    case actif:
      onAir = true;
      for (size_t j = i * 3; j < i * 3 + 3; j++)
      {
        strip.SetPixelColor(j, couleurActif);
      }
      break;
    case inactif:
      for (size_t j = i * 3; j < i * 3 + 3; j++)
      {
        strip.SetPixelColor(j, couleurInactif);
      }
      break;
    case inconnu:
      for (size_t j = i * 3; j < i * 3 + 3; j++)
      {
        strip.SetPixelColor(j, couleurInconnu);
      }
      break;

    default:
      break;
    }
  }
  strip.Show();

  if (onAir)
    digitalWrite(PIN_ONAIR, HIGH);
  else
    digitalWrite(PIN_ONAIR, LOW);
}

//********************************************************************************************************************
//                 SETUP
//********************************************************************************************************************
void setup()
{
  Serial.begin(9600);
  log_e("log_e");
  delay(1000);
  
  debugPrintln("Début!!");

  // Initialisation du ruban de LED
  pinMode(PIN_RUBAN, OUTPUT);
  pinMode(PIN_ONAIR, OUTPUT);

  strip.Begin();
  for (int i = 0; i < MAX_LED; i++)
  {
    strip.SetPixelColor(i, couleurAttente);
  };
  strip.Show();
  delay(1000); // On affiche cette couleur 1s en attendant la connexion Wifi

  // Initialisation de la connexion WiFi
  debugPrintln("Tentative de connexion au reseau WiFi :" + String(ssid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    debugPrintln("Erreur : Connexion impossible !");
    for (int i = 0; i < MAX_LED; i++)
    {
      strip.SetPixelColor(i, couleurWiFiFail);
    };
    strip.Show();
    delay(5000);
  }
  else
  {
    debugPrintln("Connection WiFi réussie");
    debugPrintln("Ouverture d'un port UDP");
    udp.begin(RECEIVE_PORT);
    for (int i = 0; i < MAX_LED; i++)
    {
      strip.SetPixelColor(i, couleurWiFiOK);
    };
    strip.Show();
    delay(1000); // On affiche cette couleur 1s en pour confirmer la connexion Wifi
  };

  // initialisation des instances de micro
  for (size_t i = 0; i < nbMicro; i++)
  {
    micro[i] = new Micro(i + 1);
    micro[i]->updateFader();
    micro[i]->updateMute();
  };

#ifdef TEST3
  envoiCommande("/xremote");
  dernierInstantEnvoieCommande = millis();
#endif

#ifdef TEST4
  envoiCommande("/formatsubscribe ,ssiii /testme /ch/**/mix/on 6 9 80");
  dernierInstantEnvoieCommande = millis();

#endif
}

//********************************************************************************************************************
//                 LOOP
//********************************************************************************************************************

void loop()
{

  // On verifie si un paquet est reçu
  int packetSize = udp.parsePacket();
  if (packetSize != 0)
  {
    debugPrintln(" Paquet reçu de : " + udp.remoteIP().toString());
    debugPrintln(" Taille : " + String(packetSize));

    if (packetSize > MAX_BUFFER)
    {
      debugPrintln(" ERREUR : Taille du paquet supérieure à celle du buffer !");
    }
    else
    {
      // TRAITEMENT DU PAQUET
      int len = udp.read(packetBuffer, packetSize);
      // stockage de la réponse dans une chaine de caractère en remplaçant
      // les caractères nuls et retour ligne par des '_'
      String strReponse = "";
      for (size_t i = 0; i < len; i++)
      {
        // debugPrintln(packetBuffer[i]);
        // debugPrintln(int(packetBuffer[i]));
        if (int(packetBuffer[i]) == 0 || int(packetBuffer[i]) == 10)
        {
          strReponse += '_';
        }
        else
        {
          strReponse += char(packetBuffer[i]);
        }
      };
      debugPrintln("Contenu du paquet: " + strReponse);

      // ANNALYSE DU PAQUET

      // s'il s'agit d'un paquet relatif au bouton mute
      if (strReponse.indexOf("/mix/on") > 0)
      {
        // recherche du n° de canal
        int pos1 = strReponse.indexOf("/ch/");
        if (pos1 >= 0)
        {
          String sCannal = strReponse.substring(pos1 + 4, pos1 + 6);
          // debugPrintln("canal : " + sCannal);

          // Est que le n° fait il partie des canaux suivis ?
          if (sCannal.toInt() <= nbMicro)
          {
            // dans ce cas on recupère la valeur du 'mute' (4 derniers octet converti en int)
            BytesToInt converter;

            converter.bytes[0] = packetBuffer[len - 1];
            converter.bytes[1] = packetBuffer[len - 2];
            converter.bytes[2] = packetBuffer[len - 3];
            converter.bytes[3] = packetBuffer[len - 4];

            micro[sCannal.toInt() - 1]->mute = (converter.value == 0);
            micro[sCannal.toInt() - 1]->muteUpToDate = true;
          }
          else
          {
            debugPrintln("n° canal" + sCannal + " hors limite ");
          }
        }
      }

      // s'il s'agit d'un paquet relatif au bouton fader
      else if (strReponse.indexOf("/mix/fader") > 0)
      {
        int pos1 = strReponse.indexOf("/ch/");
        if (pos1 >= 0)
        {
          // recherche du n° de canal
          String sCannal = strReponse.substring(pos1 + 4, pos1 + 6);
          // debugPrintln("canal : " + sCannal);
          if (sCannal.toInt() <= nbMicro)
          {
            micro[sCannal.toInt() - 1]->updateFader();
            BytesToFloat converter;

            converter.bytes[0] = packetBuffer[len - 1];
            converter.bytes[1] = packetBuffer[len - 2];
            converter.bytes[2] = packetBuffer[len - 3];
            converter.bytes[3] = packetBuffer[len - 4];

            micro[sCannal.toInt() - 1]->faderUpToDate = true;
            micro[sCannal.toInt() - 1]->fader = converter.value;
          }
        }
      }
    }
  }

 
  // envoie de la commande Xremote pour rester abonner aux modif de la console
  if (millis() > dernierInstantEnvoieCommande + 9000)
  {
    envoiCommande("/xremote");
   //envoiCommande("/formatsubscribe ,ssiii /testme /ch/**/mix/on 6 9 80");
      dernierInstantEnvoieCommande = millis();
  }

  // Affichage de l'état de tous les micros toutes les 3s
  if (millis() > dernierInstantAffichageEtat + 3000)
  {
    for (size_t i = 0; i < nbMicro; i++)
    {
      debugPrintln(micro[i]->etatToStr());
    };

    dernierInstantAffichageEtat = millis();
  };

  // Actualisation des LED toutes les 500ms
  if (millis() > dernierInstantAffichageLED + 500)
  {
    afficheLed();
  }
  // Serial.print('.');
}