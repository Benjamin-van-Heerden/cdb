TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default ./$(TARGET)
	./$(TARGET) -f mydb.db -n
	./$(TARGET) -f mydb.db -a "Ben,123 Barn Street,60"
	./$(TARGET) -f mydb.db -a "Dan,123 Barn Street,60"
	./$(TARGET) -f mydb.db -l

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o: src/%.c
	gcc -c $< -o $@ -Iinclude
