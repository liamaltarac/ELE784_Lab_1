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
Pour tester le pilote, nous avons écrit une application usager. Cet 

Pour la lancer, 


## License
[Dual BSD/GPL]
