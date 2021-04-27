# EDJournal_server
This is a websocket server that broadcasts all events received in the elite dangerous journal. This make it possible to read these events from a different pc.

# Usage
When u start the program you can connect via ws://127.0.0.1:5234 
or you can connect with a other pc on your local network or even via the other side of the world if you have port forwarded.

```
--h show all command line options
--p <port> change port that the server runs on
--c <bool> turn caching of events on and of if this is on all events that have happened will be send on a new connection
```
