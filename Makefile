################################
#
#     Makefile BRIS (progetto lso 2013)
#     (fram 1)(fram 2)(fram 3)
################################

# ***** DA COMPLETARE ******  con i file da consegnare *.c e *.h     
# primo frammento 
FILE_DA_CONSEGNARE1=users.c users.h bris.c bris.h

# secondo frammento 
FILE_DA_CONSEGNARE2=comsock.h comsock.c bristat

# terzo frammento
FILE_DA_CONSEGNARE3=brsserver.c brsclient.c errors.h errors.c commonstrings.h Doxyfile relazione-labSOL.pdf

# Compilatore
CC= gcc
# Flag di compilazione
CFLAGS = -Wall -pedantic -g

# Librerie 
# Directory in cui si trovano le librerie
LIBDIR = ../lib
# Opzione di linking
LIBS = -L $(LIBDIR)

# Nome libreria progetto
LIBNAME1 = libbris.a
LIBNAME2 = libcomm.a
LIBNAME3 = liberr.a

# Oggetti libreria $(LIBNAME1)
# DA COMPLETARE (se si usano altri file sorgente)

# Determina l'architettura della macchina (32 o 64 bit) 
ARCH=$(shell uname -m)

# newMazzoObj contiene il nome del object file corretto

ifeq ($(ARCH), i686)                   # 32 bit
newMazzoObj = newMazzo_x86.o
newMazzoObjR = newMazzo_r_x86.o 
else                                   # 64 bit
newMazzoObj = newMazzo_x86_64.o
newMazzoObjR = newMazzo_r_x86_64.o 
endif

# per il terzo frammento
objects1 = $(newMazzoObj) users.o bris.o $(newMazzoObjR)
objects2 = comsock.o
objects3 = errors.o

# Nome eseguibili primo frammento
EXE1=bris1
EXE2=bris2
EXE3=bris3

.PHONY: clean lib test11 test12 test13 consegna1 docu

.PHONY: test21 test22 test23 consegna2 

.PHONY: test31 test32 test33 consegna3 execs 

# creazione libreria 
lib:  $(objects1) $(objects2) $(objects3)
	-rm  -f $(LIBNAME1) 
	-rm  -f $(LIBNAME2) 
	-rm  -f $(LIBNAME3)
	-mkdir ../lib
	-rm  -f $(LIBDIR)/$(LIBNAME1) 
	-rm  -f $(LIBDIR)/$(LIBNAME2)
	-rm  -f $(LIBDIR)/$(LIBNAME3)
	ar -r $(LIBNAME1) $(objects1)
	ar -r $(LIBNAME2) $(objects2)
	ar -r $(LIBNAME3) $(objects3)
	cp $(LIBNAME1) $(LIBNAME2) $(LIBNAME3) $(LIBDIR)


###### Primo test 
bris1: test-one.o 
	$(CC) -o $@  $^ $(LIBS) -lbris

test-one.o: test-one.c bris.h 
	$(CC) $(CFLAGS) -c $<	 


###### Secondo test 
bris2: test-two.o 
	$(CC) -o $@  $^ $(LIBS) -lbris

test-two.o: test-two.c users.h bris.h
	$(CC) $(CFLAGS) -c $<	 

###### Terzo test 
bris3: test-three.o 
	$(CC) -o $@  $^ $(LIBS) -lbris

test-three.o: test-three.c users.h bris.h
	$(CC) $(CFLAGS) -c $<	 

#make rule per gli altri .o del primo frammento (***DA COMPLETARE***)

bris.o: bris.c bris.h
	$(CC) $(CFLAGS) -c $<

users.o: users.c users.h
	$(CC) $(CFLAGS) -c $<

comsock.o: comsock.c comsock.h
	$(CC) $(CFLAGS) -c $<


######### target test libreria comunicazione 

testserv: testserv.o
	$(CC) -o $@  $^ $(LIBS) -lbris -lcomm
	
testserv.o: testserv.c comsock.h
	$(CC) $(CFLAGS) -c $<

testcli: testcli.o
	$(CC) -o $@  $^ $(LIBS) -lbris -lcomm
	
testcli.o: testcli.c comsock.h
	$(CC) $(CFLAGS) -c $<

######### target server e client

brsserver: brsserver.o
	$(CC) -o $@  $^ $(LIBS) -lbris -lcomm -lerr -lpthread

