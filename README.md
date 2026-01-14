# Hcpp
(Hypertext C PreProcessor)

# Dependancies
- cc

# Setup
first you need to bootstrap the executable, after this it will automatically recompile itself on changes in the provided hcml file. 
cc ./src/hcpp.c ./src/libs/*.c -o build/hcpp && ./build/hcpp ./[FILENAME].hcml ./src/temp.h
