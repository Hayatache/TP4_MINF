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
        }
```

#### Important !
Nous devons faire "+2" pour trouver le debut d'une valeur car la trame etant xxxF=3000xxxx, nous trouvons la valuer de F a l'adresse 3 et le debut du 3000 a l'adresse 5.
Autre information importante, nous devons avoir une verification pour trouver si un "S" a été vu car le sinus etant defini aussi par "S", il sera detecter et supprimer ce qui nous fera perdre l'information du sinus.

### 3. Optention des informations grace a atoi
la fonction atoi est une fonction permettant de transformer une chaine de caractere en int.
en lui donnant une chaine de caractere la fonction va du debut de la chaine jusqu'au caractere de fin de chaine de caractere "/0", mais il est aussi possible de faire commencer la lecture au milieu d'une chaine donné.

comme ci dessus, nous commencons la lecture de l'amplitude au debut de l'amplitude et la fonction va lire jusqu'au caractere de fin mis a la place du debut de la trame de l'offset.

![black](https://img.shields.io/badge/!/O=D/O=1000/O=&shy;-black?style=for-the-badge) ![red](https://img.shields.io/badge/1000/O-red?style=for-the-badge) ![black](https://img.shields.io/badge/&shy;=-500/O=0%23-black?style=for-the-badge)

ce qui fait que nous lirons uniquement la valeur qui nous interesse.
