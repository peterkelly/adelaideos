all: guide.pdf

guide.pdf: guide.lyx
	/Applications/LyX.app/Contents/MacOS/lyx --export pdflatex guide.lyx
	pdflatex guide
	pdflatex guide
	pdflatex guide

clean:
	rm -f *.pdf guide.tex guide.aux guide.log guide.out guide.toc
