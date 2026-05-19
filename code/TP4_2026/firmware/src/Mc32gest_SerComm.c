// Mc32Gest_SerComm.C
// fonction d'émission et de réception des message
// transmis en USB CDC
// Canevas TP4 SLO2 2015-2015


#include "app.h"
#include "app_generator.h"
#include "Mc32gest_SerComm.h"
#include "Mc32DriverLcd.h"
#include "MenuGen.h"
#include "Mc32gestI2cSeeprom.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern APP_GENERATOR_DATA app_generatorData;
extern APP_DATA appData;
// Fonction de reception  d'un  message
// Met à jour les paramètres du generateur a partir du message recu
// Format du message
//  !S=TF=2000A=10000O=+5000D=100W=0#
//  !S=PF=2000A=10000O=-5000D=100W=1#

 S_Receive_error_type Get_message_error;
 
void Init_error_type(void){
    Get_message_error.Amp_too_big = 0;
    Get_message_error.Amp_too_small = 0;
    Get_message_error.Unknown_signal_type = 0;
    Get_message_error.frequency_too_high = 0;
    Get_message_error.frequency_too_low = 0;
    Get_message_error.off_too_big = 0;
    Get_message_error.off_too_small = 0;
}
// Fonction de lecture et de décodage d'un message USB
// ---------------------------------------------------
// Analyse une trame reçue depuis USB et extrait les paramètres
// du générateur de signal.
//
// Format attendu :
// !S=TF=2000A=10000O=+5000W=1#
//
// S = forme du signal
// F = fréquence
// A = amplitude
// O = offset
// W = sauvegarde
//
bool GetMessage(int8_t *USBReadBuffer, S_ParamGen *pParam, bool *SaveTodo)
{
    // Variable utilisée pour parcourir le buffer
    uint8_t index;

    // Indique si la trame est valide
    uint8_t validation = 0;

    // Position de début des différents paramètres dans la trame
    uint8_t start_signal= 0;
    uint8_t start_frequence= 0;
    uint8_t start_amplitude= 0;
    uint8_t start_offset= 0;

    // Non utilisé actuellement
    uint8_t offset_signe= 0;

    // Position du paramètre de sauvegarde
    uint8_t start_sauvegarde= 0;

    // Permet de détecter uniquement le premier 'S'
    bool first_S_found = false;

    // Variable temporaire utilisée avec atoi()
    int32_t var = 0;

    // Buffer inutilisé actuellement
    char buffer[10];

    // ------------------------------------------------
    // Lecture et première validation de la trame
    // ------------------------------------------------

    // Vérifie si la trame commence bien par '!'
    if(USBReadBuffer[0] =='!'){

        // Recherche du caractère de fin '#'
        for(index = 0; index <= appData.numBytesRead; index++){

            // Si '#' est trouvé alors la trame est valide
            if(USBReadBuffer[index] == '#'){
                validation = 1;
            }
        }
    }
    else{

        // Affichage d'une erreur si le début est invalide
        lcd_gotoxy(1,4);
        printf_lcd("trame invalide debut");
    }

    // ------------------------------------------------
    // Décodage des paramètres si la trame est valide
    // ------------------------------------------------
    if(validation == 1){

        // Remise à zéro de la variable de validation
        validation = 0;

        // Recherche des différents champs dans la trame
        for(index = 0; index <= appData.numBytesRead; index++){

            // Détection du champ signal
            if(USBReadBuffer[index] == 'S' && first_S_found == false){

                // Position du contenu après "S="
                start_signal = index +2;

                // Empêche la détection d'un second 'S'
                first_S_found = true;

                // Coupe la chaîne précédente
                USBReadBuffer[index] = '\0';
            }

            // Détection du champ fréquence
            else if(USBReadBuffer[index] == 'F'){

                start_frequence = index +2;

                // Coupe la chaîne précédente
                USBReadBuffer[index] = '\0';
            }

            // Détection du champ amplitude
            else if(USBReadBuffer[index] == 'A'){

                start_amplitude = index +2;

                // Coupe la chaîne précédente
                USBReadBuffer[index] = '\0';
            }

            // Détection du champ offset
            else if(USBReadBuffer[index] == 'O'){

                start_offset = index +2;

                // Coupe la chaîne précédente
                USBReadBuffer[index] = '\0';
            }

            // Détection du champ sauvegarde
            else if(USBReadBuffer[index] == 'W'){

                start_sauvegarde = index +2;

                // Coupe la chaîne précédente
                USBReadBuffer[index] = '\0';
            }
        }

        // ------------------------------------------------
        // Décodage de la forme du signal
        // ------------------------------------------------
        switch (USBReadBuffer[start_signal]) {

            // Triangle
            case 'T':
                pParam->Forme = 1;
                break;

            // Sinus
            case 'S':
                pParam->Forme = 0;
                break;

            // Carré
            case 'C':
                pParam->Forme = 3;
                break;

            // Dent de scie
            case 'D':
                pParam->Forme = 2;
                break;

            // Type inconnu
            default:
                Get_message_error.Unknown_signal_type = 1;
                break;
        }

        // ------------------------------------------------
        // Lecture et validation de la fréquence
        // ------------------------------------------------

        // Conversion ASCII -> entier
        var = atoi(&USBReadBuffer[start_frequence]);

        // Vérifie la limite maximale
        if(var > 2000){

            // Saturation à 2000 Hz
            pParam->Frequence = 2000 ;

            // Activation du flag d'erreur
            Get_message_error.frequency_too_high = 1;
        }

        // Vérifie la limite minimale
        else if(var < 20){

            // Saturation à 20 Hz
            pParam->Frequence = 20 ;

            // Activation du flag d'erreur
            Get_message_error.frequency_too_low = 1;
        }

        // Valeur valide
        else{
            pParam->Frequence = var;
        }

        // ------------------------------------------------
        // Lecture et validation de l'amplitude
        // ------------------------------------------------

        // Conversion ASCII -> entier
        var = atoi(&USBReadBuffer[start_amplitude]);

        // Vérifie la limite maximale
        if(var > 10000){

            // Saturation à 10000
            pParam->Amplitude = 10000 ;

            // Activation du flag d'erreur
            Get_message_error.Amp_too_big = 1;
        }

        // Vérifie la limite minimale
        else if(var < 100){

            // Saturation à 100
            pParam->Amplitude = 100 ;

            // Activation du flag d'erreur
            Get_message_error.Amp_too_small = 1;
        }

        // Valeur valide
        else{
            pParam->Amplitude = var;
        }

        // ------------------------------------------------
        // Lecture et validation de l'offset
        // ------------------------------------------------

        // Conversion ASCII -> entier
        var = atoi(&USBReadBuffer[start_offset]);

        // Vérifie la limite maximale
        if(var > 5000){

            // Saturation à +5000
            pParam->Offset = 5000 ;

            // Activation du flag d'erreur
            Get_message_error.off_too_big = 1;
        }

        // Vérifie la limite minimale
        else if(var < -5000){

            // Saturation à -5000
            pParam->Offset = -5000 ;

            // Activation du flag d'erreur
            Get_message_error.off_too_small = 1;
        }

        // Valeur valide
        else{
            pParam->Offset = var;
        }

        // ------------------------------------------------
        // Lecture du paramètre de sauvegarde
        // ------------------------------------------------

        // Conversion ASCII -> entier
        var = atoi(&USBReadBuffer[start_sauvegarde]);

        // Si 0 alors pas de sauvegarde
        if(var == 0){

            *SaveTodo = false;
        }

        // Sinon sauvegarde demandée
        else{

            *SaveTodo = true;

            // Sauvegarde EEPROM désactivée actuellement
            //I2C_WriteSEEPROM((uint32_t*)pParam,MCP79411_EEPROM_BEG, sizeof(S_ParamGen));

            // Demande de confirmation de sauvegarde
            MENU_DemandeSave();
        }

        // Gestion des erreurs désactivée actuellement
        //Error_manager();
    }
    else{

        // Affichage erreur si le caractère de fin manque
        lcd_gotoxy(1,4);
        printf_lcd("trame invalide fin");
    }

    // Retour toujours vrai
    return 1;

} // GetMessage


