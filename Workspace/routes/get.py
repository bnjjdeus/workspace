from fastapi import APIRouter, HTTPException
import requests
import pandas as pd 

router = APIRouter()

@router.get("/clima")
def get_clima_santiago():

    url = "http://api.open-meteo.com/v1/forecast"
    params = {
        "latitude": -33.490358289872816,
        "longitude": -70.6167991291248,
        "current_weather": True
    }
    try:
        response = requests.get(url, params=params)
        response.raise_for_status()
        data = response.json()
        temperatura = data['current_weather']['temperature']
        is_day = data['current_weather']['is_day']
        estado_dia = "True" if is_day == 1 else "False"

        return {
            "temperatura": f"{temperatura}",
            "estado_dia": estado_dia,
        }
    except requests.exceptions.RequestException as e:
        raise HTTPException(status_code=500, detail=f"Error al conectar con la API del clima: {e}")

@router.get("/historial/temperatura")
def get_historial_temperatura():
    try:

        df = pd.read_csv("historico.csv")

        ultimos_20 = df.tail(20)
        horas = ultimos_20["hora"].tolist()
        lecturas = ultimos_20["temp"].tolist()
        
        return {
            "horas": horas,
            "lecturas": lecturas
        }
    except FileNotFoundError:
        raise HTTPException(status_code=404, detail="El archivo historico.csv no fue encontrado.")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ocurrió un error al leer el archivo: {e}")

@router.get("/historial/humedad")
def get_historial_humedad():
    try:
        df = pd.read_csv("historico.csv")
        ultimos_20 = df.tail(20)

        horas = ultimos_20["hora"].tolist()
        lecturas = ultimos_20["hum"].tolist()
        
        return {
            "horas": horas,
            "lecturas": lecturas
        }
    except FileNotFoundError:
        raise HTTPException(status_code=404, detail="El archivo historico.csv no fue encontrado.")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Ocurrió un error al leer el archivo: {e}")