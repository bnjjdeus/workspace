# recibirDatos.py

from fastapi import APIRouter
from models.paqueteESP import paqueteESP, respuestaESP
from utils import obtenerFechaHora, insertarRegistroenCSV
from routes.get import get_clima_santiago

router = APIRouter()

@router.post("/recepcion")
def recepcion(dato : paqueteESP):
    fecha, hora = obtenerFechaHora()
    
    # 1. Obtén los datos del clima
    try:
        data_clima = get_clima_santiago()
        temp_clima = data_clima["temperatura"]
        estado_dia = data_clima["estado_dia"]
    except Exception as e:
        print(f"Error al obtener los datos del clima: {e}")
        temp_clima = "N/A"
        estado_dia = "N/A"

    # 2. --- NUEVO: Añade los datos de los relés a la fila del CSV ---
    # Los nuevos datos ya vienen en el objeto 'dato' gracias al modelo actualizado
    filaCVS = [
        fecha, 
        hora, 
        dato.origen, 
        dato.humedad, 
        dato.temperatura, 
        temp_clima,
        dato.ventana,     # <-- DATO AÑADIDO
        dato.ventilador,  # <-- DATO AÑADIDO 
        estado_dia
    ]
    # 3. Inserta el registro en el CSV (esta función no necesita cambios)
    insertarRegistroenCSV("historico.csv", filaCVS)
    
    respuesta = respuestaESP(recepcion="ok",
                             fecha=fecha,
                             hora=hora)
    return respuesta