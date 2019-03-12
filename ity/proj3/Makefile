pdf: ps
	ps2pdf -sPAPERSIZE=a4 proj3.ps
ps: dvi
	dvips -t a4 proj3.dvi
dvi:
	latex proj3.tex
	latex proj3.tex #Přidáno dvakrát kvůli správnému zobrazení referencí
clean:
	-rm proj3.aux proj3.dvi proj3.log proj3.out proj3.ps proj3.pdf
