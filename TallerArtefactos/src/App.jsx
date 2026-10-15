import { useEffect, useState } from 'react'
import mqtt from 'mqtt'
import './App.css'

const MQTT_URL = import.meta.env.VITE_MQTT_URL || 'ws://localhost:9001'
const MQTT_TOPICS = {
  temperature: import.meta.env.VITE_MQTT_TEMPERATURE_TOPIC || 'clima/esp32/temperatura',
  humidity: import.meta.env.VITE_MQTT_HUMIDITY_TOPIC || 'clima/esp32/humedad',
  sunlight: import.meta.env.VITE_MQTT_SUNLIGHT_TOPIC || 'clima/esp32/luz',
}
const fallbackReading = { temperature: 24, humidity: 58, sunlight: 74 }
const fallbackHistory = [24, 26, 27, 23, 20]

function parseSensorValue(message, fieldNames) {
  const text = message.toString().trim()
  try {
    const payload = JSON.parse(text)
    if (typeof payload === 'number') return payload
    const value = fieldNames.map((field) => payload[field]).find((item) => item !== undefined)
    return Number(value)
  } catch {
    return Number(text)
  }
}

function getWeatherCondition(sunlight) {
  if (sunlight < 20) return { label: 'Lluvioso', icon: '☔' }
  if (sunlight < 65) return { label: 'Parcialmente nublado', icon: '◐' }
  return { label: 'Soleado', icon: '☀' }
}

function App() {
  const [reading, setReading] = useState(fallbackReading)
  const [temperatureHistory, setTemperatureHistory] = useState(fallbackHistory)
  const [connectionState, setConnectionState] = useState('Conectando...')

  useEffect(() => {
    const client = mqtt.connect(MQTT_URL, { reconnectPeriod: 3000, connectTimeout: 10000 })
    const topics = Object.values(MQTT_TOPICS)

    client.on('connect', () => {
      console.info('[MQTT] Conectado a', MQTT_URL)
      setConnectionState('Sistema en linea')
      topics.forEach((topic) => {
        client.subscribe(topic, (error) => {
          if (error) {
            console.error('[MQTT] Error al suscribirse a', topic, error)
            setConnectionState('Error de suscripcion')
            return
          }
          console.info('[MQTT] Suscrito a:', topic)
        })
      })
    })
    client.on('reconnect', () => {
      console.info('[MQTT] Intentando reconectar a', MQTT_URL)
      setConnectionState('Reconectando...')
    })
    client.on('offline', () => {
      console.warn('[MQTT] Broker fuera de linea:', MQTT_URL)
      setConnectionState('Sin conexion')
    })
    client.on('error', (error) => {
      console.error('[MQTT] Error:', error.message)
      setConnectionState('Error de conexion')
    })
    client.on('message', (topic, message) => {
      console.info('[MQTT] Mensaje recibido:', topic, message.toString())
      const topicFields = {
        [MQTT_TOPICS.temperature]: ['temperatura', 'temperature', 'valor'],
        [MQTT_TOPICS.humidity]: ['humedad', 'humidity', 'valor'],
        [MQTT_TOPICS.sunlight]: ['luz', 'sunlight', 'valor'],
      }
      const value = parseSensorValue(message, topicFields[topic] || ['valor'])
      if (!Number.isFinite(value)) {
        console.warn('[MQTT] Valor no numerico:', message.toString())
        return
      }

      console.info('[MQTT] Actualizando dashboard:', topic, value)
      setReading((current) => {
        if (topic === MQTT_TOPICS.temperature) return { ...current, temperature: value }
        if (topic === MQTT_TOPICS.humidity) return { ...current, humidity: value }
        if (topic === MQTT_TOPICS.sunlight) return { ...current, sunlight: value }
        return current
      })
      if (topic === MQTT_TOPICS.temperature) setTemperatureHistory((history) => [...history.slice(-4), value])
    })

    return () => client.end(true)
  }, [])

  const formatTemperature = (temperature) => `${Math.round(temperature)}°`
  const history = temperatureHistory.slice(-5)
  const weather = getWeatherCondition(reading.sunlight)
  const points = history.map((temperature, index) => {
    const x = history.length === 1 ? 300 : 25 + index * (550 / (history.length - 1))
    const y = 145 - ((temperature - 18) / 12) * 100
    return `${x},${Math.max(35, Math.min(145, y))}`
  }).join(' ')

  return (
    <div className="app-shell">
      <aside className="sidebar"><div className="brand"><span className="brand-mark">◒</span><span>Clima<span className="brand-accent">Lab</span></span></div><nav className="nav-list" aria-label="Navegacion principal"><a className="nav-item active" href="#dashboard"><span>⌂</span> Dashboard</a></nav><div className="sidebar-footer"><div className={`status-dot ${connectionState === 'Sistema en linea' ? '' : 'status-warning'}`}></div><div><strong>{connectionState}</strong><small>MQTT · 3 topics</small></div></div></aside>
      <main className="main-content" id="dashboard"><header className="topbar"><div><h1>Resumen del clima</h1></div></header><section className="location-row"><div><span className="location-pin">⌖</span><strong>San Salvador, El Salvador</strong><span className="unit-fixed">°C</span></div></section>
        <section className="dashboard-grid"><article className="current-weather card"><div className="weather-heading"><div><p className="label">CONDICIÓN ACTUAL</p><h2>{weather.label}</h2></div><span className="weather-symbol" aria-label={weather.label}>{weather.icon}</span></div><div className="current-temperature">{formatTemperature(reading.temperature)}<span>C</span></div></article><article className="metric-card card"><div className="metric-icon blue">≈</div><div><p className="label">HUMEDAD</p><strong>{Math.round(reading.humidity)}<span>%</span></strong><p className="metric-caption">Lectura del sensor</p></div><div className="metric-ring ring-blue"><span>{Math.round(reading.humidity)}%</span></div></article><article className="metric-card card"><div className="metric-icon yellow">☼</div><div><p className="label">LUZ SOLAR</p><strong>{Math.round(reading.sunlight)}<span>%</span></strong><p className="metric-caption">Lectura del sensor</p></div><div className="metric-ring ring-yellow"><span>{(reading.sunlight * 0.084).toFixed(1)} h</span></div></article><article className="forecast-card card" id="pronostico"><div className="card-header"><div><p className="label">TEMPERATURA RECIENTE</p><h2>Últimas 5 mediciones</h2></div></div><div className="line-chart"><svg viewBox="0 0 600 190" role="img" aria-label="Gráfica lineal de las últimas mediciones de temperatura" preserveAspectRatio="none"><line className="chart-gridline" x1="0" y1="35" x2="600" y2="35" /><line className="chart-gridline" x1="0" y1="85" x2="600" y2="85" /><line className="chart-gridline" x1="0" y1="135" x2="600" y2="135" /><polyline className="chart-line" points={points} />{history.map((temperature, index) => { const x = history.length === 1 ? 300 : 25 + index * (550 / (history.length - 1)); const y = Math.max(35, Math.min(145, 145 - ((temperature - 18) / 12) * 100)); return <circle className="chart-point" cx={x} cy={y} r="5" key={`${temperature}-${index}`} /> })}</svg><div className="chart-labels">{history.map((temperature, index) => <span key={`${temperature}-label-${index}`}><strong>{formatTemperature(temperature)}</strong></span>)}</div></div></article></section>
      </main>
    </div>
  )
}

export default App
