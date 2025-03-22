import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';

const Register = () => {
  const navigate = useNavigate();
  const [formData, setFormData] = useState({
    name: '',
    email: '',
    password: '',
  });
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

    fetch('http://localhost:4000/api/auth/register', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(formData)
    })
      .then((response) => response.json())
      .then((data) => {
        if (data.message) {
            toast.success('Регистрацията е успешна!');
            navigate('/');
          } else {
            setErrorMessage(data.error || 'Грешка при регистрация.');
          }
      })
      .catch((error) => {
        setErrorMessage('Грешка при регистрация. Опитайте отново.');
        console.error('Грешка:', error);
      });
  };

  return (
    <div className="container"> 
      <h1>Регистрация</h1>
      {errorMessage && <p className="message" style={{ color: 'red' }}>{errorMessage}</p>}
      {successMessage && <p className="message" style={{ color: 'green' }}>{successMessage}</p>}
      
      <form onSubmit={handleSubmit}>
        <input
          type="text"
          name="name"
          placeholder="Име"
          value={formData.name}
          onChange={handleInputChange}
          required
        />
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
        <button type="submit">Регистрирай се</button>
      </form>
      <p className="message">Вече имате акаунт? <a href="/">Влезте</a></p>
    </div>
  );
};

export default Register;