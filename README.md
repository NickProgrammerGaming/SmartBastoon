# Smart Bastoon

Смарт бастун за незрящи хора, който включва различни сензори, интернет на нещата (IoT), blockchain за сигурност и уеб интерфейс за наблюдение.

- Версия 1 на Smart Bastoon

![bastoon](https://github.com/user-attachments/assets/048e7ef7-c51a-4b81-b634-308204378de0)

- Версия 2 на Smart Bastoon (на бредборд)

![image](https://github.com/user-attachments/assets/cc821061-2647-4e52-9d5a-07a0c4644733)

- Версия 2 на Smart Batsoon (завършен)

*TBA*

## Описание на проекта

Проектът включва смарт бастун за незрящи хора, който използва различни технологии, за да осигури безопасност и удобство:

- **Ултразвукови сензори** за разстояние, които засичат препятствия отпред и отстрани на умния бастун.
- **ESP32-CAM** за засичане на дупки и неравности по пътя.
- **Piezo buzzer**, който изписква при засичане на препятствие.
- **Вибромотор**, който вибрира при засичане на препятствие, за да информира потребителя.
- **Жироскоп**, който проверява дали незрящият е паднал или не.
- **Raspberry Pi** като gateway, който обработва и пренасочва данни към сървър.
- **LED** светлинна сигнализация.

## Технологии

Проектът включва следните основни технологии:

- **Node.js** за backend сървър
- **MySQL** за база данни, хоствана на AWS RDS
- **MQTT** за комуникация с IoT устройствата
- **WiFi** за връзка между устройството и сървъра.
- **Docker** за контейнеризация на приложението
- **Hedera Hashgraph** за сигурни транзакции и blockchain
- **React** за frontend
- **OpenCV** за разпознаване на неравности от снимките.

## Инсталация и настройка

### 1. Клониране на репото

Клонирайте репото от GitHub:

```bash
git clone https://github.com/NickProgrammerGaming/SmartBastoon.git
cd SmartBastoon
```

### 2. Настройка на AWS RDS

1. Създайте база данни MySQL в AWS RDS.
2. Вземете данните за хост, потребител, парола и име на база данни.
3. Създайте файл `.env` в бекенд директорията със следното съдържание:

```env
DB_HOST=your-db-host
DB_USER=your-db-user
DB_PASSWORD=your-db-password
DB_NAME=bastoon_db
```

### 3. Настройка на Backend

1. Навигирайте до директорията на бекенда:

```bash
cd backend
```

2. Инсталирайте зависимостите:

```bash
npm install
```

3. Стартирайте сървъра:

```bash
npm start
```

Вашето backend приложение ще бъде достъпно на http://localhost:4000.

### 4. Настройка на Frontend

1. Навигирайте до директорията на фронтенда:

```bash
cd frontend
```

2. Инсталирайте зависимостите:

```bash
npm install
```

3. Стартирайте приложението:

```bash
npm start
```

Вашето frontend приложение ще бъде достъпно на http://localhost:4173.

### 5. Настройка на Hedera Server

1. Клонирайте репото за Hedera Hashgraph сървъра от repo-то:

2. Инсталирайте зависимостите:

```bash
npm install
```

3. Конфигурирайте сървъра с вашите Hedera идентификатори. Създайте файл .env и добавете вашите ACCOUNT_ID, PRIVATE_KEY и TOPIC_ID:

```bash
env
ACCOUNT_ID=your-account-id
PRIVATE_KEY=your-private-key
TOPIC_ID=your-topic-id
```

4. Стартирайте сървъра:

```bash
npm start
```

### 6. IoT част
1. Използвайте ESP32-CAM за засичане на изображения и изпращане на данни към MQTT брокера.

2. За комуникация с MQTT, използвайте библиотеката за MQTT:

```bash
const mqtt = require('mqtt');
const client = mqtt.connect('mqtt://localhost');

client.on('connect', () => {
  console.log('IoT Device connected to MQTT broker');
  client.subscribe('esp32/sensordata');
  client.subscribe('esp32/emergency');
});

client.on('message', (topic, message) => {
  const data = JSON.parse(message.toString());
  // Записвайте данни в база данни или изпращайте на blockchain сървъра
});
```
### 7. Информация за работата на проекта
Сензорни данни като сензори за разстояние, жироскоп и изображения от ESP32 се събират от IoT устройства, които изпращат данни към сървъра чрез MQTT.

Данните се записват в MySQL база данни, хоствана на AWS RDS.

Hedera Hashgraph сървърът използва HCS (Hedera Consensus Service) за сигурно записване на данни за изображения и друга информация в blockchain.

Уеб интерфейсът предоставя възможност за наблюдение на данни от сензорите и управление на системата.

### 8. Лиценз
Това приложение е с отворен код и може да се използва, променя и разпространява по лиценз MIT.
