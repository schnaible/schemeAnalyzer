P3.out : Project3.o Set_Limits.o Lexical_Analyzer.o Syntactical_Analyzer.o Code_Generator.o
	g++ -g -o P3.out Project3.o Set_Limits.o Lexical_Analyzer.o Syntactical_Analyzer.o Code_Generator.o

Project3.o : Project3.cpp Set_Limits.h Syntactical_Analyzer.h
	g++ -g -c Project3.cpp

Set_Limits.o : Set_Limits.cpp Set_Limits.h
	g++ -g -c Set_Limits.cpp

Lexical_Analyzer.o : Lexical_Analyzer.o.save
	cp Lexical_Analyzer.o.save Lexical_Analyzer.o

Syntactical_Analyzer.o : Syntactical_Analyzer.cpp Syntactical_Analyzer.h Lexical_Analyzer.h
	g++ -g -c Syntactical_Analyzer.cpp

Code_Generator.o: Code_Generator.cpp Code_Generator.h Lexical_Analyzer.h
	g++ -g -c Code_Generator.cpp

clean : 
	rm *.o P3.out *.gch

test :
	make
	./P3.out ./P3-1.ss  
	./P3.out ./P3-2.ss  
	./P3.out ./P3-3.ss  
	./P3.out ./P3-4.ss  
	./P3.out ./P3-5.ss  
	./P3.out ./P3-6.ss  
	./P3.out ./P3-7.ss  
	./P3.out ./P3-8.ss  
	./P3.out ./P3-9.ss  
	./P3.out ./P3-A.ss  


submit : Project3.cpp Lexical_Analyzer.h Syntactical_Analyzer.h Syntactical_Analyzer.cpp makefile README.txt
	rm -rf schnaibleP3
	mkdir schnaibleP3
	cp Project3.cpp schnaibleP3
	cp Syntactical_Analyzer.h schnaibleP3
	cp Syntactical_Analyzer.cpp schnaibleP3
	cp Code_Generator.cpp schnaibleP3
	cp Code_Generator.h schnaibleP3
	cp makefile schnaibleP3
	cp README.txt schnaibleP3
	tar cfvz schnaibleP3.tgz schnaibleP3
	cp schnaibleP3.tgz ~tiawatts/cs460drop
