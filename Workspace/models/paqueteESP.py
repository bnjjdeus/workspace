
from pydantic import BaseModel

class paqueteESP(BaseModel):
    origen: str
    temperatura: float
    humedad: float
    ventana: bool   # <-- Cambiado de int a bool
    ventilador: bool # <-- Cambiado de int a bool

class respuestaESP(BaseModel):
    recepcion: str
    fecha: str
    hora: str
