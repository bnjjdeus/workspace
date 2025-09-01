from utils import crearCSV

def inicioAPI():
    columnas = [
        "fecha", 
        "hora", 
        "origen",  
        "hum", 
        "temp", 
        "temp_clima",
        "ventana",
        "ventilador",
        "estado_dia"
    ]
    crearCSV("historico.csv", columnas)