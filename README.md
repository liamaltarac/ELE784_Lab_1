# ELE784_Lab_1

Pilot Linux d'un port serie pc16550d
## Installation

Pour telecharger le code, faite:

```
git clone https://github.com/liamaltarac/ELE784_Lab_1
```
Ensiute, pour l'installer faites la commande


```
 setserial -ga /dev/ttyS[56]
```
Et prennez en note les numeros de ports et IRQs des deux lignes.
Maintenant, vous pouvez installer le pilate avec la commande suivante :

```
sudo insmod SerialDriver.ko Port0Addr=[Port0Addr] Port1Addr=[Port1Addr] Port0IRQ=[Port0IRQ] Port1IRQ=[Port1IRQ]
```
(Remplacez les [PortXYX] par les valeurs de retour de la commande ```setserial```)

## Usage
Pour tester le pilote, nous avons écrit une application usager. 

Cet application peut etre executé en 3 modes.
- Read (Blocking)
- Write(Blocking)
- Read/Write (Non-blocking)

Voici les arguments pour executer l'application:


```
./SerialTest <noeud> {-R|-W|-RW} [-d donnnées] [-s taille] -c -? [Actions IOCTL.]
```
Voici trois examples pour lancer l'application dans les mode mentionnes ci-decu.

### Mode Read:
```
./SerialTest /dev/SerialDev0 r-s 10  
```
### Mode Write:
```
./SerialTest /dev/SerialDev0 w -d "123 texte bidon 321"
```

### Mode Read / Write:
```
./SerialTest /dev/SerialDev0 rw 
```
Voici la liste des parameters avec un leur description. 

 | Argument      | Description  |
| ------------- |:-------------:|
| noeud | **Premier argument.** Le noeud de que nous allons interagir avec (par exemple : /dev/SerialDev1)     |
| -R    ou -r         | Pour lancer l'application en mode Read |
| -W ou -w     | Pour lancer l'application en mode Write      | 
| -RW ou -rw stripes | Pour lancer l'application en mode Read / Write.  **Prend seulement le noeud comme argument et les Actions IOCTL.**   |
| -d | **Seuelement en mode Write.**  Les donnees ques nous voulons envoyer|
| -s | **Seuelement en mode Read.** Le nombre de donnees que nous voulons lire|
| -c | Mode conitue.|
| -? | Menu *Help*.|
| --set_baud | Specifier le baud rate (Ex.: ``` --set_baud 9600 ```)|
| --set_parity | Specifier la parite. (0:Impaire, 1:Paire, -1:Aucune_parite) (Ex.: ``` --set_parity 2 ```)|
| --get_buf_size | Imprime la taille du tampon circulare |
| --set_buf_size | Specifier la taille du tampon circulaire (Ex.: ``` --set_buf_size 20 ```)|
| --set_data_size | Specifie la taille des donnees (5bits - 8bits) (Ex.: ``` --set_baud 8 ```)|


## License
[Dual BSD/GPL]