brsserver.o: brsserver.c comsock.h bris.h users.h commonstrings.h
	$(CC) $(CFLAGS) -c $<

brsclient: brsclient.o
	$(CC) -o $@  $^ $(LIBS) -lbris -lcomm -lerr

brsclient.o: brsclient.c comsock.h bris.h users.h commonstrings.h
	$(CC) $(CFLAGS) -c $<
	

# make rule per gli altri .o del terzo frammento (***DA COMPLETARE***)


errors.o: errors.c errors.h
	$(CC) $(CFLAGS) -c $<


# make rule "semplice" per gli eseguibili

execs:
	make clean
	make lib
	make $(exeserv)
	make $(execli)


########### NON MODIFICARE DA QUA IN POI ################
# genera la documentazione con doxygen
docu: ../doc/Doxyfile
	make -C ../doc/
	
#ripulisce  l'ambiente (cancella tutti gli object file tranne newMazzo_*x86.o e newMazzo*x86_64.o)
clean:
	find . -name "*.o" ! -name "newMazzo*x86*" -print -exec rm {} \; 



MTRACEFILE=./bris.log
FILEMAZZO=./mazzo1.out
test11: 
	make clean
	make lib
	make $(EXE1)
	-rm $(FILEMAZZO) $(FILEMAZZO).check
	cp DATA/$(FILEMAZZO).check .
	chmod 644 $(FILEMAZZO).check	
	-rm -fr $(MTRACEFILE)
	MALLOC_TRACE=$(MTRACEFILE) ./$(EXE1)
	mtrace ./$(EXE1) $(MTRACEFILE)
	diff $(FILEMAZZO) $(FILEMAZZO).check
	@echo "********** Test11 superato!"


MTRACEFILE=./bris.log
test12: 
	make clean
	make lib
	make $(EXE2)
	-rm -fr $(MTRACEFILE)
	MALLOC_TRACE=$(MTRACEFILE) ./$(EXE2)
	mtrace ./$(EXE2) $(MTRACEFILE)
	@echo "********** Test12 superato!"


FILETEST2=./users1.txt
FILETEST3=./users2.txt
test13: 
	make clean
	make lib
	make $(EXE3)
	-rm -fr $(MTRACEFILE)
	-rm $(FILETEST2) $(FILETEST3) $(FILETEST2).sort $(FILETEST3).sort
	cp DATA/$(FILETEST2) .
	chmod 644 $(FILETEST2)
	-rm -fr $(MTRACEFILE)
	MALLOC_TRACE=$(MTRACEFILE) ./$(EXE3)
	mtrace ./$(EXE3) $(MTRACEFILE)
	sort < $(FILETEST2) >$(FILETEST2).sort
	sort < $(FILETEST3) >$(FILETEST3).sort
	diff $(FILETEST2).sort $(FILETEST3).sort
	@echo "********** Test13 superato!"

# test secondo frammento
# test scrip statistiche
OUTSCRIPT=out.bristat
test21:
	cp DATA/BRS-*.log .
	chmod 644 BRS-*.log
	cp DATA/$(OUTSCRIPT).check .
	chmod 644 $(OUTSCRIPT).check
	./testbristat 1> $(OUTSCRIPT)
	diff $(OUTSCRIPT) $(OUTSCRIPT).check
	@echo "********** Test21 superato!"

# test libreria di comunicazione
OUTFILE=out.cli
TRACESERV=./traceserv.log
TRACECLI=./tracecli.log
test22:
	make clean
	make lib
	make testserv
	make testcli
	cp DATA/$(OUTFILE).check .
	chmod 644 $(OUTFILE).check
	rm -fr ./tmp
	mkdir ./tmp
	MALLOC_TRACE=$(TRACESERV) ./testserv &
	MALLOC_TRACE=$(TRACECLI) ./testcli > $(OUTFILE)
	mtrace ./testcli  $(TRACECLI)
	mtrace ./testserv $(TRACESERV)
	diff $(OUTFILE) $(OUTFILE).check
	@echo "********** Test22 superato!"



