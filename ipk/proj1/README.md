# IPK Projekt 1: Server poskytující informace o systému pomocí HTTP

## Popis problému

Tématem projektu do předmětu IPK je implementovat jednoduchý server, jež bude poskytovat různé informace o systému, na němž je spuštěn pomocí protokolu HTTP. 

Mezi poskytované informace patří:

1. Síťové jméno počítače včetně domény
2. Informace o procesoru
3. Aktuální informace o zátěži procesoru  
   * Tato informace rovněž může být opakovaně aktualizována

Server komunikuje pomocí internetového prohlížeče a nástroji ``wget`` a ``curl``.

## Návrh řešení

Pro implementaci jednoduchého serveru je použit jazyk Python 3.6. Ústředním prvkem požitým pro zpracování síťové komunikace jsou sockety přijímající provoz s protokolem TCP. V jazyce Python je pro tuto komunikaci využit modul ``socket``. 

Samotný server je implementován jako třída ``HttpServer``, která obsahuje všechny metody potřebné ke správnému fungování serveru. 

Neblokování jednotlivých požadavků a jejich souběžné zpracování je implementováno pomocí mechanismu [select](https://pymotw.com/2/select/). Tento způsob zajišťuje efektivní zpracování vstupně-výstupních systému a zařazuje odpovědi do výstupní fornty. K tomuto účelu jsou v programu využity knihovny ``select`` a ``queue``.

Další částí projektu bylo vytvoření správné odpovědi na daný požadavek. Pro tento účel obsahuje třída ``HttpServer`` několik metod. Zejména se jedná o metody ``generate_response`` a ``generate_headers``. Tyto metody dohromady vytvoří správnou HTTP hlavičku a vygenerují odpověď. Odpověď se odešle buď v MIME formátu ``text/plain`` nebo ``application/json``. Typ odpovědi je vybírán na základě hlavičky ``Accept`` v příchozím požadavku.

## Použití aplikace

Díky implementaci v jazyce Python není nutné před spuštěním provádět překlad. Příkaz ``make build`` je tedy prázdný a nic neprovádí. 

Samotné spuštění aplikace pomocí programu make probíhá příkazem ``make run port=12345``, kde 12345 je číslo portu, na kterém bude server naslouchat.

Klient na server může zasílat dotazy metodou GET, kde ``servername`` značí název počítače a 12345 je port. Zpracovávané dotazy mohou být následující:

1. ``http://servername:12345/hostname``  
   Vrací síťové jméno počítače včetně domény

2. ``http://servername:12345/cpu-name``  
   Vrací informace o procesoru

3. ``http://servername:12345/load``  
   Vrací zátěž systému
   
4. ``http://servername:12345/load?refresh=x``  
   Vrací zátěž systému, kterou každých x sekund obnovuje

