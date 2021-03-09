import re

code = input("Dame el código: ")
boton = input("Dame el botón: ")

while code != "exit":
    string = "uint16_t dish" + boton.capitalize() + "[26] = {\n  "

    codes = []
    for i in code.split(" "):
        codes.append("0x" + i)

    iterator = 0
    for code in codes:
        string += code
        if iterator == 7:
            string += ",\n  "
            iterator = 0
        else:
            string += ", "
            iterator += 1
    else:
        string = string.rstrip(", ")
        string += "\n};"
    print(string)

    code = input("Dame el código: ")
    boton = input("Dame el botón: ")