// --------------------------------------------------------
// Fonction d'envoi d'un message USB
// --------------------------------------------------------
// Remplit le buffer d'émission USB en fonction des paramètres
// du générateur.
//
// Format envoyé :
// !S=TF=2000A=10000O=+5000D=25WP=0#
//
// ACK sauvegarde :
// !S=TF=2000A=10000O=+5000D=25WP=1#
//
void SendMessage(int8_t *USBSendBuffer, S_ParamGen *pParam, bool Saved )
{
    // Variable de parcours du buffer
    uint8_t index;

    // Position du champ sauvegarde
    uint8_t start_sauvegarde= 0;

    // Variable inutilisée actuellement
    int32_t var = 0;

    // Recherche du caractère 'W'
    for(index = 0; index <= appData.numBytesRead; index++){

        if(USBSendBuffer[index] == 'W'){

            // Sauvegarde de la position
            start_sauvegarde = index ;
        }
    }

    // Décalage des caractères pour insérer 'P'
    USBSendBuffer[start_sauvegarde+4] = USBSendBuffer[start_sauvegarde+3];
    USBSendBuffer[start_sauvegarde+3] = USBSendBuffer[start_sauvegarde+2];
    USBSendBuffer[start_sauvegarde+2] = USBSendBuffer[start_sauvegarde+1];

    // Insertion du caractère 'P'
    USBSendBuffer[start_sauvegarde+1] = 'P';

    // Envoi du buffer via USB CDC
    USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                        &appData.writeTransferHandle,
                        USBSendBuffer, appData.numBytesRead+1,
                        USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);

} // SendMessage

