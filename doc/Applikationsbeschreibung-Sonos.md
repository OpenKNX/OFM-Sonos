<!-- SPDX-License-Identifier: AGPL-3.0-only -->
<!-- Copyright (C) 2025 Michael Geramb -->

# Applikationsbeschreibung Sonos

Die Anwendung besteht aus der Definition der Sonos Tonquellen und der Sonos Lautsprecher.

<!-- DOC HelpContext="SonosTonquellen" -->
# Sonos Tonquellen

Es können mehrere Tonquellen definiert werden, die über ein Objekt abgespielt werden könnnen.

### Tonquelle

Defintion einer Tonquelle.

<!-- DOC -->
### Quellenart

Folgene Option stehen zur Auswahl:

- Inaktiv
- Radio Stream
- Http
- Musikbibliothek Datei
- Musikbibliothek Verzeichnis
- Line In
- TV In (Nur mit Playbar)
- Sonos Playlist
- Sonos Uri

### Radio Stream

<!-- DOC HelpContext="StreamUrl" -->
### Streaming Url

Die URL des Radio Streams.

z.B.: https://orf-live.ors-shoutcast.at/wie-q2a

<!-- DOC HelpContext="StreamTitle" -->
### Name 

Name der Radiostation.

z.B. Radio Wien

<!-- DOC HelpContext="StreamImageUrl" -->
### Logobild URL

Die URL des Logobildes des Radiosenders.

Dieser Parameter is optional.

z.B.:
https://cdn-profiles.tunein.com/s44255/images/logod.jpg

<!-- DOC -->
## Http

Wiedergabe eines HTTP Audio streams.

<!-- DOC HelpContext="HTTPUrl" -->
### URL

Die URL der HTTP Audio Datei.

<!-- DOC -->
## Musikbibliothek Datei

Wiedergabe einer Datei aus der Musikbibliothek

<!-- DOC HelpContext="Netzwerkdatei" -->
### Netzwerk Dateipfad

Der UNC Pfad der Datei in der Musikbibliothek.

Hinweis: Es können nur UNC Pfade verwenden werden, die in die Musikbibliothek importiert wurden.

<!-- DOC -->
## Musikbibliothek Verzeichnis

<!-- DOC HelpContext="Verzeichnis" -->
### Neztwerk Verzeichnis

Der UNC Pfad eines Verzecichnises in der Musikbibliothek.
Beispielsweise das Verzeichnis eines Albums.

Hinweis: Es können nur UNC Pfade verwenden werden, die in die Musikbibliothek importiert wurden.

<!-- DOC HelpContext="VerzeichnisZufallswiedergabe" -->
### Zufallswiedergabe

Die Dateien im Verzeichnis werden in zufälliger Reihenfolge abgespielt wenn die Option aktiviert ist.

<!-- DOC -->
## Line In

Schaltet auf den Line In Eingang um.

<!-- DOC -->
## TV In (Nur mit Playbar)

Schaltet auf den TV Eingang.
Diese Option funktioniert nur am Sonos Playbar.


<!-- DOC -->
## Sonos Playlist

Spielt eine Sonos Playliste ab.

<!-- DOC HelpContext="Playlist" -->
## Name der Playlist

Name der Playliste.

<!-- DOC HelpContext="PlaylistZufallswiedergabe" -->
### Zufallswiedergabe

Die Titel in der Playliste werden in zufälliger Reihenfolge abgespielt wenn die Option aktiviert ist.

<!-- DOC -->
## Sonos Uri

Diese Einstellung ist nur für Experten. 
Es können die Sonos eigenen Schemas verwendet werden.

<!-- DOC HelpContext="SonosUri" -->
### Sonos URI

Sonos interne URI.

<!-- DOC HelpContext="SonosTitle" -->
### Name

Name der für die Tonquellenanzeige verwendet wird.

<!-- DOC HelpContext="SonsosImageUrl" -->
### Logobild URL

Url für das Logo das Angezeigt werden soll.
Dieser Parameter ist optional.

<!-- DOC HelpContext="SonosZufallswiedergabe" -->
### Zufallswiedergabe

Stellt die Sonos URI mehrere Title bereit, werden diese bei aktivierter Option in zufälliger Reihenfolge wiedergegeben.

<!-- DOC -->
# Sonos Lautsprecher

In diesem Abschnitt werden die Sonos Lautsprecher definiert.
Es müssen alle Sonos Lautsprecher definiert werden, ausgenommen Stereopaare.
Bei Stereopaaren wird nur einer der Lautsprecher definiert.

<!-- DOC -->
## Allgemein 

In diesem Abschnitt werden die Basiseinstellungen des Moduls vorgenommen.

<!-- DOC -->
### Anzahl der Lautsprecher

Die Anzahl der Sonos Lautsprecher.
Stereopaare zählen als ein Lautsprecher.

Es müssen alle Sonos Lautsprecher definiert werden.

Hinweis:
Die Liste aller Sonos ist notwendig, damit die Gruppenumschaltungen richtig erfolgen.
Soll ein Lautsprecher nicht über KNX gesteuert werden, kann dieser als "Nur Gruppenteilnehmer" definiert werden.

### Benachrichtung 1-4

Es können 4 Benachrichtungstöne konfiguriert werden die über ein Objekt aktiviert werden.

<!-- DOC HelpContext="NotificationUrl" -->
#### URL

URL auf eine MP3-Datei.

Beispiele:

- https://cdn.freesound.org/previews/555/555016_12434214-lq.mp3
- https://cdn.freesound.org/previews/410/410384_2462-lq.mp3
- https://cdn.freesound.org/previews/428/428860_8610345-lq.mp3
- https://cdn.freesound.org/previews/27/27880_208079-lq.mp3

<!-- DOC HelpContext="NotificationVolume" -->
#### Lautstärke

Lautstärke für die Benachrichtigung.

<!-- DOC -->
### Verwendung

Einstellung:

- **Inaktiv**
der Kanal wird nicht verwendet

- **Einzelsteuerung**
der Kanal wird zum Steueren eines einzeln Lautsprechers verwendet.

- **Gruppensteuerung**
der Kanal wird zur Steuerung einer Sonosgruppe verwendet.

- **Einzel- und Gruppensteurung**
der Kanal wird zum Steuern eines einzeln Lautsprechers und zur Steuerung einer Gruppe verwendet.

- **Nur Gruppenteilnehmer**
Der Lautsprecher wird über KNX nicht verwendet. 
Die Konfiguration dient dazu, dass der Lautsprecher als Gruppenteilnehmer verwendet werden kann.

<!-- DOC -->
### Name

Name des Sonos Lautsprecher.
Erleichert die KNX Konfiguration da beispielsweise die Objekte und Kanäle den Namen verwenden.

<!-- DOC -->
### Sonos-IP

Die IP-Adresse des Sonos-Lautsprechers

<!-- DOC -->
### Kanal deaktivieren (zu Testzwecken)

Der Kanal wird deaktivert, die Konfiguration bleibt jedoch erhalten

<!-- DOC -->
### Relative-Lautstaerkeänderung

Die relative Lautstärkeänderung in Prozent bei Verwendung des Eingangs "Lautstärke Verringern/Erhöhen"

<!-- DOC -->
### Relative Gruppen Lautstärkeänderung

Die relative Lautstärkeänderung der Gruppe in Prozent bei Verwendung des Eingangs "Gruppenlautstärke Verringern/Erhöhen"


