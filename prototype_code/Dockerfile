FROM python:3.11-slim

ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

WORKDIR /app

COPY prototype_code/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY prototype_code/scada_backend.py .
COPY dataset/dataset.csv ./dataset.csv

CMD ["python", "-u", "scada_backend.py"]