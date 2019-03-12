pdf: ps
	ps2pdf proj1.ps
ps: dvi
	dvips -t a4 proj1.dvi
dvi:
	latex proj1.tex
	latex proj1.tex #Přidáno dvakrát kvůli správnému zobrazení referencí
clean:
	-rm proj1.aux proj1.dvi proj1.log proj1.out proj1.ps proj1.pdf
