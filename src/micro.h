#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>

#define X32_IP "192.168.50.25" // adresse IP locale de la console X32 à domicile
// #define X32_IP "192.168.1.11" // adresse IP locale de la console X32 chez Radio Albiges

#define X32_PORT 10023    // port de communication de la console
#define RECEIVE_PORT 1024 // port de communication de l'ESP

WiFiUDP udp; // connexion UDP avec la console X32
#define MAX_BUFFER 255
char packetBuffer[MAX_BUFFER]; // buffer pour stoker les réponses de la console

//*******************************************************************************************************************
// Option de compilation pour faciliter le debugage

#define DEBUG_MICRO

#if defined(DEBUG_MICRO)
#define debug1Print(...)                   \
    {                                      \
        Serial.print(String(__LINE__));    \
        Serial.print(": ");                \
        Serial.print(String(__VA_ARGS__)); \
        Serial.print(" ");                 \
    }

#define debug1Println(...)                   \
    {                                        \
        Serial.print(String(__LINE__));      \
        Serial.print(": ");                  \
        Serial.println(String(__VA_ARGS__)); \
    }

#else
#define debug1Print(...)
#define debug1Println(...)
#endif

union BytesToFloat
{
    unsigned char bytes[4];
    float value;
};


union BytesToInt
{
    unsigned char bytes[4];
    int value;
};

//*******************************************************************************************************************
// Procédure qui envoie une commande à la X32

void envoiCommande(String commande)
{
    //debug1Println("Envoi de la commande : " + commande);
    // Envoi du message
    OSCMessage mess(commande.c_str()); // il faut appeler le constructeur avec le message sinon il y a un bug...
    udp.beginPacket(X32_IP, X32_PORT);
    mess.send(udp);
    udp.endPacket();
}

//*******************************************************************************************************************
// les differents états que peut avoir le micro de la platine
enum EtatMicro
{
    actif,
    inactif,
    inconnu
};

//*******************************************************************************************************************
// Objet pour la gestion des micros de la console
class Micro
{
public:
    // constructeur
    Micro(int numero_canal);

    // Numéro de canal du micro
    int canal;

    // Variable pour gérer l'état actuel du micro
    bool mute;         // true si mute est on, c'est dire micro inactif
    bool muteUpToDate; // true si la valeur de mute correspond à celui de la console (=a jour)

    float fader;        // etat du curseur de fader
    bool faderUpToDate; // true si la valeur de fader correspond à celui de la console (=a jour)

    // fonction qui indique l'état du micro
    EtatMicro etat();

    // Fonction qui renvoit l'état du micro sous forme d'une chaine
    String etatToStr();

    // Procédure qui interroge la console X32 pour mettre à jour l'état mute du micro
    void updateMute();

    // Procédure qui interroge la console X32 pour mettre à jour l'état fader du micro
    void updateFader();
};

Micro::Micro(int numero_canal)
{
    this->canal = numero_canal;
    this->muteUpToDate = false;
    this->faderUpToDate = false;
};

EtatMicro Micro::etat()
{
    if (this->mute and muteUpToDate)
        return inactif;

    if (faderUpToDate)
    {
        if (fader < 0.00001)
            return inactif;
        else
            return actif;
    }
    return inconnu;
};

String Micro::etatToStr()
{
    switch (this->etat())
    {
    case actif:
        return "Etat micro " + String(this->canal) + " ACTIF";
        break;
    case inactif:
        return "Etat micro " + String(this->canal) + " non actif";
        break;
    case inconnu:
        return "Etat micro " + String(this->canal) + " inconnu";
        break;

    default:
        return "Erreur";
        break;
    }
};

