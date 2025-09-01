from fastapi import APIRouter, HTTPException
import requests

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
        response.raise_for_status() # Lanza una excepción si la respuesta es un error
        data = response.json()
        temperatura = data['current_weather']['temperature']
        is_day = data['current_weather']['is_day']

        # Usar un operador ternario para determinar el estado del día
        estado_dia = "True" if is_day == 1 else "False"

        return {
            "temperatura": f"{temperatura}",
            "estado_dia": estado_dia,

        }
    except requests.exceptions.RequestException as e:
        raise HTTPException(status_code=500, detail=f"Error al conectar con la API del clima: {e}")