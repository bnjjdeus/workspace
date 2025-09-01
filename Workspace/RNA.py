from tensorflow.keras.models import load_model
from models.paqueteESP import paqueteESP, respuestaESP
import numpy as np
from routes import recibirDatos, get

RNA = load_model("models/modelo_entrenado.h5")
RNA.load_weights("models/modelo.weights.h5")

def results_RNA(temp_int:float, temp_ext:float, hum_int:float, hum:float, es_dia:int):
    entrada = np.array([[temp_int, temp_ext, hum_int, hum, es_dia]])
    prediccion = RNA.predict(entrada, verbose=0)
    Prediccion = (prediccion > 0.5).astype(int)
    Ventana = int(Prediccion[0][0])
    Ventilador = int(Prediccion[0][1])

    return Ventana, Ventilador
