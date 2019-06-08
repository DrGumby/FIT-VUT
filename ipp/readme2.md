## Implementační dokumentace ke 2. a úloze do IPP 2018/2019

Jméno a příjmení: Kamil Vojanec  
Login: xvojan00  

### Úvod
Ve druhé části projektu do předmětu IPP bylo vytvořit interpret kódu IPPcode19 (reprezentovaný ve formě XML) a také testovací rámec pro obě části projektu. Implementace interpretu je provedena v jazyce Python 3.6.8 ve zdrojovém souboru ``interpret.py`` a testovací rámec je napsán v jazyce PHP 7.3 v souboru ``test.php``.

### Implementace
#### Interpret
Program využívá modulu ElementTree pro zpracování vstupní XML reprezentace. Na začátku programu jsou zkontrolovány uživatelské argumenty příkazové řádky. 

Základním prvkem interpretu je třída ``Program``, která uchovává objekty a datové struktury nutné ke správné interpretaci. Mezi tyto struktury patří:

* Zásobník volání podprogramů
* Globální rámec, dočasný rámec, zásobník lokálních rámců
* Tabulky symbolů příslušící každému rámci
* Seznam instrukcí
* Index aktuálně prováděné instrukce
* Tabulka návěští

Po zpracování vstupního XML se vytvoří seznam instrukcí, které bude interpret provádět. Rovněž se před samotnou interpretaci zaznamenají návěští do asociativního pole (``dictionary``). 

Samotné instrukce jsou reprezentovány abstraktní třídou ``Instruction``, která obsahuje polymorfní metodu ``run``. Z této třídy poté dědí jednotlívé konkrétní instrukce, jejichž funkčnost je impementována metodou ``run``. 

Ve třídě ``Program`` je impementována metoda ``runall``, která cyklem prochází seznamem instrukcí a invokuje metodu ``run`` každé instrukce.

Skoky jsou implementovány vyhledáním daného návěští v tabulce návěští a poté je změněn index aktuální instrukce na index daného návěští.

### Testovací rámec
Testovací rámec zkoumá vstupy a výstupy obou částí programu a generuje HTML výstup s výsledky testu.

Základní strukturou testovacího rámce je asociativní pole jednotlivých testů. Toto pole obsahuje názvy testů a přiřazuje k nim testovací soubory. 

Po vytvoření tohoto pole se vykonají všechny v něm načtené testy. Volba testů se děje pomocí argumentů ``--int-only`` a ``--parse-only``. Tyto přepínače rozhodují, jakým způsobem budou vyhodnoceny vstupy a výstupy.

V případě přepínače ``--parse-only`` se jako vstup ze souboru s příponou ``.src`` načítá kód v jazyce IPPcode19 a testuje se funkcionalita skriptu ``parse.php`` z první úlohy. Výstup je v jazyce XML a je s referenčním výstupem porovnán nástrojem JExamXML.

V případě přepínače ``int-only`` se na vstupu čte XML reprezentace jazyka IPPcode19 a testuje se funkcionalita skriptu ``interpret.py``. Při testování interpretu se rovněž načítá soubor s příponou ``.in``, který se použije jako vstup od uživatele při využití instrukce READ v interpretovaném programu. Referneční výstup se v tomto případě porovnává programem ``diff`` se standardním výstupem interpretu.

V případě, že není zadán ani jeden ze zmíněných přepínačů se testují oba skripty (tj. ``parse.php`` a ``interpret.py``) zřetězeně. Jako vstup se načte text v jazyce IPPcode19 a na výstupu se porovnává standardní výstup interpretu.

Formát HTML výstupu obsahuje v hodní části přehlednou tabulku s výsledky testů, kde v levém sloupci je uveden název testu a v pravém sloupci je uveden výsledek (OK / FAIL). Rovněž jsou buňky s výsledkem vybarveny podle úspěchu či neúspěchu testu. Pod tabulkou jsou dále uvedeny očekávané i skutečné výstupy testů, které skončily chybným výsledkem (FAIL). Pro výpis XML kódu v HTML bylo potřeba dočasně zaměnit některé, v HTML rezervované, znaky (např. ``<``, ``>``) za jejich nekonfliktní zápis (``&lt;``, ``&gt;``) a takto upravený text obalit do tagů ``<pre>``.

## Dodatek
Pro seřazení instrukcí v XML reprezentaci byl využit postup uvedený zde:

https://effbot.org/zone/element-sort.htm

Rovněž pro převod escape sekvencí do tisknutelné podoby byl převzat postup odsud:

https://stackoverflow.com/questions/4020539/process-escape-sequences-in-a-string-in-python