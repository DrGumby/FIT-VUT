## Implementační dokumentace k 1. úloze do IPP 2018/2019

Jméno a příjmení: Kamil Vojanec  
Login: xvojan00  

### Úvod

V první části projektu do předmětu IPP bylo úkolem vytvořit lexikální a syntaktický analyzátor jazyka IPPcode19. Tato implementace je provedena v jazyce PHP 7.3 ve zdrojovém souboru ``parse.php``. Vstupem programu je text v jazyce IPPcode19, který je čten ze standardního vstupu a výstupem je analyzovaná posloupnost instrukcí zapsána v jazyce XML na standardní výstup.

### Implementace

Program využívá knihovny XMLWriter pro tvorbu XML výstupu. Na začátku běhu programu jsou zkontrolovány uživatelské argumenty příkazové řádky. Povoleným argumentem je ``--help``, který tiskne nápovědu a také sada argumentů, jež je součástí rozšíření, viz [kapitola rozšíření](#Rozšíření)

Analýza programu probíhá po řádcích. Nejprve je zkonrolována hlavička souboru, ta musí obsahovat povinné pole ``.ippcode19``, a to nezávisle na velikosti písmen. Analýza jednotlivých řádků probíhá následovně:
1. Kontrola platnosti operačního kódu a zjištění očekávaného počtu argumentů

2. Kontrola lexikální platnosti argumentů pomocí regulárních výrazů  
   V tomto bodě rovněž probíhá převod řetězcových literálů do správného tvaru
   
3. Syntaktická kontrola správného počtu a platnosti argumentů vůči danému operačnímu kódu

XML výstup je generován zároveň s analýzou, jednotlivé položky jsou generovány okamžitě po jejich analýze. V případě chyby je program ukončen voláním funkce ``exit()`` s určeným návratovým kódem.

### Rozšíření

Implementováno je rozšíření STATP, jehož cílem je poskytovat určité statistické údaje o analyzovaném kódu. Tyto statistiky jsou implemtovány jako soubor čítaču, který je aktualizován za běhu samotné analýzy.

Pro výpis statistických dat je nutné v příkazové řádce specifikovat argument ``--file=filename``, kde filename je název souboru, v němž budou statistiky umístěny. Dále je nustno specifikovat některou z následujících možností:

* ``--loc``  zapíše počet užitečných řádků s vynecháním prázdných řádků a komentářů
* ``--comments`` zapíše počet řádků obsahujících komentáře
* ``--jumps`` zapíše počet skoků v programu
* ``--labels`` zapíše počet návěští v programu

Jednotlivé statistiky jsou v souboru zapsány ve stejném pořadí, v jakém byly uvedeny argumenty příkazové řádky.