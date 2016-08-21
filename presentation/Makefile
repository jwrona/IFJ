SOURCE = prezentace

latex: $(SOURCE).tex
	pdflatex $(SOURCE).tex
	pdflatex $(SOURCE).tex #dvoji preklad kvuli odkazum
#	dvips -t a4 $(SOURCE).dvi #prevod na ps s velikosti strany a4
#	ps2pdf $(SOURCE).ps #prevod do pdf

vlna: $(SOURCE).tex
	vlna -l -m -n $(SOURCE).tex #pridani vlnek do zdrojaku, nedelat dokud to neni hotove
clean:
	rm $(SOURCE).aux $(SOURCE).log $(SOURCE).out $(SOURCE).toc
