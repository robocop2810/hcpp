# Hcpp
(Hypertext C PreProcessor)

## Disclaimer
**DO NOT USE THIS IN A PRODUCTION ENVIRONMENT.**
This webserver is an experimental proof-of-concept. It uses `system()` calls to dynamically recompile code at runtime, which poses significant security risks and performance overhead. It is intended solely for educational purposes and development experiments.

# Dependancies
- cc
- linux (microslop support might come later)
# Setup
To start the server run the following bootstrapping command:
```bash
cc ./src/hcpp.c ./src/libs/*.c -o build/hcpp && ./build/hcpp ./[FILENAME].hcml on
```