void Micro::updateMute()
{
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        debug1Println("Erreur : Pas de connexion WiFi !"); // impossible d'avoir la mise a jour du micro
        this->muteUpToDate = false;
        return;
    };

    // on converti le n° de canal en String avec un zero devant pour avoir les 2 digits nécessaires
    String numCanal;

    if (this->canal <= 9)
        numCanal = "0" + String(this->canal);
    else
        numCanal = String(this->canal);

    // on interroge la console pour savoir sir le micro est mute
    String commande = "/ch/" + numCanal + "/mix/on";

    envoiCommande(commande);
    // Attente du traitement par la X32
    delay(5);

    // attente de la réponse de la console pensant 2000ms

    int maintenant = millis();
    int packetSize = udp.parsePacket();

    bool reponseRecue = (packetSize != 0);
    while (millis() < maintenant + 2000 and not(reponseRecue))
    {
        packetSize = udp.parsePacket();
        reponseRecue = (packetSize != 0);
        delay(5);
    };

    if (not(reponseRecue))
    {
        debug1Println(" Erreur : Pas de réponse <mute> de la console ");
        this->muteUpToDate = false;
    }
    else
    {
        //debug1Println(" Paquet reçu de : " + udp.remoteIP().toString());
        //debug1Println(" Taille : " + String(packetSize));

        if (packetSize > MAX_BUFFER)
        {
            debug1Println(" ERREUR <mute>: Taille du paquet supérieure à celle du buffer !");
            this->muteUpToDate = false;
        }
        else
        {
            int len = udp.read(packetBuffer, packetSize);
            //debug1Println("len:" + String(len));
            //if (len > 0)
             //   packetBuffer[len - 1] = 0;
            // stockage de la réponse dans une chaine de caractère en remplaçant les caractères spéciaux par espace
            String strReponse = "";
            for (size_t i = 0; i < len; i++)
            {
                //debug1Println(packetBuffer[i]);
                //debug1Println("valeur n°"+String(i+1)+":"+String((packetBuffer[i])));
                
                if (int(packetBuffer[i]) == 0 || int(packetBuffer[i]) == 10)
                {
                    strReponse += '_';
                }
                else
                {
                    strReponse += char(packetBuffer[i]);
                }
            };

            //debug1Println("Reponse <mute> de la X32:" + strReponse);

            // on verifie que la reponse correspond à notre demande sinon erreur dans les paquets
            if (strReponse.indexOf("/ch/" + numCanal + "/mix/on") < 0)
            {
                this->muteUpToDate = false;
                debug1Println("Reponse <mute> incorrecte");
            }
            else
            { 
                                
                BytesToInt converter;

                converter.bytes[0] = packetBuffer[len - 1];
                converter.bytes[1] = packetBuffer[len - 2];
                converter.bytes[2] = packetBuffer[len - 3];
                converter.bytes[3] = packetBuffer[len - 4];
                //debug1Println("Mute lu:"+String(converter.value));

                this->muteUpToDate = true;
                this->mute = (converter.value==0);
                //debug1Println(" Le mute est de " + String(this->mute));
        
                
            }
        }
    }
}

void Micro::updateFader()
{
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        debug1Println("Erreur : Pas de connexion WiFi !"); // impossible d'avoir la mise a jour du micro
        this->faderUpToDate = false;
        return;
    };

    // on converti le n° de canal en String avec un zero devant pour avoir les 2 digits nécessaires
    String numCanal;

    if (this->canal <= 9)
        numCanal = "0" + String(this->canal);
    else
        numCanal = String(this->canal);

    // on interroge la console pour recuperer la valeur du fader
    // String commande = "/node   ,s  ch/" + numCanal + "/mix/fader";
    String commande = "/ch/" + numCanal + "/mix/fader";

    envoiCommande(commande);

    // Attente du traitement par la X32
    delay(5);

    // attente de la réponse de la console pensant 2000ms

    int maintenant = millis();
    int packetSize = udp.parsePacket();

    bool reponseRecue = (packetSize != 0);
    while (millis() < maintenant + 2000 and not(reponseRecue))
    {
        packetSize = udp.parsePacket();
        reponseRecue = (packetSize != 0);
        delay(5);
    };

    if (not(reponseRecue))
    {
        debug1Println(" Erreur : Pas de réponse <fader> de la console ");
        this->faderUpToDate = false;
    }
    else
    {
        //debug1Println(" Paquet reçu de : " + udp.remoteIP().toString());
        //debug1Println(" Taille : " + String(packetSize));

        if (packetSize > MAX_BUFFER)
        {
            debug1Println(" ERREUR <fader> : Taille du paquet supérieure à celle du buffer !");
            this->faderUpToDate = false;
        }
        else
        {
            int len = udp.read(packetBuffer, packetSize);
            //debug1Println("len:" + String(len));
            // stockage de la réponse dans une chaine de caractère en remplaçant les caractères spéciaux par espace
            String strReponse = "";
            for (size_t i = 0; i < len; i++)
            {
                //debug1Println(packetBuffer[i]);
                //debug1Println(int(packetBuffer[i]));
                if (int(packetBuffer[i]) == 0 || int(packetBuffer[i]) == 10)
                {
                    strReponse += '_';
                }
                else
                {
                    strReponse += char(packetBuffer[i]);
                }
            };
            //debug1Println("reponse <fader> de la X32:" + strReponse);

            // traitement de la reponse <fader>
            if (strReponse.indexOf("/ch/" + numCanal + "/mix/fader") < 0)
            {
                debug1Println("Reponse <fader> incorrecte");
                this->faderUpToDate = false;
            }
            else
            {
                //la valeur du fader est le float stocké dans les 4 derniers octets du message
                BytesToFloat converter;

                converter.bytes[0] = packetBuffer[len - 1];
                converter.bytes[1] = packetBuffer[len - 2];
                converter.bytes[2] = packetBuffer[len - 3];
                converter.bytes[3] = packetBuffer[len - 4];

                this->faderUpToDate = true;
                this->fader = converter.value;
                //debug1Println(" Le fader est de " + String(this->fader));
            }
        }
    }
}
