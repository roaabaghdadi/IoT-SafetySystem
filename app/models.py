from app import db
from datetime import datetime

class SensorData(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    sensor_type = db.Column(db.String(50), nullable=False)
    value = db.Column(db.Float, nullable=False)
    unit = db.Column(db.String(10), nullable=False)
    status = db.Column(db.String(20), default='normal')
    timestamp = db.Column(db.DateTime, index=True, default=datetime.utcnow)

    def __repr__(self):
        return f'<SensorData {self.sensor_type}: {self.value} {self.unit}>'