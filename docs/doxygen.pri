# -------------------------------------------------
# Doxygen HTML documentation
# -------------------------------------------------

DOXYGEN = doxygen
DOXYFILE =$$PWD/doxygen/Doxyfile
DOCDIR = ./docs

doc.target = doc
doc.commands = \
    rm -rf $$DOCDIR/html $$DOCDIR/latex && \
    $$DOXYGEN $$DOXYFILE
doc.CONFIG += no_link

# -------------------------------------------------
# Doxygen PDF documentation
# -------------------------------------------------

docpdf.target = doc-pdf
docpdf.commands = \
    rm -rf $$DOCDIR/html $$DOCDIR/latex && \
    $$DOXYGEN $$DOXYFILE && \
    cd $$DOCDIR/latex && make
docpdf.CONFIG += no_link

QMAKE_EXTRA_TARGETS += doc docpdf

#endable to build documentation ...
#QMAKE_POST_LINK += $$DOXYGEN $$DOXYFILE
