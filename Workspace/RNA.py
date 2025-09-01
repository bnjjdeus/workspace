from tensorflow.keras.models import load_model
import numpy as np

RNA = load_model("models/modelo_entrenado.h5")

def results_RNA(temp_int: float, temp_ext: float, hum: float, es_dia: int):

    entrada = np.array([[temp_int, temp_ext, hum, es_dia]])
    
    prediccion_raw = RNA.predict(entrada, verbose=0)
    
    prediccion_final = (prediccion_raw > 0.5).astype(int)
    
    accion_ventana = int(prediccion_final[0][0])
    accion_ventilador = int(prediccion_final[0][1])

    return accion_ventana, accion_ventilador