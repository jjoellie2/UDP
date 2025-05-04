Maak een UDP-server "Wie is het dichtste ?" die een reeks nummers in ASCII verwacht en de dichtste gok als winnaar aanduidt.

Vraag 1
UDP SOCKETS correct : het aanmaken, verbinden en opruimen van sockets
Vraag 2
CLIENT to SERVER COMMUNICATION correct : UDP-client stuurt correcte integers in ASCII en worden juist ontvangen door UDP-server en vergeleken worden het het te raden getal dat de eerste keer genereert wordt (tussen 0 en 99)
Vraag 3
TIME-OUT correct : UDP-server maakt gebruik van een time-out om alle gokken te ontvangen en pas bij time-out de mogelijke winnaar aanduidt, UDP-client maakt gebruik van een time-out om te bepalen of het mogelijks gewonnen of verloren heeft.
Vraag 4
DYNAMIC TIME-OUT correct : UDP-server halveert de time-out per gok dat binnenkomt.
Vraag 5
SERVER to CLIENT COMMUNICATION correct : enkel de dichtste gok en gokker worden genomineerd voor te winnen door het bericht "You Won ?", bij het wegblijven van een bericht, print de UDP-client "You lost ?"
Vraag 6
LATE MESSAGES correct : UDP-server stuurt "You lost !" op elk bericht dat binnenkomt gedurende bepaalde tijd (inclusief time-out) nadat de mogelijke winnaar is geselecteerd. Indien dit dezelfde client is die genomineerd is, zal deze toch niet winnen.
Vraag 7
WINNER correct : UDP-server stuurt correcte moment en naar juiste UDP-client "You won !" die enkel binnen tijd en als eerste de dichtste gok heeft gedaan.
Vraag 8
CONTINOUS correct : UDP-server en UDP-client moeten niet herstarten om een volgende ronde te kunnen spelen en er wordt een nieuw random nummer gekozen.
Vraag 9
CLEAN CODE : (Eens alles juist werkt) UDP-server en UDP-client code zijn volgens industriestandaard geschreven in handelbare functies en geen globale variabelen.
Vraag 10
EXTRA : (Eens alles juist werkt) bijvoorbeeld : high-score met IP en poort, logging to files met IP en poort, gaming AI-client, ...
