# TP4_MINF
## Explication complementaire gestion memoire de l'EEPROM
# Initialisation du bus I2C

## Fonction

```C
void I2C_InitMCP79411(void)
{
   bool Fast = true;
   i2c_init( Fast );
}
```

---

## Explication

Cette fonction initialise la communication I2C.

```C
bool Fast = true;
```

Cette variable permet de demander une initialisation en mode rapide.

Ensuite :

```C
i2c_init(Fast);
```

configure le module I2C du microcontrôleur.

---

# Écriture dans l'EEPROM

## Fonction complète

```C
void I2C_WriteSEEPROM(void *SrcData, uint32_t EEpromAddr, uint16_t NbBytes)
```

---

## Paramètres

| Paramètre  | Description                     |
| ---------- | ------------------------------- |
| SrcData    | Adresse des données à écrire    |
| EEpromAddr | Adresse de départ dans l'EEPROM |
| NbBytes    | Nombre d'octets à écrire        |

---

# Boucle principale d'écriture

```C
for(y = 0; y < NbBytes;)
```

Cette boucle continue tant que tous les octets n'ont pas été écrits.

Le compteur n'est pas incrémenté dans le `for`.

L'incrémentation est faite plus loin avec :

```C
y += NbBytesPage;
```

car le programme avance page par page.

---

# Calcul de l'adresse courante

```C
current_addr = EEpromAddr + y;
```

Cette ligne permet de connaître l'adresse exacte où écrire les prochaines données.

---

## Exemple

| Adresse de départ | y  | Adresse actuelle |
| ----------------- | -- | ---------------- |
| 32                | 0  | 32               |
| 32                | 8  | 40               |
| 32                | 16 | 48               |

---

# Gestion des pages mémoire

## Calcul de l'espace restant

```C
espaceRestant = MCP79411_PAGE_SIZE - (current_addr % MCP79411_PAGE_SIZE);
```

L'EEPROM fonctionne par pages mémoire.

Il est donc important de ne pas écrire au-delà d'une page.

Cette ligne calcule le nombre d'octets encore disponibles avant la fin de la page actuelle.

---

## Exemple

Si :

| Taille page | Adresse actuelle |
| ----------- | ---------------- |
| 8           | 14               |

Alors :

```C
14 % 8 = 6
```

Le programme est au 6ème octet de la page.

Il reste donc :

```C
8 - 6 = 2 octets
```

avant de changer de page.

---

# Choix du nombre d'octets à écrire

```C
if((NbBytes - y) < espaceRestant)
{
    NbBytesPage = NbBytes - y;
}
else
{
    NbBytesPage = espaceRestant;
}
```

Cette partie permet de choisir combien d'octets peuvent être écrits sans dépasser la page.

---

## Deux cas possibles

| Situation                                         | Action                                                  |
| ------------------------------------------------- | ------------------------------------------------------- |
| Il reste moins de données que la place disponible | Toutes les données sont écrites                         |
| Il reste trop de données                          | Le programme écrit uniquement ce qui tient dans la page |

---

# Attente de disponibilité du composant

```C
do
{
    i2c_start();
    ack = i2c_write(MCP79411_EEPROM_W);

    if(ack == false)
    {
        i2c_stop();
    }

    timeout++;

    if(timeout > 1000)
    {
        return;
    }

} while (ack == false);
```

Après une écriture, l'EEPROM peut être occupée pendant quelques millisecondes.

Pendant ce temps, elle refuse de communiquer.

Le programme tente donc de communiquer jusqu'à ce que le composant réponde correctement.

---

# Envoi de l'adresse mémoire

```C
i2c_write((uint8_t)current_addr);
```

Cette instruction envoie l'adresse interne de l'EEPROM.

---

# Envoi des données

```C
for(i = 0; i < NbBytesPage; i++)
{
    i2c_write(pointeur[y + i]);
}
```

Cette boucle envoie tous les octets de la page.

---

## Exemple

Si :

```C
y = 8
```

Alors :

```C
pointeur[8]
pointeur[9]
pointeur[10]
```

seront envoyés.

---

# Fin de transmission

```C
i2c_stop();
```

Cette instruction termine la communication I2C et libère le bus.

---

# Passage à la page suivante

```C
y += NbBytesPage;
```

Permet de passer directement au bloc suivant.

---

# Lecture dans l'EEPROM

## Fonction complète

```C
void I2C_ReadSEEPROM(void *DstData, uint32_t EEpromAddr, uint16_t NbBytes)
```

---

# Vérification de sécurité

```C
if(NbBytes == 0)
{
    return;
}
```

Si aucun octet n'est demandé, la fonction quitte immédiatement.

---

# Vérification de disponibilité

Le programme attend que l'EEPROM soit disponible avant de commencer la lecture.

Le fonctionnement est identique à celui utilisé dans la fonction d'écriture.

---

# Envoi de l'adresse à lire

```C
i2c_write(EEpromAddr);
```

Permet d'indiquer au composant l'adresse de départ de lecture.

---

# Restart I2C

```C
i2c_reStart();
```

Le restart permet de conserver le contrôle du bus sans envoyer de STOP.

Cela permet :

1. d'envoyer l'adresse mémoire,
2. de changer en mode lecture,
3. de commencer la réception des données.

---

# Passage en mode lecture

```C
i2c_write(MCP79411_EEPROM_R);
```

Le composant passe en mode lecture.

---

# Lecture des données

```C
for(y = 0; y < NbBytes; y++)
```

Boucle principale de lecture.

---

# Gestion des ACK et NACK

## Cas normal

```C
pointeur[y] = i2c_read(true);
```

Le microcontrôleur envoie un ACK.

Cela signifie :

```text
Je veux encore des données
```

---

## Dernier octet

```C
pointeur[y] = i2c_read(false);
```

Le microcontrôleur envoie un NACK.

Cela signifie :

```text
Lecture terminée
```

Cette étape est obligatoire dans le protocole I2C.

---

# Fin de lecture

```C
i2c_stop();
```

Termine la communication et libère le bus.

---

# Correction importante apportée

## Problème détecté

Dans la première version du code :

```C
*pointeur = i2c_read(true);
```

le pointeur n'était jamais incrémenté.

Le programme écrivait donc toujours dans la même case mémoire.

Résultat :

seul le dernier octet lu était conservé.

---

## Correction

La lecture correcte est :

```C
pointeur[y] = i2c_read(true);
```

et :

```C
pointeur[y] = i2c_read(false);
```

Cette correction permet de stocker chaque octet dans une position différente du tableau.

---

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
