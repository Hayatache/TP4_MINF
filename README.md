# TP4_MINF
## Explication complementaire gestion memoire de l'EEPROM
# Explication complementaire du fonctionnement I2C avec la SEEPROM

Le fonctionnement de la communication I2C avec la SEEPROM du MCP79411 se fait en plusieurs etapes afin de garantir une communication fiable.

---

## 1. Initialisation du bus I2C

Avant toute communication, le bus I2C doit etre initialise.

```C
bool Fast = true;
i2c_init(Fast);
```

Le mode rapide est active afin d'utiliser une frequence I2C plus elevee.

---

## 2. Verification de disponibilite de l'EEPROM

Avant chaque lecture ou ecriture, le programme attend que l'EEPROM soit disponible.

```C
do
{
    i2c_start();

} while(!i2c_write(MCP79411_EEPROM_W));
```

Le programme envoie l'adresse du composant jusqu'a recevoir un ACK.

### Important !

Cette verification est necessaire car l'EEPROM peut etre occupee pendant son ecriture interne.

Sans cela :
- certaines donnees pourraient etre perdues
- la communication pourrait devenir instable

---

## 3. Envoi de l'adresse memoire

Une fois le composant disponible, l'adresse memoire de depart est envoyee.

```C
i2c_write((uint8_t)EEpromAddr);
```

Cette adresse indique :
- ou commencer la lecture
- ou commencer l'ecriture

---

## 4. Gestion des pages EEPROM

L'EEPROM fonctionne avec des pages de taille fixe.

```C
for(pageIndex = 0; pageIndex <= (NbBytes / EEPROM_PAGE_SIZE); pageIndex++)
```

Le programme decoupe automatiquement les donnees en plusieurs pages afin de respecter la taille maximale d'ecriture.

---

## 5. Ecriture des donnees

Les donnees sont envoyees byte par byte.

```C
i2c_write(writeBuffer[byteIndex + (pageIndex * EEPROM_PAGE_SIZE)]);
```

Une fois la page envoyee :

```C
i2c_stop();
```

Le STOP indique la fin de transmission.

---

## 6. Passage en mode lecture

Pour lire les donnees, le programme passe du mode ecriture au mode lecture.

```C
i2c_reStart();
i2c_write(MCP79411_EEPROM_R);
```

Le `reStart` permet de garder le controle du bus sans couper la communication.

---

## 7. Lecture des donnees

Lecture des bytes avec ACK :

```C
readBuffer[byteIndex] = i2c_read(1);
```

Le ACK indique que la lecture continue.

Dernier byte sans ACK :

```C
readBuffer[byteIndex] = i2c_read(0);
```

Le NACK indique la fin de lecture.

---

## Resume du fonctionnement

### Ecriture

```text
START
↓
Adresse composant (Write)
↓
Adresse memoire
↓
Donnees
↓
STOP
```

### Lecture

```text
START
↓
Adresse composant (Write)
↓
Adresse memoire
↓
RESTART
↓
Adresse composant (Read)
↓
Lecture des donnees
↓
STOP
```

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

---

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

---

### 3. Optention des informations grace a atoi
la fonction atoi est une fonction permettant de transformer une chaine de caractere en int.
en lui donnant une chaine de caractere la fonction va du debut de la chaine jusqu'au caractere de fin de chaine de caractere "/0", mais il est aussi possible de faire commencer la lecture au milieu d'une chaine donné.

comme ci dessus, nous commencons la lecture de l'amplitude au debut de l'amplitude et la fonction va lire jusqu'au caractere de fin mis a la place du debut de la trame de l'offset.

![black](https://img.shields.io/badge/!/O=D/O=1000/O=&shy;-black?style=for-the-badge) ![red](https://img.shields.io/badge/1000/O-red?style=for-the-badge) ![black](https://img.shields.io/badge/&shy;=-500/O=0%23-black?style=for-the-badge)

ce qui fait que nous lirons uniquement la valeur qui nous interesse.
