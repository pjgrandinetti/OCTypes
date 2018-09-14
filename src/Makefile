LEX     = flex
YACC    = bison -y
YFLAGS  = -d

objects =  OCComplexParser.tab.o OCComplexScanner.o OCType.o OCAutoreleasePool.o OCString.o OCNumber.o OCData.o OCArray.o OCBoolean.o \
OCDictionary.o OCMath.o

libOCTypes.a : $(objects)
	ar rcs libOCTypes.a $(objects)

OCComplexScanner.c : OCComplexScanner.l

OCComplexParser.tab.c + OCComplexParser.tab.h : OCComplexParser.y
	yacc -d OCComplexParser.y
	mv y.tab.c OCComplexParser.tab.c
	mv y.tab.h OCComplexParser.tab.h

OCType.o : OCType.h OCLibrary.h
	clang -c -O3 OCType.c

OCAutoreleasePool.o : OCAutoreleasePool.h OCLibrary.h
	clang -c -O3 OCAutoreleasePool.c

OCString.o : OCString.h OCLibrary.h
	clang -c -O3 OCString.c

OCNumber.o : OCNumber.h OCLibrary.h
	clang -c -O3 OCNumber.c

OCData.o : OCData.h OCLibrary.h
	clang -c -O3 OCData.c

OCArray.o : OCArray.h OCLibrary.h
	clang -c -O3 OCArray.c

OCBoolean.o : OCBoolean.h OCLibrary.h
	clang -c -O3 OCBoolean.c

OCDictionary.o : OCDictionary.h OCLibrary.h
	clang -c -O3 OCDictionary.c

OCMath.o : OCMath.h OCLibrary.h
	clang -c -O3 OCMath.c

clean :
	rm $(objects) libOCTypes.a OCComplexScanner.c OCComplexParser.tab.c
