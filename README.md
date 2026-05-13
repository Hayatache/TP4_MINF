# TP4_MINF
## explication complementaire gestion memoire de l'EEPROM

## explication complementaire decodage de la trame

le decodage se faire en plusieurs etape pouvant mener a une detection d'erreur.
### 1. reperage du debut et fin de trame
dans un premiere temps,
``` C
if(USBReadBuffer[0] =='!')
```
va regarder si il y a bien le debut de la trame qui est un "!".
ensuite, 
``` C
for(index = 0; index <= appData.numBytesRead; index++){
    if(USBReadBuffer[index] == '#'){
        validation = 1;
    }
}
```
nous allons scanner la trame complete pour trouver si il y a le caractere de fin de trame qui est "#".
si c'est le cas, la trame est conciderer comme valide pour la prochaine etape
### 2. Reperage des debuts de chaque information
etand donné que nous devons trouvé les infos suivantes : 

| information dans la trame | caractere qui defini l'information |
|------|------|
| formes | S |
| frequence  | F |
| amplitude  | A | 
| offset  | O |
| sauvegarde  | W | 
