import React, { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { Line } from 'react-chartjs-2';
import { Chart as ChartJS, CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend } from 'chart.js';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

const Stats = () => {
  const navigate = useNavigate();
  const [sensorData, setSensorData] = useState([]);
  const [emergencyData, setEmergencyData] = useState([]);
  const token = localStorage.getItem('token');

  useEffect(() => {
    if (!token) {
      navigate('/');
    }
  }, [token, navigate]);

  useEffect(() => {
    if (token) {
      fetch('http://localhost:4000/api/sensor-data', {
        method: 'GET',
        headers: {
          Authorization: `Bearer ${token}`,
        },
      })
        .then((response) => {
          if (response.status === 401) {
            localStorage.removeItem('token');
            navigate('/');
          }
          return response.json();
        })
        .then((data) => {
          setSensorData(data);

          const emergencyMessages = data.map((entry, index) => {
            if (Math.abs(entry.gyroscope_x) < 0.1 && Math.abs(entry.gyroscope_y) < 0.1 && Math.abs(entry.gyroscope_z) < 0.1) {
              return { timestamp: entry.timestamp, message: 'Emergency: User not standing up!' };
            } else {
              return null;
            }
          }).filter((msg) => msg !== null);
          setEmergencyData(emergencyMessages);
        })
        .catch((error) => {
          console.error('Error fetching sensor data:', error);
          navigate('/');
        });
    }
  }, [token, navigate]);

  const ultrasonicData = {
    labels: sensorData.map((data) => new Date(data.timestamp).toLocaleTimeString()),
    datasets: [
      {
        label: 'Ultrasonic Sensor 1 (cm)',
        data: sensorData.map((data) => data.ultrasonic1),
        borderColor: 'rgba(255, 99, 132, 1)',
        backgroundColor: 'rgba(255, 99, 132, 0.2)',
        borderWidth: 1,
      },
      {
        label: 'Ultrasonic Sensor 2 (cm)',
        data: sensorData.map((data) => data.ultrasonic2),
        borderColor: 'rgba(54, 162, 235, 1)',
        backgroundColor: 'rgba(54, 162, 235, 0.2)',
        borderWidth: 1,
      },
      {
        label: 'Ultrasonic Sensor 3 (cm)',
        data: sensorData.map((data) => data.ultrasonic3),
        borderColor: 'rgba(75, 192, 192, 1)',
        backgroundColor: 'rgba(75, 192, 192, 0.2)',
        borderWidth: 1,
      },
    ],
  };

  const emergencyMessageData = {
    labels: sensorData.map((data) => new Date(data.timestamp).toLocaleTimeString()),
    datasets: [
      {
        label: 'Emergency Messages',
        data: emergencyData.map((msg) => msg ? 1 : 0),
        borderColor: 'rgba(255, 99, 132, 1)',
        backgroundColor: 'rgba(255, 99, 132, 0.2)',
        borderWidth: 1,
      },
    ],
  };

  const chartOptions = {
    responsive: true,
    plugins: {
      legend: {
        position: 'top',
      },
      title: {
        display: true,
        text: 'Sensor Data Over Time',
      },
    },
  };

  return (
    <div className="container">
      <h1>Sensor Data Statistics</h1>

      <div>
        <h2>Ultrasonic Sensors Over Time</h2>
        {ultrasonicData.labels.length > 0 ? (
          <Line data={ultrasonicData} options={chartOptions} />
        ) : (
          <p>Loading Ultrasonic Sensor Data...</p>
        )}
      </div>

      <div>
        <h2>Emergency Messages</h2>
        {emergencyMessageData.labels.length > 0 ? (
          <Line data={emergencyMessageData} options={chartOptions} />
        ) : (
          <p>Loading Emergency Data...</p>
        )}
      </div>
    </div>
  );
};

export default Stats;
