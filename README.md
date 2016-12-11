# BattagliaNavale
A simple net game developed in C using socket TCP/UDP

# Prossime cose da fare
- Cominciare a pensare ad un modo per rendere solido il sistema (gestione input utente)
- Cominciare a fare i timeout (socket udp e input in pvp)

# TODO
- Verificare se in fase di chiusura del client ci siano strutture dati da eliminare
- Terminazione server (?)
- verificare i parametri (ip porta)

- GESTIONE ERRORI SULLE VARIE FUNZIONI DEL SOCKET (ORA SE SUCCEDE QUALCOSA MUORE TUTTO MALISSIMO)
- GESTIONE TIME OUT SU TUTTO
- RIVEDERE TUTTE LE RECV
- GESTIRE DISCONNESSIONE IMPROVVISA utente
- GESTIRE DISCONNESSIONE IMPROVVISA SERVER (?)
- (RISCRIVERE PARTE SERVER SELECT() USANDO LA MIA LIB???ANKENO)

# Cose che penso di aver fatto
- disconnect udp funziona (forse non è la cosa più del mondo ma va)
- Ora è possibile fare una partita con un altro client, non ci sono protezioni varie
- Scritta una piccola lib UDP
- Inizializzazione della fase di gioco (solo in locale) (ovviamente va rivista e mancano i timeout ecc.)
- Server (salvo TIMEOUT e gestione client che muore)
- Interazione Client-Server (TCP)
- Gestione stream IO/Socket (senza polling)
- Gestione comandi su ambiente MENU/GAME
- Casino
