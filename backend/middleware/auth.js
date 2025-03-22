const jwt = require('jsonwebtoken');

const authMiddleware = (roles = []) => (req, res, next) => {
  const token = req.header('Authorization')?.replace('Bearer ', '');

  if (!token) {
    return res.status(401).json({ message: 'Нямате достъп. Моля, влезте в системата.' });
  }

  try {
    const decoded = jwt.verify(token, 'your_secret_key');

    if (roles.length > 0 && !roles.includes(decoded.role)) {
      return res.status(403).json({ message: 'Нямате права за достъп до този ресурс.' });
    }

    req.user = decoded;
    next();
  } catch (error) {
    console.error("Грешка при декодиране на токена:", error);
    res.status(401).json({ message: 'Невалиден токен. Моля, влезте отново.' });
  }
};

module.exports = authMiddleware;