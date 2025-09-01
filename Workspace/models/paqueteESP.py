
from pydantic import BaseModel

class paqueteESP(BaseModel):
    origen: str
    temperatura: float
    humedad: float
    ventana: bool 
    ventilador: bool

class respuestaESP(BaseModel):
    recepcion: str
    fecha: str
    hora: str
