const express = require('express');
const mysql = require('mysql2');
const cors = require('cors');
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const { body, validationResult } = require('express-validator');
const authMiddleware = require('./middleware/auth');
const mqtt = require('mqtt');
require('dotenv').config();

const app = express();
app.use(cors({ origin: 'http://localhost:4173' }));
app.use(express.json());

// MySQL connection
const db = mysql.createConnection({
  host: process.env.DB_HOST,
  user: process.env.DB_USER,
  password: process.env.DB_PASSWORD,
  database: process.env.DB_NAME,
  port: 3308,
});

db.connect((err) => {
  if (err) {
    console.error('Database connection failed: ' + err.stack);
    return;
  }
  console.log('Connected to MySQL database.');
});

// MQTT Client Setup
const mqttClient = mqtt.connect('mqtt://192.168.236.174');

mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');
  mqttClient.subscribe('esp32/sensordata');
  mqttClient.subscribe('esp32/emergency');
});

mqttClient.on('message', (topic, message) => {
  const data = JSON.parse(message.toString());

  // Handle sensor data
  if (topic === 'esp32/sensordata') {
    console.log('Sensor Data:', data);
    // Save sensor data to MySQL database
    const sql = 'INSERT INTO sensor_data (ultrasonic1, ultrasonic2, ultrasonic3, gyroscope_x, gyroscope_y, gyroscope_z, timestamp) VALUES (?, ?, ?, ?, ?, ?, ?)';
    db.query(sql, [data.ultrasonic1, data.ultrasonic2, data.ultrasonic3, data.gyroscope_x, data.gyroscope_y, data.gyroscope_z, new Date()], (err) => {
      if (err) console.error('Error saving sensor data:', err);
    });
  }

  if (topic === 'esp32/emergency') {
    console.log('Emergency Data:', data);
  }
});

app.post(
  '/api/auth/register',
  [
    body('name').not().isEmpty().withMessage('Name is required'),
    body('email').isEmail().withMessage('Invalid email'),
    body('password').isLength({ min: 6 }).withMessage('Password must be at least 6 characters'),
  ],
  async (req, res) => {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({ errors: errors.array() });
    }

    const { name, email, password } = req.body;

    try {
      const hashedPassword = await bcrypt.hash(password, 10);
      const sql = 'INSERT INTO users (name, email, password) VALUES (?, ?, ?)';
      db.query(sql, [name, email, hashedPassword], (err, result) => {
        if (err) return res.status(500).json({ error: err.message });

        res.json({ message: 'Registration successful!' });
      });
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }
);

app.post(
  '/api/auth/login',
  [
    body('email').isEmail().withMessage('Invalid email'),
    body('password').not().isEmpty().withMessage('Password is required'),
  ],
  async (req, res) => {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({ errors: errors.array() });
    }

    const { email, password } = req.body;

    const sql = 'SELECT * FROM users WHERE email = ?';
    db.query(sql, [email], async (err, results) => {
      if (err) return res.status(500).json({ error: err.message });

      if (results.length === 0) {
        return res.status(400).json({ message: 'Invalid email or password' });
      }

      const user = results[0];
      const isMatch = await bcrypt.compare(password, user.password);

      if (!isMatch) {
        return res.status(400).json({ message: 'Invalid email or password' });
      }

      const token = jwt.sign({ id: user.id }, 'your_secret_key', { expiresIn: '10h' });

      res.json({ token, user: { id: user.id, name: user.name, email: user.email } });
    });
  }
);

app.get('/api/sensor-data', authMiddleware(), (req, res) => {
  const sql = 'SELECT * FROM sensor_data';
  db.query(sql, (err, result) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json(result);
  });
});

const PORT = 4000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});