void Error_manager(void) {
    // Déclaration du buffer. 
    // 256 octets permettent de stocker plusieurs messages d'erreur à la suite.
    char USBSendBuffer[256]; 
    int msg_len = 0;

    // Initialisation du buffer
    USBSendBuffer[0] = '\0';

    // Construction du message avec des retours à la ligne (\n)
    if (Get_message_error.Unknown_signal_type) {
        msg_len += snprintf(USBSendBuffer + msg_len, sizeof(USBSendBuffer) - msg_len, "ERR: Type de signal inconnu\n");
    }
    if (Get_message_error.Amp_too_big) {
        msg_len += snprintf(USBSendBuffer + msg_len, sizeof(USBSendBuffer) - msg_len, "ERR: Amplitude trop grande\n");
    }
    if (Get_message_error.Amp_too_small) {
        msg_len += snprintf(USBSendBuffer + msg_len, sizeof(USBSendBuffer) - msg_len, "ERR: Amplitude trop petite\n");
    }
    if (Get_message_error.off_too_big) {
        msg_len += snprintf(USBSendBuffer + msg_len, sizeof(USBSendBuffer) - msg_len, "ERR: Offset trop grand\n");
    }
    if (Get_message_error.off_too_small) {
        msg_len += snprintf(USBSendBuffer + msg_len, sizeof(USBSendBuffer) - msg_len, "ERR: Offset trop petit\n");
    }
    if (Get_message_error.frequency_too_high) {
        msg_len += snprintf(USBSendBuffer + msg_len, sizeof(USBSendBuffer) - msg_len, "ERR: Frequence trop haute\n");
    }
    if (Get_message_error.frequency_too_low) {
        msg_len += snprintf(USBSendBuffer + msg_len, sizeof(USBSendBuffer) - msg_len, "ERR: Frequence trop basse\n");
    }

    // Envoi si au moins une erreur a été détectée
    if (msg_len > 0) {
        USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                             &appData.writeTransferHandle,
                             USBSendBuffer, 
                             msg_len,
                             USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
    }
    
    Get_message_error.Amp_too_big = 0;
    Get_message_error.Amp_too_small = 0;
    Get_message_error.Unknown_signal_type = 0;
    Get_message_error.frequency_too_high = 0;
    Get_message_error.frequency_too_low = 0;
    Get_message_error.off_too_big = 0;
    Get_message_error.off_too_small = 0;
}
