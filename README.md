# TP4_MINF
## Explication complementaire gestion memoire de l'EEPROM

## Explication complementaire decodage de la trame

Le decodage se faire en plusieurs etape pouvant mener a une detection d'erreur.
### 1. Reperage du debut et fin de trame
dans un premiere temps,
``` C
if(USBReadBuffer[0] =='!')
```
Va regarder si il y a bien le debut de la trame qui est un "!".
ensuite, 
``` C
for(index = 0; index <= appData.numBytesRead; index++){
    if(USBReadBuffer[index] == '#'){
        validation = 1;
    }
}
```
Nous allons scanner la trame complete pour trouver si il y a le caractere de fin de trame qui est "#".
Si c'est le cas, la trame est conciderer comme valide pour la prochaine etape
### 2. Reperage des debuts de chaque information
Etand donné que nous devons trouvé les infos suivantes : 

| Information dans la trame | Caractere qui defini l'information |
|------|------|
| formes | S |
| frequence  | F |
| amplitude  | A | 
| offset  | O |
| sauvegarde  | W | 

Une fois que nous avons trouver un des caractere qui defini l'information, nous remplacons ce caracter par "/0" pour definir une fin de chaine.
Nous enregistrons aussi la position dans le tableau de caractere du debut de la valeur numerique.

``` C
        for(index = 0; index <= appData.numBytesRead; index++){
            if(USBReadBuffer[index] == 'S' && first_S_found == false){
                start_signal = index +2;
                first_S_found = true;
                USBReadBuffer[index] = '\0';
            }
            else if(USBReadBuffer[index] == 'F'){
                start_frequence = index +2;
                USBReadBuffer[index] = '\0';
            }
            else if(USBReadBuffer[index] == 'A'){
                start_amplitude = index +2;
                USBReadBuffer[index] = '\0';
            }
            else if(USBReadBuffer[index] == 'O'){
                start_offset = index +2;
                USBReadBuffer[index] = '\0';
            }
            else if(USBReadBuffer[index] == 'W'){
                start_sauvegarde = index +2;
                USBReadBuffer[index] = '\0';
            }
        }
```

#### Important !
Nous devons faire "+2" pour trouver le debut d'une valeur car la trame etant xxxF=3000xxxx, nous trouvons la valuer de F a l'adresse 3 et le debut du 3000 a l'adresse 5.
Autre information importante, nous devons avoir une verification pour trouver si un "S" a été vu car le sinus etant defini aussi par "S", il sera detecter et supprimer ce qui nous fera perdre l'information du sinus.
