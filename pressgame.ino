const int buttonPin = 2;
const int lightPin = A0;   // 光敏電阻模組 AO
const int RledPin = 9;     // PWM
const int GledPin = 10;    // PWM
const int BledPin = 11;    // PWM

const int neutralMood = 10;
const int darkThreshold = 500;        // 暗/亮的分界，要依實測調整
const long unTouchInterval = 5000;    // 5 秒沒按才開始扣分
const long reducedInterval = 1000;    // 每 1 秒扣 1 分（只在亮處）
const long breathPeriod = 4000;       // 呼吸一圈的時間（毫秒）

int mood = neutralMood;
bool buttonPressed = false;
unsigned long touchedTimer = 0;
unsigned long reducedTimer = 0;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // 讀光線
  int light = analogRead(lightPin);
  // Serial.println(light);   // 校準時把這行的註解打開
  bool isDark = light > darkThreshold;

  // 暗處用呼吸亮度，亮處全亮
  float scale = isDark ? breathing() : 1.0;
  showLEDState(mood, scale);

  bool button = digitalRead(buttonPin);
  unsigned long now = millis();

  if (isDark) {
    // 睡覺：心情完全不變（不加分、不扣分），按鈕無效
    touchedTimer = now;
    buttonPressed = (button == HIGH);  // 同步按鈕狀態，避免醒來瞬間誤判成新的按下
  }
  else {
    // 按鈕：按下瞬間 +1
    if (button == HIGH && !buttonPressed) {
      mood = min(mood + 1, 20);
      touchedTimer = now;
      buttonPressed = true;
    }
    if (button == LOW) {
      buttonPressed = false;
    }

    // 超過 5 秒沒按，每秒 -1
    if (now - touchedTimer > unTouchInterval && now - reducedTimer > reducedInterval) {
      mood = max(mood - 1, 0);
      reducedTimer = now;
    }
  }
}

// 回傳 0.02 ~ 1.0 的呼吸亮度比例（三次方，暗亮時間比較平均）
float breathing() {
  float phase = (millis() % breathPeriod) / (float)breathPeriod * 2.0 * PI;
  float wave = (sin(phase - PI / 2) + 1.0) / 2.0;
  return 0.02 + 0.98 * wave * wave * wave;
}

// 共陽極：r、g、b 以「越大越亮」計算，輸出時才反相
void showLEDState(int state, float scale) {
  float step = 255 / 10.0;
  float r, g, b;

  if (state >= neutralMood) {          // 綠 → 藍
    float d = step * (state - neutralMood);
    r = 0;
    g = 255 - d;
    b = d;
  } else {                             // 綠 → 紅
    float d = step * (neutralMood - state);
    r = d;
    g = 255 - d;
    b = 0;
  }

  analogWrite(RledPin, 255 - r * scale);
  analogWrite(GledPin, 255 - g * scale);
  analogWrite(BledPin, 255 - b * scale);
}