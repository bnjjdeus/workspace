# events.py
from utils import crearCSV

def inicioAPI():
    crearCSV("historico.csv", ["fecha", "hora", "origen",  "hum", "temp", "temp_clima", "estado_dia"])