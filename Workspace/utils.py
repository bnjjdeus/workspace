import datetime
import csv

def obtenerFechaHora():
    ahora = datetime.datetime.now()
    strFecha= ahora.strftime(format="%d-%m-%Y")
    strHora = ahora.strftime(format="%H:%M:%S")
    return (strFecha, strHora)

def crearCSV(nombre:str, columnas:list):
    with open(nombre, "w", newline="", encoding="latin-1") as archivo:
        escritor = csv.writer(archivo)
        escritor.writerow(columnas)
    print(f"archivo{nombre} creado")

def insertarRegistroenCSV(nombre:str, registros:list):
    with open(nombre, "a", newline="",encoding="latin-1") as archivo:
        escritor = csv.writer(archivo)
        escritor.writerow(registros)