### Ueberwachung des Online Status abschalten

Über ein Ping-Telegram (ICMP) alle 15 Sekunden wird der Online Status überwacht.
Dies ist wichtig, da bei Aufrufen von Lautsprechern die Offline sind es zur starken Beeinträchtigung des Systems kommt.

Wenn die Überwachung deaktivert wird, muss entweder sicher gestellt sein das die Lautsprecher immer mit Strom versorgt sind und Online sind.
Alternativ kann man bei zum Beispiel gewohlten Abschalten der Stromversorgung des Lautsprechers durch einen Aktor auch das Freigabe Gruppenobjekt verwendet werden. 

Ist das Freigabe Gruppenobjekt auf Abschalten (0), wird der Ping ebenfalls deaktiviert.

