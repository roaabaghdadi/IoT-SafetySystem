from flask import Blueprint, request, jsonify, render_template
from app import db
from app.models import SensorData
from config import Config

main_bp = Blueprint('main', __name__)

@main_bp.route('/api/data', methods=['POST'])
def receive_data():
    data = request.get_json()
    if not data or 'temperature' not in data or 'gas_level' not in data:
        return jsonify({"status": "error", "message": "Invalid data"}), 400

    temp_value = float(data['temperature'])
    temp_status = 'critical' if temp_value > Config.TEMP_THRESHOLD else 'normal'
    temp_entry = SensorData(sensor_type='DHT11_Temp', value=temp_value, unit='°C', status=temp_status)
    db.session.add(temp_entry)

    gas_value = float(data['gas_level'])
    gas_status = 'critical' if gas_value > Config.GAS_THRESHOLD else 'normal'
    gas_entry = SensorData(sensor_type='MQ2', value=gas_value, unit='ppm', status=gas_status)
    db.session.add(gas_entry)

    db.session.commit()

    if temp_status == 'critical' or gas_status == 'critical':
        print(f"ALARM! Kritischer Wert erkannt: {data}")

    return jsonify({"status": "success"}), 201

@main_bp.route('/')
def dashboard():
    latest_data = SensorData.query.order_by(SensorData.timestamp.desc()).limit(100).all()
    return render_template('index.html', data=latest_data)