# test connessioni, registrazioni e cancellazioni
exeserv=brsserver
execli=brsclient
FILECHECK=brs.checkpoint
test23:
	make clean
	make lib
	make $(exeserv)
	make $(execli)
	-rm $(FILETEST2) $(FILECHECK) $(FILECHECK).check
	cp DATA/$(FILETEST2) .
	chmod 644 $(FILETEST2)
	cp DATA/$(FILECHECK).check .
	chmod 644 $(FILECHECK).check
	rm -fr ./tmp
	mkdir ./tmp
	./testseq
	sort $(FILECHECK) > $(FILECHECK).sort
	diff $(FILECHECK).sort $(FILECHECK).check
	@echo "********** Test23 superato!"

# test terzo frammento
# test registrazioni e cancellazioni in parallelo
FILECHECK2=brs.checkpoint2
test31:
	make clean
	make lib
	make $(exeserv)
	make $(execli)
	-rm $(FILETEST2) $(FILECHECK) $(FILECHECK2).check
	cp DATA/$(FILETEST2) .
	chmod 644 $(FILETEST2)
	cp DATA/$(FILECHECK2).check .
	chmod 644 $(FILECHECK2).check
	rm -fr ./tmp
	mkdir ./tmp
	./testpartree
	sort $(FILECHECK) > $(FILECHECK).sort
	diff $(FILECHECK).sort $(FILECHECK2).check
	@echo "********** Test31 superato!"

# test singola partita
LOGFILE=BRS-1.log
test32:
	make clean
	make lib
	make $(exeserv)
	make $(execli)
	-rm $(FILETEST2) $(LOGFILE) $(LOGFILE).check
	cp DATA/$(FILETEST2) .
	chmod 644 $(FILETEST2)
	cp DATA/$(LOGFILE).check .
	chmod 644 $(LOGFILE).check
	rm -fr ./tmp
	mkdir ./tmp
	./testfunz
	diff $(LOGFILE) $(LOGFILE).check
	@echo "********** Test32 superato!"

# test piu' partite 
test33:
	make clean
	make lib
	make $(exeserv)
	make $(execli)
	-rm $(FILETEST2) BRS-*.log BRS-*.check
	cp DATA/$(FILETEST2) .
	cp DATA/BRS-*.check .
	chmod 644 $(FILETEST2) 
	chmod 644 BRS-*.check
	rm -fr ./tmp
	mkdir ./tmp
	./testpar
	@echo "********** Test33 superato!"

SUBJECT1="lso13: consegna primo frammento"
ADDRESS="susanna.pelagatti@gmail.com"
# target di consegna del primo frammento
# effettua i test e prepara il tar per la consegna
consegna1:
	make clean
	make test11
	make test12
	make test13
	./gruppo-check.pl < gruppo.txt
	tar -cvf $(USER)-f1.tar ./gruppo.txt ./Makefile $(FILE_DA_CONSEGNARE1) 
	@echo "*** PRIMO FRAMMENTO: TAR PRONTO $(USER)-f1.tar ***"
	@echo "*** inviare per email a $(ADDRESS) con subject\n \"$(SUBJECT1)\" ***"

SUBJECT2="lso13: consegna secondo frammento"
# target di consegna del secondo frammento
# effettua i test e prepara il tar per la consegna
consegna2:
	make clean
	make test21
	make test22
	make test23
	./gruppo-check.pl < gruppo.txt
	tar -cvf $(USER)-f2.tar ./gruppo.txt ./Makefile $(FILE_DA_CONSEGNARE1) $(FILE_DA_CONSEGNARE2) 
	@echo "*** SECONDO FRAMMENTO: TAR PRONTO $(USER)-f2.tar ***"
	@echo "*** inviare per email a $(ADDRESS) con subject\n \"$(SUBJECT2)\" ***"

SUBJECT3="lso13: consegna progetto finale"
# target di consegna del terzo frammento
#effettua i test e prepara il tar per la consegna
consegna3:
	make clean
	make test11
	make test12
	make test13
	make test21
	make test22
	make test23
	make test31
	make test32
	make test33
	./gruppo-check.pl < gruppo.txt
	tar -cvf $(USER)-f3.tar ./Makefile ./gruppo.txt $(FILE_DA_CONSEGNARE1) $(FILE_DA_CONSEGNARE2) $(FILE_DA_CONSEGNARE3)
	@echo "*** TERZO FRAMMENTO: TAR PRONTO $(USER)-f3.tar ***"
	@echo "*** inviare per email a $(ADDRESS) con subject\n \"$(SUBJECT3)\" ***"
