from fastapi import APIRouter, HTTPException
from models.paqueteESP import paqueteESP
from utils import obtenerFechaHora, insertarRegistroenCSV
from routes.get import get_clima_santiago
from RNA import results_RNA

router = APIRouter()

@router.post("/recepcion")
def recepcion(dato: paqueteESP):
    fecha, hora = obtenerFechaHora()
    
    try:
        data_clima = get_clima_santiago()
        temp_clima_externa = float(data_clima["temperatura"])
        es_dia_modelo = 1 if data_clima["estado_dia"] == "True" else 0
        estado_dia_bool = True if es_dia_modelo == 1 else False
    except Exception as e:
        raise HTTPException(status_code=503, detail=f"Error al obtener datos del clima: {e}")

    prediccion_ventana, prediccion_ventilador = results_RNA(
        temp_int=dato.temperatura,
        temp_ext=temp_clima_externa,
        hum=dato.humedad,
        es_dia=es_dia_modelo
    )

    filaCVS = [
        fecha, hora, dato.origen, dato.humedad, dato.temperatura, 
        temp_clima_externa, dato.ventana, dato.ventilador, estado_dia_bool
    ]
    insertarRegistroenCSV("historico.csv", filaCVS)

    return {
        "ventana_pred": prediccion_ventana,
        "ventilador_pred": prediccion_ventilador,
        "temp_externa": temp_clima_externa,
        "es_dia": estado_dia_bool,
        "hora_actual": hora
    }