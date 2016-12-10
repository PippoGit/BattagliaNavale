# BattagliaNavale
A simple net game developed in C using socket TCP/UDP

# Prossime cose da fare
- Terminare fase di connect (parte UDP)
- Iniziare fase di gioco UDP

# TODO
- Verificare se in fase di chiusura del client ci siano strutture dati da eliminare
- Terminazione server (?)
- verificare i parametri (ip porta)
- Il Server muore sulla richiesta pending di sfida... (non proprio ok)

- GESTIONE TIME OUT SU TUTTO
- RIVEDERE TUTTE LE RECV
- GESTIRE DISCONNESSIONE IMPROVVISA utente

# Cose che penso di aver fatto
- Server (salvo TIMEOUT e gestione client che muore)
- Interazione Client-Server (TCP)
- Gestione stream IO/Socket (senza polling)
- Gestione comandi su ambiente MENU/GAME
- Casino
