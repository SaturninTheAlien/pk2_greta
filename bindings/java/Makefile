all: compile headers

compile:
	javac pk2/PekkaKana2.java
	javac pk2/PrototypesHandler.java

headers:
	javac -h . pk2/PekkaKana2.java
	javac -h . pk2/PrototypesHandler.java
	javac -h . pk2/Prototype.java

run: compile
	java -XstartOnFirstThread pk2.PekkaKana2

clean:
	rm pk2/*.class

.PHONY: headers clean compile all run