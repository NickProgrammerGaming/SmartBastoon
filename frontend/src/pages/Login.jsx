import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';

const Login = () => {
  const navigate = useNavigate();
  const [formData, setFormData] = useState({ email: '', password: '' });
  const [errorMessage, setErrorMessage] = useState('');
  const [successMessage, setSuccessMessage] = useState('');

  const handleInputChange = (e) => {
    const { name, value } = e.target;
    setFormData({ ...formData, [name]: value });
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    setErrorMessage('');
    setSuccessMessage('');

    fetch('http://localhost:4000/api/auth/login', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(formData),
    })
      .then((response) => response.json())
      .then((data) => {
        if (data.token) {
          localStorage.setItem('token', data.token);
          setSuccessMessage('Успешен вход!');
          setTimeout(() => navigate('/stats'), 2000);
        } else {
          setErrorMessage(data.message || 'Невалиден имейл или парола');
        }
      })
      .catch((error) => {
        setErrorMessage('Грешка при вход. Опитайте отново.');
        console.error('Грешка:', error);
      });
  };

  return (
    <div className="container"> 
      <h1>Вход</h1>
      {errorMessage && <p className="message" style={{ color: 'red' }}>{errorMessage}</p>}
      {successMessage && <p className="message" style={{ color: 'green' }}>{successMessage}</p>}

      <form onSubmit={handleSubmit}>
        <input
          type="email"
          name="email"
          placeholder="Имейл"
          value={formData.email}
          onChange={handleInputChange}
          required
        />
        <input
          type="password"
          name="password"
          placeholder="Парола"
          value={formData.password}
          onChange={handleInputChange}
          required
        />
        <button type="submit">Влез</button>
      </form>
      <p className="message">Нямате акаунт? <a href="/register">Регистрирайте се</a></p>
    </div>
  );
};

export default Login;