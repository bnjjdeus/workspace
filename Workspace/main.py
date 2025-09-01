# main.py
from fastapi import FastAPI
from routes import recibirDatos, get
from events import inicioAPI

app = FastAPI()

app.include_router(recibirDatos.router)
app.include_router(get.router)

@app.on_event("startup")
def star_event():
    inicioAPI()