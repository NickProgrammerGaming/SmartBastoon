import { BrowserRouter as Router, Route, Routes, Link } from 'react-router-dom';
import Login from './pages/Login';
import Register from './pages/Register';
import Stats from './pages/Stats';

function App() {
  return (
    <Router>
      <div>
        <nav className="navbar">
          <div>
            <Link to="/">Login</Link>
            <Link to="/register">Register</Link>
            <Link to="/stats">Stats</Link>
          </div>
          <div>
            <button
              onClick={() => {
                localStorage.removeItem('token');
                window.location.href = '/';
              }}
            >
              Logout
            </button>
          </div>
        </nav>
        <div className="container">
          <Routes>
            <Route path="/" element={<Login />} />
            <Route path="/register" element={<Register />} />
            <Route path="/stats" element={<Stats />} />
          </Routes>
        </div>
      </div>
    </Router>
  );
}

export default App;