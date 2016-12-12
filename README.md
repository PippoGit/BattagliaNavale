# BattagliaNavale
A simple net game developed in C using socket TCP/UDP

# Prossime cose da fare
- verificare gli input:
  - nome (adesso muore tutto, forse non è la cosa migliore)
  - ip (verificare se quello inserito è veramente un ip)
  - porta (verificare se quella inserita è veramente una porta)


# TODO
- Verificare se in fase di chiusura del client ci siano strutture dati da eliminare
- Terminazione server (?)

- GESTIONE ERRORI SULLE VARIE FUNZIONI DEL SOCKET (ORA SE SUCCEDE QUALCOSA MUORE TUTTO MALISSIMO)
- RIVEDERE TUTTE LE RECV
- GESTIRE DISCONNESSIONE IMPROVVISA SERVER (?)

# Cose che penso di aver fatto
- Progetto completato e funzionante, mancano test, robustezza sugli input e opportuna gestione errori su funzioni socket (12/12/2016)
- Timeout funzionanti
- Gestita disconnessione improvvisa di un utente
- disconnect udp funziona (forse non è la cosa più del mondo ma va)
- Ora è possibile fare una partita con un altro client, non ci sono protezioni varie
- Scritta una piccola lib UDP
- Inizializzazione della fase di gioco (solo in locale) (ovviamente va rivista e mancano i timeout ecc.)
- Server (salvo TIMEOUT e gestione client che muore)
- Interazione Client-Server (TCP)
- Gestione stream IO/Socket (senza polling)
- Gestione comandi su ambiente MENU/GAME
- Casino
