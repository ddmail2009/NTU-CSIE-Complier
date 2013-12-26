TARGET = parser
OBJECT = parser.tab.c parser.tab.o lex.yy.c alloc.o functions.o semanticAnalysis.o symbolTable.o codeGen.o
OUTPUT = parser.output parser.tab.h
CC = g++ -g -Wall
LEX = flex
YACC = bison -v
YACCFLAG = -d
LIBS = -lfl 
DEBUG = 1

parser: parser.tab.o alloc.o functions.o symbolTable.o semanticAnalysis.o codeGen.o
	$(CC) -o $(TARGET) parser.tab.o alloc.o functions.o symbolTable.o semanticAnalysis.o codeGen.o $(LIBS)

parser.tab.o: parser.tab.c lex.yy.c alloc.o functions.c symbolTable.o semanticAnalysis.o
	$(CC) -c parser.tab.c

semanticAnalysis.o: semanticAnalysis.c symbolTable.o
	$(CC) -c semanticAnalysis.c

symbolTable.o: symbolTable.c
	$(CC) -c symbolTable.c

lex.yy.c: lexer3.l
	$(LEX) lexer3.l

parser.tab.c: parser.y 
	$(YACC) $(YACCFLAG) parser.y

alloc.o: alloc.c
	$(CC) -c alloc.c
	
functions.o: functions.c
	$(CC) -c functions.c

codeGen.o: codeGen.c
ifeq ($(DEBUG), 0)
	$(CC) -c codeGen.c 
else
	$(CC) -c codeGen.c -D DEBUG
endif

clean:
	rm -f $(TARGET) $(OBJECT) $(OUTPUT)

